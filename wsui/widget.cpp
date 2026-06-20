#include "widget.h"
#include "ui_widget.h"
#include "cJSON.h"
#include "QDebug"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    qDebug()<<cJSON_Version();
}

Widget::~Widget()
{
    delete ui;
}

