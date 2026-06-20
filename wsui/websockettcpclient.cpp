#include "websockettcpclient.h"
#include <QCryptographicHash>

//WebSocketTcpClient::WebSocketTcpClient(QObject *parent)
//    : QObject(parent)
//    , m_handshakeDone(false)
//    , m_isFragmented(false)
//    , m_reconnectInterval(1000)     // 初始重连间隔 1 秒
//    , m_maxReconnectInterval(30000) // 最大重连间隔 30 秒
//    , m_autoReconnect(true)         // 默认开启自动重连
//    , m_manualClose(false)          // 默认不是主动关闭
//{
//    m_tcpSocket = new QTcpSocket(this);
//    m_timer     = new QTimer(this);

//    connect(m_tcpSocket, &QTcpSocket::connected, this, &WebSocketTcpClient::onTcpConnected);
//    connect(m_tcpSocket, &QTcpSocket::readyRead, this, &WebSocketTcpClient::onDataReceived);
//    connect(m_timer, &QTimer::timeout, this, &WebSocketTcpClient::sendAutoMessage);

//    m_tcpSocket->connectToHost("127.0.0.1", 8765);
//}
WebSocketTcpClient::WebSocketTcpClient(QObject *parent)
    : QObject(parent)
    , m_handshakeDone(false)
    , m_isFragmented(false)
    , m_reconnectInterval(1000)     // 初始重连间隔 1 秒
    , m_maxReconnectInterval(30000) // 最大重连间隔 30 秒
    , m_autoReconnect(true)         // 默认开启自动重连
    , m_manualClose(false)          // 默认不是主动关闭
{
    m_tcpSocket = new QTcpSocket(this);
    m_timer     = new QTimer(this);
    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setSingleShot(true); // 单次触发

    connect(m_tcpSocket, &QTcpSocket::connected, this, &WebSocketTcpClient::onTcpConnected);
    connect(m_tcpSocket, &QTcpSocket::readyRead, this, &WebSocketTcpClient::onDataReceived);
    connect(m_tcpSocket, &QTcpSocket::disconnected, this, &WebSocketTcpClient::onTcpDisconnected);
    connect(m_tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
            this, [this](QAbstractSocket::SocketError err) {
        Q_UNUSED(err)
        qDebug() << "[TCP] 连接错误:" << m_tcpSocket->errorString();
        if (m_autoReconnect && !m_manualClose) {
            startReconnect();
        }
    });
    connect(m_timer, &QTimer::timeout, this, &WebSocketTcpClient::sendAutoMessage);
    connect(m_reconnectTimer, &QTimer::timeout, this, [this]() {
        qDebug() << "[RECONNECT] 尝试重连... 间隔:" << m_reconnectInterval << "ms";
        m_tcpSocket->connectToHost("127.0.0.1", 8765);
    });

    m_tcpSocket->connectToHost("127.0.0.1", 8765);
}
// ============================================================
// TCP 断开回调（异常断开自动重连）
// ============================================================
void WebSocketTcpClient::onTcpDisconnected()
{
    qDebug() << "[TCP] 连接已断开";

    m_timer->stop();
    m_handshakeDone = false;
    m_recvBuffer.clear();
    m_fragmentBuffer.clear();
    m_isFragmented = false;

    // 主动关闭的不重连
    if (m_manualClose) {
        qDebug() << "[RECONNECT] 主动关闭，不重连";
        m_manualClose = false;
        return;
    }

    // 自动重连
    if (m_autoReconnect) {
        startReconnect();
    }
}
// ============================================================
// 开始重连（指数退避）
// ============================================================
void WebSocketTcpClient::startReconnect()
{

    if (m_reconnectTimer->isActive()) return;
    if (m_tcpSocket->state() == QAbstractSocket::ConnectingState) return;
    if (m_tcpSocket->state() == QAbstractSocket::ConnectedState) return;

    m_reconnectTimer->start(m_reconnectInterval);
    qDebug() << "[RECONNECT] " << m_reconnectInterval << "ms 后重连";

    m_reconnectInterval *= 2;
    if (m_reconnectInterval > m_maxReconnectInterval) {
        m_reconnectInterval = m_maxReconnectInterval;
    }
}

