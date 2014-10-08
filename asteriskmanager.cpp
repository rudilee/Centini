#include <QDateTime>

#include "asteriskmanager.h"

AsteriskManager::AsteriskManager(QObject *parent) :
    QTcpSocket(parent),
    responseEnum(AsteriskManager::staticMetaObject.enumerator(AsteriskManager::staticMetaObject.indexOfEnumerator("Response"))),
    eventEnum(AsteriskManager::staticMetaObject.enumerator(AsteriskManager::staticMetaObject.indexOfEnumerator("Event"))),
    actionEnum(AsteriskManager::staticMetaObject.enumerator(AsteriskManager::staticMetaObject.indexOfEnumerator("Action")))
{
    connect(this, SIGNAL(readyRead()), SLOT(onReadyRead()));
}

void AsteriskManager::actionAbsoluteTimeout()
{
}

void AsteriskManager::actionAgentLogoff()
{
}

void AsteriskManager::actionAgents()
{
}

void AsteriskManager::actionAGI()
{
}

void AsteriskManager::actionAOCMessage()
{
}

void AsteriskManager::actionAtxfer()
{
}

void AsteriskManager::actionBridge()
{
}

void AsteriskManager::actionChallenge()
{
}

void AsteriskManager::actionChangeMonitor()
{
}

void AsteriskManager::actionCommand()
{
}

void AsteriskManager::actionConfbridgeKick()
{
}

void AsteriskManager::actionConfbridgeList()
{
}

void AsteriskManager::actionConfbridgeListRooms()
{
}

void AsteriskManager::actionConfbridgeLock()
{
}

void AsteriskManager::actionConfbridgeMute()
{
}

void AsteriskManager::actionConfbridgeSetSingleVideoSrc()
{
}

void AsteriskManager::actionConfbridgeStartRecord()
{
}

void AsteriskManager::actionConfbridgeStopRecord()
{
}

void AsteriskManager::actionConfbridgeUnlock()
{
}

void AsteriskManager::actionConfbridgeUnmute()
{
}

void AsteriskManager::actionCoreSettings()
{
}

void AsteriskManager::actionCoreShowChannels()
{
}

void AsteriskManager::actionCoreStatus()
{
}

void AsteriskManager::actionCreateConfig()
{
}

void AsteriskManager::actionDAHDIDialOffhook()
{
}

void AsteriskManager::actionDAHDIDNDoff()
{
}

void AsteriskManager::actionDAHDIDNDon()
{
}

void AsteriskManager::actionDAHDIHangup()
{
}

void AsteriskManager::actionDAHDIRestart()
{
}

void AsteriskManager::actionDAHDIShowChannels()
{
}

void AsteriskManager::actionDAHDITransfer()
{
}

void AsteriskManager::actionDataGet()
{
}

void AsteriskManager::actionDBDel()
{
}

void AsteriskManager::actionDBDelTree()
{
}

void AsteriskManager::actionDBGet()
{
}

void AsteriskManager::actionDBPut()
{
}

void AsteriskManager::actionEvents()
{
}

void AsteriskManager::actionExtensionState()
{
}

void AsteriskManager::actionFilter()
{
}

void AsteriskManager::actionFilterList()
{
}

void AsteriskManager::actionGetConfig()
{
}

void AsteriskManager::actionGetConfigJSON()
{
}

void AsteriskManager::actionGetvar()
{
}

void AsteriskManager::actionHangup(QString channel, uint cause, QString actionID)
{
    QVariantMap headers;
    headers["ActionID"] = actionID;
    headers["Channel"] = channel;

    insertNotEmpty(&headers, "Cause", cause);

    sendAction(Action::Login, headers);
}

void AsteriskManager::actionIAXnetstats()
{
}

void AsteriskManager::actionIAXpeerlist()
{
}

void AsteriskManager::actionIAXpeers()
{
}

void AsteriskManager::actionIAXregistry()
{
}

void AsteriskManager::actionJabberSend()
{
}

void AsteriskManager::actionJabberSend_res_jabber()
{
}

void AsteriskManager::actionJabberSend_res_xmpp()
{
}

