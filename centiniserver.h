#ifndef CENTINISERVER_H
#define CENTINISERVER_H

#include <QObject>
#include <QSettings>
#include <QSqlDatabase>
#include <QTcpServer>
#include <QWebSocketServer>

#include "callbackquery.h"
#include "asteriskmanager.h"
#include "user.h"

class CentiniServer : public QObject
{
	Q_OBJECT

public:
	explicit CentiniServer(QObject *parent = 0);

	void run();

private:
	enum Query {
		CheckUser
	};

	QSettings *settings;
	QSqlDatabase database;
	AsteriskManager *asterisk;
	QTcpServer *tcpServer;
	QWebSocketServer *webSocketServer;
	QHash<QString, User *> agents, supervisors, managers;
	QString loginActionID;

	void executeQuery(CallbackQuery *query);

	void actionLogin(User *user, QString username, QString passwordHash);
	void actionLogout(User *user);
	void actionDial(QString peer, QString number);
	void actionHangup(User *user, QString username);
	void actionSpy(User *user, QString target);
	void actionWhisper(User *user, QString target);
	void actionJoinQueue(QString peer, QString queue);
	void actionPauseQueue(QString peer, QString queue, bool paused);
	void actionLeaveQueue(QString peer, QString queue);

	void callbackLogin(CallbackQuery *query);

	QVariantMap populateUserInfo(User *user);
	void broadcastUserEvent(User *sender, QHash<QString, User *> *receivers, User::Event event, QVariantMap fields);
	void broadcastUserEvent(User *sender, User::Event event, QVariantMap fields);
	void enumerateUserList(User *receiver, QHash<QString, User *> *senders);

private slots:
	void openDatabaseConnection();
	void connectToAsterisk();

	void onDatabaseQueryFinished();

	void onAsteriskConnected(QString version);
	void onAsteriskDisconnected();
	void onAsteriskResponseSent(AsteriskManager::Response response, QVariantMap headers, QString actionID);
	void onAsteriskEventGenerated(AsteriskManager::Event event, QVariantMap headers);

	void onTcpServerNewConnection();
	void onWebSocketServerNewConnection();

	void onUserActionReceived(User::Action action, QVariantMap fields);
	void onUserDisconnected();
	void onUserQueueStateChanged(User::QueueState queueState);
	void onUserPhoneStateChanged(User::PhoneState phoneState);
	void onUserPeerChanged(QString peer);
};

#endif // CENTINISERVER_H
