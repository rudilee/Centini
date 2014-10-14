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

void CentiniServer::removeAction(QString action)
{
	if (actionId == actionIds.value(action))
		actionIds.remove(action);
}

void CentiniServer::addQueueMember(QVariantMap headers)
{
	QString queue = headers["Queue"].toString(),
			location = headers["Location"].toString();

	if (!location.isEmpty()) {
		queueMembers[queue] << location;

		User *user = lookupUser(sipPeers.key(location));

		if (user != NULL) {
			user->setQueue(queue);
			user->setQueueState(User::Joined);
		}
	}
}

void CentiniServer::pauseQueueMember(QvariantMap headers)
{
	QString location = headers["Location"].toString();

	if (!location.isEmpty()) {
		User *user = lookupUser(sipPeers.key(location));

		if (user != NULL)
			user->setQueueState(headers["Paused"].toUInt() == 1 ? User::Paused : User::Joined, headers["Reason"].toString());
	}
}

void CentiniServer::removeQueueMember(QVariantMap headers)
{
	QString queue = headers["Queue"].toString(),
			location = headers["Location"].toString();

	if (!location.isEmpty()) {
		queueMembers.value(queue).removeAll(location);

		User *user = lookupUser(sipPeers.key(location));

		if (user != NULL) {
			user->setQueue(QString());
			user->setQueueState(User::None);
		}
	}
}

User *CentiniServer::lookupUser(QString ipAddress)
{
	QString username = users.value(ipAddress);

	if (agents.contains(username))
		return agents[username];
	else if (supervisors.contains(username))
		return supervisors[username];
	else if (managers.contains(username))
		return managers[username];

	return NULL;
}

User::PhoneState CentiniServer::phoneStateOf(int channelState)
{
	switch (channelState) {
	case 0: // Down
	case 1: // Rsrvd
	case 2: // OffHook
		break;
	case 3: // Dialing
	case 4: // Ring
	case 5: // Ringing
		return User::Ringing;
		break;
	case 6: // Up
	case 7: // Busy
		return User::Busy;
		break;
	case 8: // Dialing Offhook
	case 9: // Pre-ring
	case 10: // Unknown
	default:
		break;
	}

	return User::Clear;
}

QString CentiniServer::channelPeer(QString channel)
{
	QRegExp channelPattern("^(.+)\\-.+$");
	QString peer;

	if (channelPattern.indexIn(channel) > -1)
		peer = channelPattern.cap(1);

	return peer;
}

