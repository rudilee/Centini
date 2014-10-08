#include <QTimer>
#include <QCoreApplication>
#include <QDebug>

#include "centiniserver.h"

CentiniServer::CentiniServer(QObject *parent) :
    QObject(parent)
{
    connect(&asterisk, SIGNAL(connected(QString)), SLOT(onAsteriskConnected(QString)));
    connect(&asterisk, SIGNAL(disconnected()), SLOT(onAsteriskDisconnected()));
    connect(&asterisk, SIGNAL(responseSent(AsteriskManager::Action,AsteriskManager::Response,QVariantMap)),
            SLOT(onAsteriskResponseSent(AsteriskManager::Action,AsteriskManager::Response,QVariantMap)));
    connect(&asterisk, SIGNAL(eventGenerated(AsteriskManager::Event,QVariantMap)),
            SLOT(onAsteriskEventGenerated(AsteriskManager::Event,QVariantMap)));

    asterisk.connectToHost("192.168.1.8", 5038);

    QTimer::singleShot(5000, this, SLOT(disconnectAsterisk()));
}

void CentiniServer::disconnectAsterisk()
{
    asterisk.actionLogoff();
}

void CentiniServer::onAsteriskConnected(QString version)
{
    asterisk.actionLogin("monitor", "m0n1t0r");

    qDebug() << "AMI version:" << version;
}

void CentiniServer::onAsteriskDisconnected()
{
    QCoreApplication::quit();
}

void CentiniServer::onAsteriskResponseSent(AsteriskManager::Action action, AsteriskManager::Response response, QVariantMap headers)
{
    qDebug() << "Response:" << headers;
}

void CentiniServer::onAsteriskEventGenerated(AsteriskManager::Event event, QVariantMap headers)
{
    qDebug() << "Event:" << headers;
}
