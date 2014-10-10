#include <QCoreApplication>
#include <QTimer>
#include <QSqlError>
#include <QtConcurrent/QtConcurrentRun>
#include <QFuture>
#include <QFutureWatcher>
#include <QUuid>

#include "centiniserver.h"
#include "desktopuser.h"
#include "webuser.h"

CentiniServer::CentiniServer(QObject *parent) :
	QObject(parent),
	settings(new QSettings("/etc/centini.conf", QSettings::IniFormat)),
	asterisk(new AsteriskManager(this)),
	tcpServer(new QTcpServer(this)),
	webSocketServer(new QWebSocketServer(QString("%1 - %2").arg(QCoreApplication::applicationName(), QCoreApplication::applicationVersion()),
										 QWebSocketServer::NonSecureMode))
{
	connect(asterisk, SIGNAL(connected(QString)), SLOT(onAsteriskConnected(QString)));
	connect(asterisk, SIGNAL(disconnected()), SLOT(onAsteriskDisconnected()));
	connect(asterisk, SIGNAL(responseSent(AsteriskManager::Response,QVariantMap,QString)),
			SLOT(onAsteriskResponseSent(AsteriskManager::Response,QVariantMap,QString)));
	connect(asterisk, SIGNAL(eventGenerated(AsteriskManager::Event,QVariantMap)),
			SLOT(onAsteriskEventGenerated(AsteriskManager::Event,QVariantMap)));

	connect(tcpServer, SIGNAL(newConnection()), SLOT(onTcpServerNewConnection()));
	connect(webSocketServer, SIGNAL(newConnection()), SLOT(onWebSocketServerNewConnection()));
}

void CentiniServer::run()
{
	openDatabaseConnection();
	connectToAsterisk();

	tcpServer->listen(QHostAddress::Any, settings->value("centini/port", 31415).toUInt());
	webSocketServer->listen(QHostAddress::Any, settings->value("centini/ws_port", 8080).toUInt());

	qDebug("Listening incoming connection");
}

void CentiniServer::executeQuery(CallbackQuery *query)
{
	QFutureWatcher<CallbackQuery *> *queryWatcher = query->queryWatcher();

	connect(queryWatcher, SIGNAL(finished()), SLOT(onDatabaseQueryFinished()));

	queryWatcher->setFuture(QtConcurrent::run([](CallbackQuery *query) -> CallbackQuery * {
		if (!query->exec())
			qDebug() << "Query failed, error:" << query->lastError().text();

		return query;
	}, query));
}

void CentiniServer::actionLogin(User *user, QString username, QString passwordHash)
{
	CallbackQuery *query = new CallbackQuery(database);
	query->setQueryId(QUuid::createUuid().toString());
	query->setType(CallbackQuery::CheckUserPassword);
	query->setUser(user);
	query->prepare("SELECT * FROM user WHERE password = :password");
	query->bindValue(":password", passwordHash);

	QVariantMap parameters;
	parameters["username"] = username;

	query->setParameters(parameters);

	executeQuery(query);
}

void CentiniServer::actionLogout(User *user)
{
	QVariantMap fields;
	fields["username"] = user->username();

	broadcastUserEvent(user, User::LoggedOut, fields);

	QVariantMap response;
	response["success"] = true;
	response["message"] = "Bye!";

	user->sendResponse(User::Logout, response);
	user->clearSession();
	user->disconnect();
}

void CentiniServer::actionDial(QString peer, QString number)
{
	asterisk->actionOriginate(peer, number);
}

void CentiniServer::actionHangup(User *user, QString username)
{
	if (user->username() != username) {
		if (user->level() != User::Agent)
			;
	}
}

void CentiniServer::actionSpy(User *user, QString target)
{
	if (user->level() != User::Agent)
		;
}

void CentiniServer::actionWhisper(User *user, QString target)
{
	if (user->level() != User::Agent)
		;
}

void CentiniServer::actionJoinQueue(QString peer, QString queue)
{
	asterisk->actionQueueAdd(queue, peer);
}

void CentiniServer::actionPauseQueue(QString peer, QString queue, bool paused)
{
	asterisk->actionQueuePause(peer, paused, queue);
}

void CentiniServer::actionLeaveQueue(QString peer, QString queue)
{
	asterisk->actionQueueRemove(queue, peer);
}

void CentiniServer::callbackLogin(CallbackQuery *query)
{
	QVariantMap response;
	bool success = false;
	QVariantMap parameters = query->parameters();
	User *user = query->user();
	QString username = parameters["username"].toString(),
			message = "Login failed.";

	if (query->next()) {
		success = true;
		message = "Successfuly logged in.";

		user->setUsername(username);
		user->setFullname(query->value("fullname").toString());
		user->setLevel((User::Level) user->enumIndex("Level", query->value("level").toString()));

		switch (user->level()) {
		case User::Manager:
			managers[username] = user;

			enumerateUserList(user, &supervisors);
			enumerateUserList(user, &agents);
			break;
		case User::Supervisor:
			supervisors[username] = user;

			enumerateUserList(user, &agents);
			break;
		default:
			agents[username] = user;

			broadcastUserEvent(user, &supervisors, User::LoggedIn, populateUserInfo(user));
			break;
		}
	} else {
		qDebug() << "Login query failed, error:" << query->lastError().text();
	}

	response["success"] = success;
	response["message"] = message;

	user->sendResponse(User::Login, response);
}

