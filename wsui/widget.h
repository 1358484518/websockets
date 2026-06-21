#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "websockettcpclient.h"
#include "OcppClient.h"
#include "OcppCtrlWidget.h"
#include "QTextEdit"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    Ui::Widget *ui;
    WebSocketTcpClient *m_wsClicent;
    OcppClient *m_ocppClicent;

    QTextEdit *m_LogTextEdit;
    OcppCtrlWidget *m_OcppCtrlWidget;
};
#endif // WIDGET_H