// ============================================================
// 停止重连
// ============================================================
void WebSocketTcpClient::stopReconnect()
{
    if (m_reconnectTimer->isActive()) {
        m_reconnectTimer->stop();
        qDebug() << "[RECONNECT] 停止重连";
    }
}

// ============================================================
// 重连成功后重置
// ============================================================
void WebSocketTcpClient::resetReconnect()
{
    stopReconnect();
    m_reconnectInterval = 1000; // 重置为初始间隔
}
void WebSocketTcpClient::onTcpConnected()
{
    qDebug() << "[TCP] 连接成功";
    sendUpgradeRequest();
}

void WebSocketTcpClient::sendUpgradeRequest()
{
    QString key = "dGhlIHNhbXBsZSBub25jZQ==";
    QString req = QString(
        "GET / HTTP/1.1\r\n"
        "Host: 127.0.0.1:8765\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: %1\r\n"
        "Sec-WebSocket-Version: 13\r\n\r\n"
    ).arg(key);

    m_tcpSocket->write(req.toUtf8());
}

void WebSocketTcpClient::onDataReceived()
{
    QByteArray data = m_tcpSocket->readAll();

    if (!m_handshakeDone) {
        qDebug() << "[WS] 握手响应：\n" << data;

        if (data.contains("101 Switching Protocols")) {
            QString response = QString::fromUtf8(data);

            QString serverAccept;
            int acceptIndex = response.indexOf("Sec-WebSocket-Accept:");
            if (acceptIndex != -1) {
                int start = acceptIndex + 21;
                int end = response.indexOf("\r\n", start);
                serverAccept = response.mid(start, end - start).trimmed();
            }

            QString clientKey = "dGhlIHNhbXBsZSBub25jZQ==";
//            这个是websocket固定的
            QString magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
            QByteArray sha1Result = QCryptographicHash::hash(
                (clientKey + magic).toUtf8(),
                QCryptographicHash::Sha1
            );
            QString realAccept = sha1Result.toBase64();

            if (serverAccept == realAccept) {
                qDebug() << "[WS] ✅ 握手成功 + 服务端认证成功！";
                m_handshakeDone = true;
                m_recvBuffer.clear();
                resetReconnect();   // ✅ 加这行：连接成功重置重连状态
                m_timer->start(1000);
            } else {
                qDebug() << "[WS] ❌ 认证失败！非法服务器！";
                qDebug() << "期望：" << realAccept;
                qDebug() << "收到：" << serverAccept;
                m_tcpSocket->close();
            }
        }
    } else {
        parseWebSocketFrame(data);
    }
}

