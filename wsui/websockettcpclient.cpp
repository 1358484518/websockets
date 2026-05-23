#include "WebSocketTcpClient.h"
#include <QCryptographicHash>
WebSocketTcpClient::WebSocketTcpClient(QObject *parent)
    : QObject(parent)
    , m_handshakeDone(false)
{
    m_tcpSocket = new QTcpSocket(this);
    m_timer = new QTimer(this);

    connect(m_tcpSocket, &QTcpSocket::connected, this, &WebSocketTcpClient::onTcpConnected);
    connect(m_tcpSocket, &QTcpSocket::readyRead, this, &WebSocketTcpClient::onDataReceived);
    connect(m_timer, &QTimer::timeout, this, &WebSocketTcpClient::sendAutoMessage);

    m_tcpSocket->connectToHost("127.0.0.1", 8765);
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
        // 打印服务器返回的握手信息（调试用）
        qDebug() << "[WS] 握手响应：\n" << data;

        // 1. 检查是否切换协议成功
        if (data.contains("101 Switching Protocols")) {

            // ==============================================
            // ✅【关键增加】开始校验 Sec-WebSocket-Accept
            // ==============================================
            QString response = QString::fromUtf8(data);

            // ① 从服务器返回的头中，提取 Sec-WebSocket-Accept
            QString serverAccept;
            int acceptIndex = response.indexOf("Sec-WebSocket-Accept:");
            if (acceptIndex != -1) {
                int start = acceptIndex + 21;
                int end = response.indexOf("\r\n", start);
                serverAccept = response.mid(start, end - start).trimmed();
            }

            // ② 客户端自己用【固定算法】计算正确的 Accept
            // 你发送的 Key（必须和发送握手时用的一样！）
            QString clientKey = "dGhlIHNhbXBsZSBub25jZQ==";
            // WebSocket 标准固定串（全球唯一，不能改）
            QString magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

            // SHA1 + Base64 编码
            QByteArray sha1Result = QCryptographicHash::hash(
                (clientKey + magic).toUtf8(),
                QCryptographicHash::Sha1
            );
            QString realAccept = sha1Result.toBase64();

            // ③ 校验：服务器返回的 和 我算的 是否一致
            if (serverAccept == realAccept) {
                qDebug() << "[WS] ✅ 握手成功 + 服务端认证成功！";
                m_handshakeDone = true;
                m_timer->start(1000);
            } else {
                qDebug() << "[WS] ❌ 认证失败！非法服务器！";
                qDebug() << "期望：" << realAccept;
                qDebug() << "收到：" << serverAccept;
                m_tcpSocket->close();
            }

        }
    } else {
        // 已经握手完成，正常解析帧
        parseWebSocketFrame(data);
    }
}

// ============================
// 解析 WebSocket 帧（含关闭处理）
// ============================
void WebSocketTcpClient::parseWebSocketFrame(const QByteArray &data)
{
    if (data.size() < 2)
        return;

    const uchar *buf = (const uchar*)data.constData();
    int pos = 0;

    // 第1字节：FIN + 操作码
    uchar fin_op = buf[pos++];
    // 第2字节：掩码位 + 长度
    uchar mask_len = buf[pos++];

    // 解析
    bool fin = (fin_op & 0x80) != 0;       // 是否最后一帧
    int opcode = fin_op & 0x0F;             // 帧类型
    bool mask = (mask_len & 0x80) != 0;    // 是否有掩码
    int payload_len = mask_len & 0x7F;     // 数据长度

    // ==========================================
    // ✅ 【关键】如果是 关闭帧（opcode == 0x08）
    // ==========================================
    if (opcode == 0x08) {
        qDebug() << "[WS] 收到服务器关闭帧 → 安全关闭TCP连接";
        m_timer->stop();        // 停止定时器
        m_tcpSocket->close();   // 关闭TCP
        return;
    }

    // 读取掩码key
    QByteArray maskKey;
    if (mask) {
        maskKey = data.mid(pos, 4);
        pos += 4;
    }

    // 读取数据
    QByteArray payload = data.mid(pos, payload_len);

    // 解码（只有mask=1才解码，服务器下发没有掩码）
    if (mask) {
        for (int i = 0; i < payload_len; i++) {
            payload[i] = payload[i] ^ maskKey[i % 4];
        }
    }

    qDebug() << "[RECV] 正确收到：" << QString::fromUtf8(payload);
}

// ============================
// 发送（标准掩码，正确）
// ============================
void WebSocketTcpClient::sendAutoMessage()
{
    static int count = 0;
    count++;

    QString msg = QString("TCP原生WebSocket消息 %1").arg(count);
    QByteArray payload = msg.toUtf8();

    uchar mask[4] = {0x11,0x22,0x33,0x44};
    QByteArray encoded = payload;

    for (int i=0; i<payload.size(); i++) {
        encoded[i] = payload[i] ^ mask[i%4];
    }

    QByteArray frame;
    frame.append((char)0x81);
    frame.append((char)(0x80 | payload.size()));
    frame.append((char)mask[0]);
    frame.append((char)mask[1]);
    frame.append((char)mask[2]);
    frame.append((char)mask[3]);
    frame.append(encoded);

    m_tcpSocket->write(frame);
    qDebug() << "[SEND] " << msg;
}
// ============================
// 标准关闭 WebSocket（客户端主动关闭）
// ============================
void WebSocketTcpClient::closeWebSocket()
{
    if (m_tcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        qDebug() << "[WS] 主动发送关闭帧，请求断开连接";

        // 关闭帧格式
        QByteArray closeFrame;
        closeFrame.append((char)0x88);  // FIN=1 + opcode=8 (关闭帧)
        closeFrame.append((char)0x00);  // 长度0，无数据

        m_tcpSocket->write(closeFrame);
        m_tcpSocket->flush();

        m_timer->stop(); // 立即停止发送
    }
}
