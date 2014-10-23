#ifndef CENTINISERVER_H
#define CENTINISERVER_H

#include <QObject>
#include <QSettings>
#include <QSqlDatabase>
#include <QTcpServer>
#include <QWebSocketServer>

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

	QHash<QString, QString> actionIDs; // ActionID, Action
	QHash<QString, User::Action> userActions; // ActionID, User Action

	QHash<QString, QString> sipPeers; // IP Address, Peer
	QHash<QString, QString> peerContexts; // Peer, Context

	QHash<QString, QString> channels; // Channel, Peer
	QHash<QString, int> channelStates; // Channel, Channel State
	QHash<QString, QDateTime> channelLastCalls; // Channel, Last Call
	QHash<QString, QString> channelLinks; // Channel, Channel

	QStringList queues;
	QHash<QString, QStringList> queueMembers; // Queue, Peers
	QHash<QString, QHash<QString, User::QueueState> > queueMemberStates; // Queue, (Peer, Queue State)

	QHash<QString, QString> users; // IP Address, Username
	QHash<QString, User *> agents, supervisors, managers;

	void addAction(QString action, QString actionID, User::Action userAction = User::InvalidAction);
	void removeAction(QString action, QString actionID);

	void addSipPeer(QVariantMap headers);

	void addChannel(QVariantMap headers);
	void removeChannel(QVariantMap headers);

	void dispatchChannelLink(QVariantMap headers);

	void addQueue(QString queue);
	void addQueueMember(QVariantMap headers);
	void pauseQueueMember(QVariantMap headers);
	void removeQueueMember(QVariantMap headers);

    bool invalidResponse(User *user, bool isRequest = false);

	User *lookupUser(QString ipAddress);
	User::PhoneState phoneStateOf(int channelState);
	QString channelPeer(QString channel);
	QDateTime durationLastCall(QString duration);
	QStringList lookupQueue(QString peer);
	QString lookupCounterpart(QString channel);
	bool groupPermited(User *sender, User *receiver);

    void actionLogin(User *user, QString username, QString password);
	void actionLogout(User *user);
	void actionDial(User *user, QString number);
	void actionHangup(User *user, QString target);
	void actionTransfer(User *user, QString destination);
	void actionSendDigit(User *user, QString digit);
	void actionListen(User *user, QString target, QChar option = QChar(), QString callerId = "Listen");
    void actionWhisper(User *user, QString target);
	void actionBarge(User *user, QString target);
    void actionPause(User *user, bool paused, QString reason);

    void requestStatus(User *user);
    void requestChangePassword(User *user, QString username, QString newPassword, QString currentPassword = QString());

	QVariantMap populateUserInfo(User *user);

	void enumerateUserList(User *receiver, QHash<QString, User *> *senders);
	void broadcastUserEvent(User *sender, QHash<QString, User *> *receivers, User::Event event, QVariantMap fields);
	void broadcastUserEvent(User *sender, User::Event event, QVariantMap fields);

private slots:
	void openDatabaseConnection();
	void connectToAsterisk();

	void onAsteriskConnected(QString version);
	void onAsteriskDisconnected();
	void onAsteriskResponseSent(AsteriskManager::Response response, QVariantMap headers, QString actionID);
	void onAsteriskEventGenerated(AsteriskManager::Event event, QVariantMap headers);

	void onTcpServerNewConnection();
	void onWebSocketServerNewConnection();

	void onUserActionReceived(User::Action action, QVariantMap fields);
    void onUserRequestReceived(User::Request request, QVariantMap fields);
	void onUserDisconnected();
	void onUserPeerChanged(QString peer);
	void onUserPhoneStateChanged(User::PhoneState phoneStateOf);
	void onUserQueueStateChanged(User::QueueState queueState);
};

#endif // CENTINISERVER_H
