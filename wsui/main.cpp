#include "widget.h"
#include "WebSocketTcpClient.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.show();
    WebSocketTcpClient client;
    return a.exec();
}