QDateTime CentiniServer::durationLastCall(QString duration)
{
	QDateTime lastCall = QDateTime();
	QRegExp durationPattern("^([0-9]{2}):([0-9]{2}):([0-9]{2})$");

	if (durationPattern.indexIn(duration) > -1) {
		int hours = durationPattern.cap(1).toInt(),
			minutes = durationPattern.cap(2).toInt(),
			seconds = durationPattern.cap(3).toInt();

		lastCall = QDateTime::currentDateTime().addSecs((hours * 3600 + minutes * 60 + seconds) * -1);
	}

	return lastCall;
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

		QString peer = sipPeers.value(user->ipAddress()),
				channel = channels.key(peer);

		user->setUsername(username);
		user->setFullname(query->value("fullname").toString());
		user->setLevel((User::Level) user->levelIndex(query->value("level").toString()));
		user->setLastCall(channelLastCalls.value(channel));
		user->setPeer(peer);
		user->setPhoneState(phoneStateOf(channelStates.value(channel)));

		connect(user, SIGNAL(peerChanged(QString)), SLOT(onUserPeerChanged(QString)));
		connect(user, SIGNAL(phoneStateChanged(User::PhoneState)), SLOT(onUserPhoneStateChanged(User::PhoneState)));
		connect(user, SIGNAL(queueStateChanged(User::QueueState)), SLOT(onUserQueueStateChanged(User::QueueState)));

		users[user->ipAddress()] = username;

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
	fields["level"] = user->levelText();
	fields["peer"] = user->peer();
	fields["phone_state"] = user->phoneStateText();
	fields["queue_state"] = user->queueStateText();

	QDateTime lastCall = user->lastCall();

	if (!lastCall.isNull())
		fields["duration"] = lastCall.secsTo(QDateTime::currentDateTime());

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
	actionIds["Login"] = asterisk->actionLogin(settings->value("asterisk/username", "monitor").toString(),
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
	if (actionID == actionIds.take("Login")) {
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
	QString actionId = headers["ActionID"].toString();

	switch (event) {
	case AsteriskManager::FullyBooted:
		actionIds["SIPpeers"] = asterisk->actionSIPpeers();
		actionIds["CoreShowChannels"] = asterisk->actionCoreShowChannels();
		actionIds["QueueStatus"] = asterisk->actionQueueStatus();

		break;
	case AsteriskManager::PeerEntry:
		if (actionId == actionIds.value("SIPpeers"))
			sipPeers[headers["IPaddress"].toString()] = QString("%1/%2").arg(headers["Channeltype"].toString(), headers["ObjectName"].toString());

		break;
	case AsteriskManager::PeerlistComplete:
		removeAction("SIPpeers");

		break;
	case AsteriskManager::PeerStatus:
	{
		QRegExp addressPattern("^(.+):[0-9]{2,5}$");
		QString ipAddress,
				peer =  headers["Peer"].toString(),
				peerStatus = headers["PeerStatus"].toString();

		if (addressPattern.indexIn(headers["Address"].toString()) > -1)
			ipAddress = addressPattern.cap(1);
		else
			ipAddress = sipPeers.key(peer);

		User *user = lookupUser(ipAddress);

		if (user != NULL)
			user->setPeer(peerStatus == "Registered" ? peer : QString());

		if (peerStatus == "Unregistered")
			sipPeers.remove(ipAddress);
		else
			sipPeers[ipAddress] = peer;
	}

		break;
	case AsteriskManager::CoreShowChannel:
	{
		if (actionId == actionIds.value("CoreShowChannels")) {
			QString channel = headers["Channel"].toString();

			channels[channel] = channelPeer(channel);
			channelStates[channel] = headers["ChannelState"].toInt();
			channelLastCalls[channel] = durationLastCall(headers["Duration"].toString());
		}
	}

		break;
	case AsteriskManager::CoreShowChannelsComplete:
		removeAction("CoreShowChannels");

		break;
	case AsteriskManager::Newchannel:
	case AsteriskManager::Newstate:
	{
		QString channel = headers["Channel"].toString(),
				peer = channelPeer(channel);

		int channelState = headers["ChannelState"].toInt();

		channels[channel] = peer;
		channelStates[channel] = channelState;

		User::PhoneState phoneState = phoneStateOf(channelState);
		User *user = lookupUser(sipPeers.key(peer));

		if (user != NULL) {
			if (user->phoneState() != phoneState)
				user->setPhoneState(phoneState);
		}
	}

		break;
	case AsteriskManager::Hangup:
	{
		QString channel = headers["Channel"].toString();

		channels.remove(channel);
		channelStates.remove(channel);
		channelLastCalls.remove(channel);

		User *user = lookupUser(sipPeers.key(channels.value(channel)));

		if (user != NULL) {
			user->setLastCall(QDateTime());
			user->setPhoneState(User::Clear);
		}
	}

		break;
	case AsteriskManager::QueueParams:
		if (actionId == actionIds.value("QueueStatus")) {
			QString queue = headers["queue"].toString();

			if (!queue.isEmpty())
				queues << queue;
		}

		break;
	case AsteriskManager::QueueMember:
		if (actionId == actionIds.value("QueueStatus"))
			addQueueMember(headers);

		break;
	case AsteriskManager::QueueStatusComplete:
		removeAction("QueueStatus");

		break;
	case AsteriskManager::QueueMemberAdded:
		addQueueMember(headers);

		break;
	case AsteriskManager::QueueMemberPaused:
		break;
	case AsteriskManager::QueueMemberRemoved:
		removeQueueMember(headers);

		break;
	case AsteriskManager::QueueMemberStatus:
		break;
	case AsteriskManager::RTCPReceived:
	case AsteriskManager::RTCPSent:
	case AsteriskManager::Newexten:
	case AsteriskManager::NewCallerid:
	case AsteriskManager::VarSet:
		break;
	default:
		qDebug() << "==< Event:" << asterisk->eventValue(event);

		QMapIterator<QString, QVariant> header(headers);

		while (header.hasNext()) {
			header.next();

			qDebug() << header.key() << header.value();
		}

		qDebug("==>");

		break;
	}
}

void CentiniServer::onTcpServerNewConnection()
{
	QVariantMap fields;
	fields["version"] = "0.0.1";

	DesktopUser *user = new DesktopUser;

	connect(user, SIGNAL(actionReceived(User::Action,QVariantMap)), SLOT(onUserActionReceived(User::Action,QVariantMap)));
	connect(user, SIGNAL(disconnected()), SLOT(onUserDisconnected()));

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
	QString username = user->username();

	if (!username.isEmpty()) {
		users.remove(user->ipAddress());

		switch (user->level()) {
		case User::Manager:
			managers.remove(username);
			break;
		case User::Supervisor:
			supervisors.remove(username);
			break;
		default:
			agents.remove(username);

			QVariantMap fields;
			fields["username"] = username;

			broadcastUserEvent(user, &supervisors, User::LoggedOut, fields);

			break;
		}
	}

	user->deleteLater();

	qDebug() << "User disconnected, username:" << username;
}

void CentiniServer::onUserPeerChanged(QString peer)
{
	User *user = (User *) sender();

	QVariantMap fields;
	fields["username"] = user->username();
	fields["peer"] = peer;

	broadcastUserEvent(user, User::PeerChanged, fields);
}

void CentiniServer::onUserPhoneStateChanged(User::PhoneState phoneState)
{
	User *user = (User *) sender();

	QVariantMap fields;
	fields["username"] = user->username();
	fields["phone_state"] = user->phoneStateText(phoneState);

	broadcastUserEvent(user, User::PhoneStateChanged, fields);
}

void CentiniServer::onUserQueueStateChanged(User::QueueState queueState)
{
	User *user = (User *) sender();

	QVariantMap fields;
	fields["username"] = user->username();
	fields["queue_state"] = user->queueStateText(queueState);

	QString pauseReason = user->pauseReason();

	if (!pauseReason.isEmpty())
		fields["pause_reason"] = pauseReason;

	broadcastUserEvent(user, User::QueueStateChanged, fields);
}
