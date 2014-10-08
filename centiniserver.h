#ifndef CENTINISERVER_H
#define CENTINISERVER_H

#include <QObject>

#include "asteriskmanager.h"

class CentiniServer : public QObject
{
    Q_OBJECT

public:
    explicit CentiniServer(QObject *parent = 0);

private:
    AsteriskManager asterisk;

private slots:
    void disconnectAsterisk();

    void onAsteriskConnected(QString version);
    void onAsteriskDisconnected();
    void onAsteriskResponseSent(AsteriskManager::Action action, AsteriskManager::Response response, QVariantMap headers);
    void onAsteriskEventGenerated(AsteriskManager::Event event, QVariantMap headers);
};

#endif // CENTINISERVER_H