void AsteriskManager::actionListCategories()
{
}

void AsteriskManager::actionListCommands()
{
}

void AsteriskManager::actionLocalOptimizeAway()
{
}

void AsteriskManager::actionLogin(QString username, QString secret, QString actionID)
{
    QVariantMap headers;
    headers["ActionID"] = actionID;
    headers["Username"] = username;

    insertNotEmpty(&headers, "Secret", secret);

    sendAction(Action::Login, headers);
}

void AsteriskManager::actionLogoff(QString actionID)
{
    QVariantMap headers;
    headers["ActionID"] = actionID;

    sendAction(Action::Logoff, headers);
}

void AsteriskManager::actionMailboxCount()
{
}

void AsteriskManager::actionMailboxStatus()
{
}

void AsteriskManager::actionMeetmeList()
{
}

void AsteriskManager::actionMeetmeListRooms()
{
}

void AsteriskManager::actionMeetmeMute()
{
}

void AsteriskManager::actionMeetmeUnmute()
{
}

void AsteriskManager::actionMessageSend()
{
}

void AsteriskManager::actionMixMonitor()
{
}

void AsteriskManager::actionMixMonitorMute()
{
}

void AsteriskManager::actionModuleCheck()
{
}

void AsteriskManager::actionModuleLoad()
{
}

void AsteriskManager::actionMonitor()
{
}

void AsteriskManager::actionMuteAudio()
{
}

void AsteriskManager::actionOriginate(QString channel,
                                      QString exten,
                                      QString context,
                                      uint priority,
                                      QString application,
                                      QString data,
                                      uint timeout,
                                      QString callerID,
                                      QVariantMap variables,
                                      QString account,
                                      bool earlyMedia,
                                      bool async,
                                      QStringList codecs, QString actionID)
{
    QVariantMap headers;
    headers["ActionID"] = actionID;
    headers["Channel"] = channel;

    insertNotEmpty(&headers, "Exten", exten);
    insertNotEmpty(&headers, "Context", context);
    insertNotEmpty(&headers, "Priority", priority);
    insertNotEmpty(&headers, "Application", application);
    insertNotEmpty(&headers, "Data", data);
    insertNotEmpty(&headers, "Timeout", timeout);
    insertNotEmpty(&headers, "CallerID", callerID);
    insertNotEmpty(&headers, "Account", account);
    insertNotEmpty(&headers, "EarlyMedia", earlyMedia);
    insertNotEmpty(&headers, "Async", async);
    insertNotEmpty(&headers, "Codecs", codecs.join(','));

    if (!variables.isEmpty()) {
        QMapIterator<QString, QVariant> variable(variables);
        while (variable.hasNext()) {
            variable.next();

            headers.insertMulti("Variable", QString("%1=%2").arg(variable.key(), valueToString(variable.value())));
        }
    }

    sendAction(Action::Originate, headers);
}

void AsteriskManager::actionPark()
{
}

void AsteriskManager::actionParkedCalls()
{
}

void AsteriskManager::actionParkinglots()
{
}

void AsteriskManager::actionPauseMonitor()
{
}

void AsteriskManager::actionPing()
{
}

void AsteriskManager::actionPlayDTMF()
{
}

void AsteriskManager::actionPresenceState()
{
}

void AsteriskManager::actionPRIShowSpans()
{
}

void AsteriskManager::actionQueueAdd()
{
}

void AsteriskManager::actionQueueLog()
{
}

void AsteriskManager::actionQueueMemberRingInUse()
{
}

void AsteriskManager::actionQueuePause()
{
}

void AsteriskManager::actionQueuePenalty()
{
}

void AsteriskManager::actionQueueReload()
{
}

void AsteriskManager::actionQueueRemove()
{
}

void AsteriskManager::actionQueueReset()
{
}

void AsteriskManager::actionQueueRule()
{
}

void AsteriskManager::actionQueues()
{
}

void AsteriskManager::actionQueueStatus()
{
}

void AsteriskManager::actionQueueSummary()
{
}

void AsteriskManager::actionRedirect()
{
}