QVariantMap CentiniServer::populateUserInfo(User *user)
{
	QVariantMap fields;
	fields["username"] = user->username();
	fields["fullname"] = user->fullname();
	fields["level"] = user->enumText("Level", user->level());
	fields["queue_state"] = user->enumText("QueueState", user->queueState());
	fields["phone_state"] = user->enumText("PhoneState", user->phoneState());
	fields["peer"] = user->peer();

	return fields;
}

void CentiniServer::broadcastUserEvent(User *sender, QHash<QString, User *> *receivers, User::Event event, QVariantMap fields)
{
	foreach (QString username, receivers->keys()) {
		User *receiver = receivers->value(username);
		if (receiver != sender)
			receiver->sendEvent(event, fields);
	}
}

void CentiniServer::broadcastUserEvent(User *sender, User::Event event, QVariantMap fields)
{
	switch (sender->level()) {
	case User::Agent:
		broadcastUserEvent(sender, &supervisors, event, fields);
	case User::Supervisor:
		broadcastUserEvent(sender, &managers, event, fields);
		break;
	}
}

void CentiniServer::enumerateUserList(User *receiver, QHash<QString, User *> *senders)
{
	foreach (QString username, senders->keys()) {
		User *sender = senders->value(username);
		if (sender != receiver) {
			receiver->sendEvent(User::LoggedIn, populateUserInfo(sender));
		}
	}
}

void CentiniServer::connectToAsterisk()
{
	asterisk->connectToHost(settings->value("asterisk/host", "192.168.1.8").toString(),
							settings->value("asterisk/port", 5038).toUInt());

	qDebug("Connecting to Asterisk server");
}

void CentiniServer::onDatabaseQueryFinished()
{
	CallbackQuery *query = ((QFutureWatcher<CallbackQuery *> *) sender())->result();

	switch (query->type()) {
	case CallbackQuery::CheckUserPassword:
		callbackLogin(query);
		break;
	}

	delete query;
}

void CentiniServer::openDatabaseConnection()
{
	if (database.databaseName().isEmpty()) {
		database = QSqlDatabase::addDatabase("QMYSQL");
		database.setHostName(settings->value("mysql/host", "192.168.1.8").toString());
		database.setPort(settings->value("mysql/port", 3306).toInt());
		database.setDatabaseName(settings->value("mysql/database", "centini").toString());
		database.setUserName(settings->value("mysql/username", "centini").toString());
		database.setPassword(settings->value("mysql/password", "C3nt1n1").toString());
		database.setConnectOptions("MYSQL_OPT_RECONNECT=1");
	}

	if (database.isOpen()) {
		qDebug("MySQL database is already opened");

		return;
	}

	if (database.open()) {
		qDebug() << "MySQL database opened, Database:"
				 << database.databaseName()
				 << "on:"
				 << database.hostName();
	} else {
		uint reconnect = settings->value("mysql/reconnect", 15000).toUInt();

		QTimer::singleShot(reconnect, this, SLOT(openDatabaseConnection()));

		qCritical() << "Failed to open MySQL database:"
					<< database.lastError()
					<< "try to reconnect in"
					<< QString::number(reconnect / 1000) << "seconds";
	}
}

void CentiniServer::onAsteriskConnected(QString version)
{
	loginActionID = asterisk->actionLogin(settings->value("asterisk/username", "monitor").toString(),
										  settings->value("asterisk/secret", "m0n1t0r").toString());

	qDebug() << "Login to Asterisk server, AMI version:" << version;
}

void CentiniServer::onAsteriskDisconnected()
{
	uint reconnect = settings->value("asterisk/reconnect", 15000).toUInt();

	QTimer::singleShot(reconnect, this, SLOT(connectToAsterisk()));

	qCritical() << "Asterisk connection closed, try to reconnect in" << QString::number(reconnect / 1000) << "seconds";
}

void CentiniServer::onAsteriskResponseSent(AsteriskManager::Response response, QVariantMap headers, QString actionID)
{
	if (actionID == loginActionID) {
		switch (response) {
		case AsteriskManager::Success:
			qDebug("Asterisk server connected");
			break;
		case AsteriskManager::Error:
		default:
			qDebug() << "Asterisk connection failed, message:" << headers["Message"].toString();
			break;
		}
	}
}