// ============================================================
// 解析 WebSocket 帧（完整健壮版：粘包/拆包/分片/全opcode）
// ============================================================
void WebSocketTcpClient::parseWebSocketFrame(const QByteArray &data)
{
    m_recvBuffer.append(data);

    while (m_recvBuffer.size() >= 2)
    {
        const uchar *buf = (const uchar*)m_recvBuffer.constData();
        int pos = 0;

        // ---- 第1字节：FIN + opcode ----
        uchar fin_op = buf[pos++];
        bool fin = (fin_op & 0x80) != 0;
        int opcode = fin_op & 0x0F;

        // ---- 第2字节：mask + 基础长度 ----
        uchar mask_len = buf[pos++];
        bool mask = (mask_len & 0x80) != 0;
        quint64 payload_len = mask_len & 0x7F;

        // ---- 扩展长度（126 / 127） ----
        if (payload_len == 126) {
            if (m_recvBuffer.size() < pos + 2) return;
            payload_len = (quint64(buf[pos]) << 8) | buf[pos + 1];
            pos += 2;
        }
        else if (payload_len == 127) {
            if (m_recvBuffer.size() < pos + 8) return;
            payload_len = 0;
            for (int i = 0; i < 8; i++) {
                payload_len = (payload_len << 8) | buf[pos + i];
            }
            pos += 8;
        }

        // ---- 掩码 key 占4字节 ----
        int maskKeyPos = pos;
        if (mask) {
            if (m_recvBuffer.size() < pos + 4) return;
            pos += 4;
        }

        // ---- payload 收全了吗？ ----
        if (m_recvBuffer.size() < pos + (int)payload_len) {
            return; // 不完整，等下一批
        }

        // ---- 提取 payload ----
        QByteArray payload = m_recvBuffer.mid(pos, (int)payload_len);

        // ---- 掩码解码 ----
        if (mask) {
            const uchar *mk = (const uchar*)m_recvBuffer.constData() + maskKeyPos;
            for (int i = 0; i < (int)payload_len; i++) {
                payload[i] = payload[i] ^ mk[i % 4];
            }
        }

        // ---- 从缓冲区移除已解析的帧 ----
        m_recvBuffer.remove(0, pos + (int)payload_len);

        // ==============================================
        // 按 opcode 分发
        // ==============================================
        switch (opcode)
        {
        case 0x00: { // 续帧
            if (!m_isFragmented) {
                qDebug() << "[WS] ⚠️ 收到意外续帧，忽略";
                break;
            }
            m_fragmentBuffer.append(payload);
            if (fin) {
                qDebug() << "[RECV] 分片消息完成，长度:" << m_fragmentBuffer.size();
                qDebug() << "[RECV] 内容：" << QString::fromUtf8(m_fragmentBuffer);
                m_fragmentBuffer.clear();
                m_isFragmented = false;
            }
            break;
        }

        case 0x01: { // 文本帧
            if (!fin) {
                m_isFragmented = true;
                m_fragmentBuffer = payload;
            } else {
                qDebug() << "[RECV] 正确收到：" << QString::fromUtf8(payload);
            }
            break;
        }

        case 0x02: { // 二进制帧
            if (!fin) {
                m_isFragmented = true;
                m_fragmentBuffer = payload;
            } else {
                qDebug() << "[RECV] 收到二进制数据，长度:" << payload.size();
            }
            break;
        }

        case 0x08: { // 关闭帧
            quint16 closeCode = 1000;
            QString closeReason;
            if (payload.size() >= 2) {
                closeCode = (quint8(payload[0]) << 8) | quint8(payload[1]);
                if (payload.size() > 2) {
                    closeReason = QString::fromUtf8(payload.mid(2));
                }
            }
            qDebug() << "[WS] 收到服务器关闭帧，码:" << closeCode
                     << "原因:" << closeReason << "→ 安全关闭";
            m_timer->stop();
            m_tcpSocket->close();
            m_recvBuffer.clear();
            m_fragmentBuffer.clear();
            m_isFragmented = false;
            return;
        }

        case 0x09: { // ping → 回 pong
            qDebug() << "[WS] 收到 ping，长度:" << payload.size() << "→ 回 pong";
            sendPong(payload);
            break;
        }

        case 0x0A: { // pong
            qDebug() << "[WS] 收到 pong，长度:" << payload.size();
            break;
        }

        default: {
            qDebug() << "[WS] ⚠️ 未知 opcode: 0x" << QString::number(opcode, 16) << "，忽略";
            break;
        }
        }
    }
}

// ============================================================
// 回 pong（带掩码，符合协议）
// ============================================================
void WebSocketTcpClient::sendPong(const QByteArray &payload)
{
    buildAndSendFrame(0x0A, payload);
}

