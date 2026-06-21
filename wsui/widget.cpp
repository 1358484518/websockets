#include "widget.h"
#include "ui_widget.h"
#include "cJSON.h"
#include "QVBoxLayout"

#include "QDebug"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    m_LogTextEdit = new QTextEdit;
    m_LogTextEdit->setReadOnly(true);
    m_LogTextEdit->append("这里打印log");
    m_OcppCtrlWidget = new OcppCtrlWidget;
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_OcppCtrlWidget,2);
    layout->addWidget(m_LogTextEdit);
    setLayout(layout);
    m_wsClicent = new WebSocketTcpClient;//自动发送bootnotify和Heartbeat心跳
    m_ocppClicent = new OcppClient;
    m_ocppClicent->setWebSocketClient(m_wsClicent);

//    qDebug()<<cJSON_Version();
}

Widget::~Widget()
{
    delete ui;
}