void CentiniServer::onAsteriskEventGenerated(AsteriskManager::Event event, QVariantMap headers)
{
	if (event == AsteriskManager::FullyBooted) {
		qDebug() << "SIPpeers:" << asterisk->actionSIPpeers();
		qDebug() << "CoreStatus:" << asterisk->actionCoreStatus();
		qDebug() << "CoreShowChannels:" << asterisk->actionCoreShowChannels();
	}
}

void CentiniServer::onTcpServerNewConnection()
{
	QVariantMap fields;
	fields["version"] = "0.0.1";

	DesktopUser *user = new DesktopUser;

	connect(user, SIGNAL(actionReceived(User::Action,QVariantMap)), SLOT(onUserActionReceived(User::Action,QVariantMap)));
	connect(user, SIGNAL(disconnected()), SLOT(onUserDisconnected()));
	connect(user, SIGNAL(queueStateChanged(User::QueueState)), SLOT(onUserQueueStateChanged(User::QueueState)));
	connect(user, SIGNAL(phoneStateChanged(User::PhoneState)), SLOT(onUserPhoneStateChanged(User::PhoneState)));
	connect(user, SIGNAL(peerChanged(QString)), SLOT(onUserPeerChanged(QString)));

	user->setSocket(tcpServer->nextPendingConnection());
	user->sendEvent(User::ActionReady, fields);

	qDebug() << "New TCP Socket incoming connection from"
			 << user->ipAddress();
}

void CentiniServer::onWebSocketServerNewConnection()
{
	QVariantMap fields;
	fields["version"] = "0.0.1";

	WebUser *user = new WebUser;

	connect(user, SIGNAL(actionReceived(User::Action,QVariantMap)), SLOT(onUserActionReceived(User::Action,QVariantMap)));
	connect(user, SIGNAL(disconnected()), SLOT(onUserDisconnected()));
	connect(user, SIGNAL(queueStateChanged(User::QueueState)), SLOT(onUserQueueStateChanged(User::QueueState)));
	connect(user, SIGNAL(phoneStateChanged(User::PhoneState)), SLOT(onUserPhoneStateChanged(User::PhoneState)));
	connect(user, SIGNAL(peerChanged(QString)), SLOT(onUserPeerChanged(QString)));

	user->setSocket(webSocketServer->nextPendingConnection());
	user->sendEvent(User::ActionReady, fields);

	qDebug() << "New WebSocket incoming connection from"
			 << user->ipAddress();
}

void CentiniServer::onUserActionReceived(User::Action action, QVariantMap fields)
{
	User *user = (User *) sender();

	if (action != User::Login) {
		if (user->username().isEmpty()) {
			QVariantMap response;
			response["success"] = false;
			response["message"] = "Unauthorized access.";

			user->sendResponse(User::Invalid, response);

			return;
		}
	}

	switch (action) {
	case User::Login:
		actionLogin(user, fields["username"].toString(), fields["password"].toString());
		break;
	case User::Logout:
		actionLogout(user);
		break;
	case User::Dial:
		actionDial(user->peer(), fields.contains("username") ? fields["username"].toString() : fields["number"].toString());
		break;
	case User::Hangup:
		actionHangup(user, fields.contains("username") ? fields["username"].toString() : user->username());
		break;
	case User::Spy:
		actionSpy(user, fields["username"].toString());
		break;
	case User::Whisper:
		actionWhisper(user, fields["username"].toString());
		break;
	case User::JoinQueue:
		actionJoinQueue(user->peer(), fields["queue"].toString());
		break;
	case User::PauseQueue:
		actionPauseQueue(user->peer(), user->queue(), fields["paused"].toBool());
		break;
	case User::LeaveQueue:
		actionLeaveQueue(user->peer(), user->queue());
		break;
	default:
		QVariantMap response;
		response["success"] = false;
		response["message"] = "Invalid action command.";

		user->sendResponse(action, response);
		break;
	}
}

void CentiniServer::onUserDisconnected()
{
	User *user = (User *) sender();

	switch (user->level()) {
	case User::Manager:
		managers.remove(user->username());
		break;
	case User::Supervisor:
		supervisors.remove(user->username());
		break;
	default:
		agents.remove(user->username());
		break;
	}

	user->deleteLater();
}

void CentiniServer::onUserQueueStateChanged(User::QueueState queueState)
{
	User *user = (User *) sender();

	QVariantMap fields;
	fields["username"] = user->username();
	fields["queue_state"] = user->enumText("QueueState", queueState);

	broadcastUserEvent(user, User::QueueStateChanged, fields);
}

void CentiniServer::onUserPhoneStateChanged(User::PhoneState phoneState)
{
	User *user = (User *) sender();

	QVariantMap fields;
	fields["username"] = user->username();
	fields["phone_state"] = user->enumText("PhoneState", phoneState);

	broadcastUserEvent(user, User::PhoneStateChanged, fields);
}

void CentiniServer::onUserPeerChanged(QString peer)
{
	User *user = (User *) sender();

	QVariantMap fields;
	fields["username"] = user->username();
	fields["peer"] = peer;

	broadcastUserEvent(user, User::PeerChanged, fields);
}