// ============================================================
// 通用发送：组装 WebSocket 帧 + 掩码 + 发出
// ============================================================
void WebSocketTcpClient::buildAndSendFrame(int opcode, const QByteArray &payload)
{
    uchar maskKey[4] = {0x11, 0x22, 0x33, 0x44};
    int len = payload.size();

    // 掩码后的数据
    QByteArray masked = payload;
    for (int i = 0; i < len; i++) {
        masked[i] = payload[i] ^ maskKey[i % 4];
    }

    QByteArray frame;

    // 第1字节：FIN=1 + opcode
    frame.append((char)(0x80 | (opcode & 0x0F)));

    // 第2字节起：mask=1 + 长度
    if (len < 126) {
        frame.append((char)(0x80 | len));
    }
    else if (len < 65536) {
        frame.append((char)(0x80 | 126));
        frame.append((char)((len >> 8) & 0xFF));
        frame.append((char)(len & 0xFF));
    }
    else {
        frame.append((char)(0x80 | 127));
        for (int i = 7; i >= 0; i--) {
            frame.append((char)((len >> (i * 8)) & 0xFF));
        }
    }

    // 掩码 key
    frame.append((char)maskKey[0]);
    frame.append((char)maskKey[1]);
    frame.append((char)maskKey[2]);
    frame.append((char)maskKey[3]);

    // 掩码后的数据
    frame.append(masked);

    m_tcpSocket->write(frame);
}

// ============================================================
// 自动发消息（定时器回调）
// ============================================================
void WebSocketTcpClient::sendAutoMessage()
{
    static int count = 0;
    count++;
    QString msg = QString("TCP原生WebSocket消息 %1").arg(count);
    QByteArray payload = msg.toUtf8();

    buildAndSendFrame(0x01, payload);
    qDebug() << "[SEND] " << msg;
}

// ============================================================
// 通用发送接口（对外）
// ============================================================
void WebSocketTcpClient::sendMessage(const QByteArray &data)
{
    buildAndSendFrame(0x02, data); // 二进制帧，需要文本就传 0x01
}

// ============================================================
// 主动关闭（关闭帧也带掩码）
// ============================================================
void WebSocketTcpClient::closeWebSocket()
{
    if (m_tcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        qDebug() << "[WS] 主动发送关闭帧，请求断开连接";
        m_manualClose = true;   // ✅ 加这行：标记为主动关闭，不重连
                stopReconnect();        // ✅ 加这行：停止重连定时器
        // 关闭帧 payload：2字节关闭码(1000) + 可选原因
        QByteArray closePayload;
        closePayload.append((char)0x03); // 1000 = 0x03E8
        closePayload.append((char)0xE8);
        closePayload.append("正常关闭");

        buildAndSendFrame(0x08, closePayload);
        m_tcpSocket->flush();
        m_timer->stop();
    }
}
// cpp 加：
void WebSocketTcpClient::setAutoReconnect(bool enable)
{
    m_autoReconnect = enable;
    if (!enable) stopReconnect();
}

bool WebSocketTcpClient::isAutoReconnect() const
{
    return m_autoReconnect;
}
// ============================================================
// 发送文本消息（槽函数）
// ============================================================
void WebSocketTcpClient::sendTextMessage(const QString &text)
{
    if (!m_handshakeDone || m_tcpSocket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "[WS] ⚠️ 未连接，发送失败";
        return;
    }

    QByteArray payload = text.toUtf8();
    buildAndSendFrame(0x01, payload); // opcode=0x01 文本帧
    qDebug() << "[SEND] 文本:" << text;
}

// ============================================================
// 发送二进制消息（槽函数）
// ============================================================
void WebSocketTcpClient::sendBinaryMessage(const QByteArray &data)
{
    if (!m_handshakeDone || m_tcpSocket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "[WS] ⚠️ 未连接，发送失败";
        return;
    }

    buildAndSendFrame(0x02, data); // opcode=0x02 二进制帧
    qDebug() << "[SEND] 二进制数据，长度:" << data.size();
}