void AsteriskManager::actionReload()
{
}

void AsteriskManager::actionSendText()
{
}

void AsteriskManager::actionSetvar()
{
}

void AsteriskManager::actionShowDialPlan()
{
}

void AsteriskManager::actionSIPnotify()
{
}

void AsteriskManager::actionSIPpeers()
{
}

void AsteriskManager::actionSIPpeerstatus()
{
}

void AsteriskManager::actionSIPqualifypeer()
{
}

void AsteriskManager::actionSIPshowpeer()
{
}

void AsteriskManager::actionSIPshowregistry()
{
}

void AsteriskManager::actionSKINNYdevices()
{
}

void AsteriskManager::actionSKINNYlines()
{
}

void AsteriskManager::actionSKINNYshowdevice()
{
}

void AsteriskManager::actionSKINNYshowline()
{
}

void AsteriskManager::actionStatus()
{
}

void AsteriskManager::actionStopMixMonitor()
{
}

void AsteriskManager::actionStopMonitor()
{
}

void AsteriskManager::actionUnpauseMonitor()
{
}

void AsteriskManager::actionUpdateConfig()
{
}

void AsteriskManager::actionUserEvent()
{
}

void AsteriskManager::actionVoicemailUsersList()
{
}

void AsteriskManager::actionWaitEvent ()
{
}

QString AsteriskManager::valueToString(QVariant value)
{
    switch (value.type()) {
    case QMetaType::Bool:
        return value.toBool() ? "true" : "false";
    default:
        return value.toString();
    }
}

QVariant AsteriskManager::stringValue(QString string)
{
    return QVariant(string);
}

AsteriskManager::Action AsteriskManager::fetchAction(QString actionID)
{
    Action action;

    if (actions.contains(actionID)) {
        action = actions[actionID];

        actions.remove(actionID);
    }

    return action;
}

void AsteriskManager::insertNotEmpty(QVariantMap *headers, QString key, QVariant value)
{
    bool isEmpty = false;

    switch (value.type()) {
    case QMetaType::UInt:
        isEmpty = value.toUInt() == 0;
        break;
    default:
        isEmpty = value.isNull();
        break;
    }

    if (!isEmpty)
        headers->insert(key, value);
}

void AsteriskManager::dispatchPacket()
{
    if (packetBuffer.contains("Response")) {
        Action action = fetchAction(packetBuffer["ActionID"].toString());
        Response response = (Response) responseEnum.keyToValue(packetBuffer["Response"].toByteArray().data());

        emit responseSent(action, response, packetBuffer);
    } else if (packetBuffer.contains("Event")) {
        Event event = (Event) eventEnum.keyToValue(packetBuffer["Event"].toByteArray().data());

        emit eventGenerated(event, packetBuffer);
    }

    packetBuffer.clear();
}

void AsteriskManager::sendAction(AsteriskManager::Action action, QVariantMap headers)
{
    if (state() == QAbstractSocket::ConnectedState) {
        QString actionID = headers["ActionID"].toString();
        if (actionID.isEmpty() || actions.contains(actionID))
            actionID = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());

        actions[actionID] = action;

        headers["ActionID"] = actionID;
        headers["Action"] = actionEnum.valueToKey(action);

        QMapIterator<QString, QVariant> header(headers);
        while (header.hasNext()) {
            header.next();

            write(QString("%1: %2\r\n").arg(header.key(), valueToString(header.value())).toLatin1());
        }

        write("\r\n");
    }
}

void AsteriskManager::onReadyRead()
{
    QRegExp keyValue("^([A-Za-z0-9\\-]+):\\s(.+)$");
    QByteArray line;

    while (canReadLine()) {
        line = readLine();qDebug() << line;

        if (line != "\r\n") {
            if (keyValue.indexIn(line) > -1)
                packetBuffer[keyValue.cap(1)] = stringValue(keyValue.cap(2));
            else if (line.startsWith("Asterisk Call Manager"))
                emit connected(line.replace("Asterisk Call Manager/", QByteArray()));
        } else if (!packetBuffer.isEmpty()) {
            dispatchPacket();
        }
    }
}

