#ifndef OCPPPROTOCOL_H
#define OCPPPROTOCOL_H

#include <QObject>
#include "cJSON.h"
//#include "ocpp/OcppAllMessages.h"
class OcppAction;

class OcppProtocol : public QObject
{
    Q_OBJECT
public:
    explicit OcppProtocol(QObject *parent = nullptr);

signals:
private:
    OcppAction *m_ocppAction;
};



#endif // OCPPPROTOCOL_H
