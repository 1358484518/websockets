#ifndef OCPPPROTOCOL_H
#define OCPPPROTOCOL_H

#include <QObject>
#include "cJSON.h"
#include "ocpp/OcppAllMessages.h"



class OcppProtocol : public QObject
{
    Q_OBJECT
public:
    explicit OcppProtocol(QObject *parent = nullptr);

signals:
//public slots:
//    void AuthorizeReqSlot();
private:

};







#endif // OCPPPROTOCOL_H
