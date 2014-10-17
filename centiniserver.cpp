#include <QCoreApplication>
#include <QTimer>
#include <QSqlQuery>
#include <QSqlError>
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

void CentiniServer::addAction(QString action, QString actionID, User::Action userAction)
{
	if (!actionID.isEmpty()) {
		actionIDs[actionID] = action;

		if (userAction != User::Invalid)
			userActions[actionID] = userAction;
	}
}

void CentiniServer::removeAction(QString action, QString actionID)
{
	if (actionID == actionIDs.key(action))
		actionIDs.remove(actionIDs.key(action));
}

void CentiniServer::addSipPeer(QVariantMap headers)
{
	QRegExp addressPattern("^(.+):[0-9]{2,5}$");
	QString ipAddress,
			peer;

	bool registered = headers.contains("PeerStatus") ? headers["PeerStatus"].toString() == "Registered" : true;

	if (headers.contains("IPaddress")) {
		ipAddress = headers["IPaddress"].toString();
	} else {
		if (addressPattern.indexIn(headers["Address"].toString()) > -1)
			ipAddress = addressPattern.cap(1);
		else
			ipAddress = sipPeers.key(peer);
	}

	if (headers.contains("Peer"))
		peer = peer = headers["Peer"].toString();
	else
		peer = QString("%1/%2").arg(headers["Channeltype"].toString(), headers["ObjectName"].toString());

	if (!ipAddress.isEmpty() && !peer.isEmpty()) {
		User *user = lookupUser(ipAddress);

		if (user != NULL)
			user->setPeer(registered ? peer : QString());
	}

	if (registered)
		sipPeers[ipAddress] = peer;
	else
		sipPeers.remove(ipAddress);

	if (!peer.isEmpty())
		addAction(peer, asterisk->actionSIPshowpeer(QString(peer).remove("SIP/")));
}

void CentiniServer::addChannel(QVariantMap headers)
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

void CentiniServer::removeChannel(QVariantMap headers)
{
	QString channel = headers["Channel"].toString(),
			ipAddress = sipPeers.key(channels.value(channel));

	if (!ipAddress.isEmpty()) {
		User *user = lookupUser(ipAddress);

		if (user != NULL) {
			user->setLastCall(QDateTime());
			user->setPhoneState(User::Clear);
		}
	}

	channels.remove(channel);
	channelStates.remove(channel);
	channelLastCalls.remove(channel);
}

void CentiniServer::dispatchChannelLink(QVariantMap headers)
{
	bool isLink = headers["Bridgestate"].toString() == "Link";

	QString channel1 = headers["Channel1"].toString(),
			channel2 = headers["Channel2"].toString();

	if (isLink)
		channelLinks[channel1] = channel2;
	else
		channelLinks.remove(channel1);
}

void CentiniServer::addQueue(QString queue)
{
	if (!queue.isEmpty()) {
		queues << queue;

		if (!queueMemberStates.contains(queue))
			queueMemberStates[queue] = QHash<QString, User::QueueState>();
	}
}

void CentiniServer::addQueueMember(QVariantMap headers)
{
	QString queue = headers["Queue"].toString(),
			location = headers["Location"].toString();

	if (!location.isEmpty()) {
		if (!queueMembers[queue].contains(location))
			queueMembers[queue] << location;

		User::QueueState queueState = User::Joined;

		if (headers.contains("Paused")) {
			if (headers["Paused"].toUInt() == 1)
				queueState = User::Paused;
		}

		queueMemberStates[queue][location] = queueState;

		User *user = lookupUser(sipPeers.key(location));

		if (user != NULL) {
			user->addQueue(queue);
			user->setQueueState(queue, queueState);
		}
	}
}

void CentiniServer::pauseQueueMember(QVariantMap headers)
{
	QString location = headers["Location"].toString();

	if (!location.isEmpty()) {
		QString queue = headers["Queue"].toString();
		User::QueueState queueState = headers["Paused"].toUInt() == 1 ? User::Paused : User::Joined;

		queueMemberStates[queue][location] = queueState;

		User *user = lookupUser(sipPeers.key(location));

		if (user != NULL)
			user->setQueueState(queue, queueState);
	}
}

void CentiniServer::removeQueueMember(QVariantMap headers)
{
	QString queue = headers["Queue"].toString(),
			location = headers["Location"].toString();

	if (!location.isEmpty()) {
		if (queueMembers.contains(queue))
			queueMembers[queue].removeAll(location);

		queueMemberStates[queue][location] = User::None;

		User *user = lookupUser(sipPeers.key(location));

		if (user != NULL) {
			user->removeQueue(queue);
			user->setQueueState(queue, User::None);
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

QStringList CentiniServer::lookupQueue(QString peer)
{
	QStringList queues;

	QHashIterator<QString, QStringList> members(queueMembers);
	while (members.hasNext()) {
		members.next();

		if (members.value().contains(peer))
			queues << members.key();
	}

	return queues;
}

QString CentiniServer::lookupCounterpart(QString channel)
{
	QString counterpart = channelLinks.value(channel);

	if (counterpart.isEmpty())
		counterpart = channelLinks.key(channel);

	return counterpart;
}

bool CentiniServer::groupPermited(User *sender, User *receiver)
{
	// Pastiin kalo Pengirim sama Penerima User yg beda [1],
	// terus Pengirim ada di Group yg sama dg Penerima [2],
	// atau Penerima punya Level Manager [3]

	return /*1*/(sender != receiver) && (/*2*/!receiver->groups().toSet().intersect(sender->groups().toSet()).isEmpty() || /*3*/receiver->level() == User::Manager);
}

void CentiniServer::actionLogin(User *user, QString username, QString passwordHash)
{
	QString message = "Login failed, Username or IP Address already used.";
	bool success = !users.values().contains(username) && !users.contains(user->ipAddress());

	if (success) {
		QSqlQuery query;
		query.prepare("SELECT * FROM users WHERE username = :username AND password = :password");
		query.bindValue(":username", username);
		query.bindValue(":password", passwordHash);

		success = query.exec();

		if (success) {
			success = query.next();

			if (success) {
				message = "Successfuly logged in.";

				QString peer = sipPeers.value(user->ipAddress()),
						channel = channels.key(peer);

				user->setUsername(username);
				user->setFullname(query.value("fullname").toString());
				user->setLevel((User::Level) user->levelIndex(query.value("level").toString()));
				user->setLastCall(channelLastCalls.value(channel));
				user->setPeer(peer);
				user->setPhoneState(phoneStateOf(channelStates.value(channel)));

				QStringList queues = lookupQueue(peer);

				if (!queues.isEmpty()) {
					user->setQueues(queues);

					foreach (QString queue, queues) {
						user->setQueueState(queue, queueMemberStates[queue].value(peer));
					}
				}

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
					broadcastUserEvent(user, &managers, User::LoggedIn, populateUserInfo(user));

					break;
				default:
					agents[username] = user;

					broadcastUserEvent(user, &managers, User::LoggedIn, populateUserInfo(user));
					broadcastUserEvent(user, &supervisors, User::LoggedIn, populateUserInfo(user));

					break;
				}
			}
		} else {
			message = "Login failed due to system error.";

			qCritical() << "Login query failed, error:" << query.lastError().text();
		}
	}

	QVariantMap response;
	response["message"] = message;

	user->sendResponse(User::Login, success, response);
}

void CentiniServer::actionLogout(User *user)
{
	QVariantMap fields;
	fields["username"] = user->username();

	broadcastUserEvent(user, User::LoggedOut, fields);

	QVariantMap response;
	response["message"] = "Bye!";

	user->sendResponse(User::Logout, true, response);
	user->disconnect();
}

void CentiniServer::actionDial(User *user, QString number)
{
	QString peer = user->peer();

	addAction(user->username(), asterisk->actionOriginate(peer, number, peerContexts.value(peer), 1, QString(), QString(), 0, number), User::Dial);
}

void CentiniServer::actionHangup(User *user, QString target)
{
	QString username = user->username();
	bool isSelf = username == target;

	if (!isSelf ? (user->level() != User::Agent) : true) {
		QString peer;

		if (isSelf) {
			peer = user->peer();
		} else {
			User *otherUser = lookupUser(users.key(target));

			if (otherUser != NULL)
				peer = otherUser->peer();
		}

		QStringList channels = this->channels.keys(peer);

		if (!channels.isEmpty()) {
			foreach (QString channel, channels) {
				addAction(username, asterisk->actionHangup(channel), User::Hangup);
			}
		}
	}
}

void CentiniServer::actionTransfer(User *user, QString destination)
{
	QString peer = user->peer(),
			channel = lookupCounterpart(channels.key(peer)),
			context = peerContexts.value(peer);

	if (context.isEmpty())
		context = settings->value("asterisk/outgoing_context", "outgoing").toString();

	if (!channel.isEmpty())
		addAction(user->username(), asterisk->actionRedirect(channel, destination, context, 1), User::Transfer);
}

void CentiniServer::actionSendDigit(User *user, QChar digit)
{
	QString channel = lookupCounterpart(channels.key(user->peer()));

	if (!channel.isEmpty())
		addAction(user->username(), asterisk->actionPlayDTMF(channel, digit), User::SendDigit);
}

void CentiniServer::actionListen(User *user, QString target, QChar option, QString callerId)
{
	QString username = user->username();

	if (username != target && user->level() != User::Agent) {
		QString peer = user->peer(),
				context = peerContexts.value(peer),
				targetPeer = sipPeers.value(users.key(target)),
				data = QString(",Eq%1").arg(option).prepend(targetPeer);

		if (!targetPeer.isEmpty())
			addAction(username, asterisk->actionOriginate(peer, QString(), context, 0, "ExtenSpy", data, 0, callerId), User::Listen);
	}
}

void CentiniServer::actionWhisper(User *user, QString target)
{
	actionListen(user, target, 'w', "Whisper");
}

void CentiniServer::actionBarge(User *user, QString target)
{
	actionListen(user, target, 'B', "Barge");
}

void CentiniServer::actionPause(User *user, bool paused, QString reason)
{
    QString peer = user->peer();

    foreach (QString queue, user->queues()) {
        asterisk->actionQueuePause(peer, paused, queue, reason);
    }

	user->setPauseReason(reason);

    if (paused)
        user->startPause();
    else
        user->finishPause();
}

QVariantMap CentiniServer::populateUserInfo(User *user)
{
	QVariantMap fields;
	fields["username"] = user->username();
	fields["fullname"] = user->fullname();
	fields["level"] = user->levelText();
	fields["groups"] = user->groups();
	fields["peer"] = user->peer();
	fields["phone_state"] = user->phoneStateText();
	fields["queue_state"] = user->queueStateText();

	QDateTime lastCall = user->lastCall();

	if (!lastCall.isNull())
		fields["duration"] = lastCall.secsTo(QDateTime::currentDateTime());

	return fields;
}

void CentiniServer::enumerateUserList(User *receiver, QHash<QString, User *> *senders)
{
	foreach (QString username, senders->keys()) {
		User *sender = senders->value(username);

		if (groupPermited(sender, receiver))
			receiver->sendEvent(User::LoggedIn, populateUserInfo(sender));
	}
}

void CentiniServer::broadcastUserEvent(User *sender, QHash<QString, User *> *receivers, User::Event event, QVariantMap fields)
{
	foreach (QString username, receivers->keys()) {
		User *receiver = receivers->value(username);

		if (groupPermited(sender, receiver))
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

void CentiniServer::connectToAsterisk()
{
	asterisk->connectToHost(settings->value("asterisk/host", "192.168.1.8").toString(),
							settings->value("asterisk/port", 5038).toUInt());

	qDebug("Connecting to Asterisk server");
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
	QString actionID = asterisk->actionLogin(settings->value("asterisk/username", "monitor").toString(),
											 settings->value("asterisk/secret", "m0n1t0r").toString());

	addAction("Login", actionID);

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
	if (actionID == actionIDs.take("Login")) {
		switch (response) {
		case AsteriskManager::Success:
			qDebug("Asterisk server connected");

			break;
		case AsteriskManager::Error:
		default:
			qCritical() << "Asterisk connection failed, message:" << headers["Message"].toString();

			break;
		}
	} else if (headers.contains("Context")) {
		QString peer = actionIDs.value(actionID);

		if (!peer.isEmpty()) {
			peerContexts[peer] = headers["Context"].toString();

			actionIDs.remove(peer);
		}
	} else if (userActions.contains(actionID)) {
		QString username = actionIDs.value(actionID);

		if (!username.isEmpty()) {
			User *user = lookupUser(users.key(username));

			if (user != NULL) {
				bool success = headers["Response"].toString() == "Success";

				QVariantMap fields;
				fields["message"] = headers["Message"].toString();

				user->sendResponse(userActions.value(actionID), success, fields);
			}
		}
	}
}

void CentiniServer::onAsteriskEventGenerated(AsteriskManager::Event event, QVariantMap headers)
{
	QString actionId = headers["ActionID"].toString();

	switch (event) {
	case AsteriskManager::FullyBooted:
		addAction("SIPpeers", asterisk->actionSIPpeers());
		addAction("CoreShowChannels", asterisk->actionCoreShowChannels());
		addAction("QueueStatus", asterisk->actionQueueStatus());

		break;
	case AsteriskManager::PeerEntry:
		if (actionId == actionIDs.key("SIPpeers"))
			addSipPeer(headers);

		break;
	case AsteriskManager::PeerlistComplete:
		removeAction("SIPpeers", actionId);

		break;
	case AsteriskManager::PeerStatus:
		addSipPeer(headers);

		break;
	case AsteriskManager::CoreShowChannel:
		if (actionId == actionIDs.key("CoreShowChannels"))
			addChannel(headers);

		break;
	case AsteriskManager::CoreShowChannelsComplete:
		removeAction("CoreShowChannels", actionId);

		break;
	case AsteriskManager::Newchannel:
	case AsteriskManager::Newstate:
		addChannel(headers);

		break;
	case AsteriskManager::Hangup:
	case AsteriskManager::HangupRequest:
	case AsteriskManager::SoftHangupRequest:
		removeChannel(headers);

		break;
	case AsteriskManager::QueueParams:
	case AsteriskManager::QueueEntry:
		if (actionId == actionIDs.key("QueueStatus"))
			addQueue(headers["queue"].toString());

		break;
	case AsteriskManager::QueueMember:
	case AsteriskManager::QueueMemberStatus:
		if (actionId == actionIDs.key("QueueStatus"))
			addQueueMember(headers);

		break;
	case AsteriskManager::QueueStatusComplete:
		removeAction("QueueStatus", actionId);

		break;
	case AsteriskManager::QueueMemberAdded:
		addQueueMember(headers);

		break;
	case AsteriskManager::QueueMemberPaused:
		pauseQueueMember(headers);

		break;
	case AsteriskManager::QueueMemberRemoved:
		removeQueueMember(headers);

		break;
	case AsteriskManager::Bridge:
		dispatchChannelLink(headers);

		break;
	case AsteriskManager::DAHDIChannel:
	case AsteriskManager::Dial:
	case AsteriskManager::RTCPReceived:
	case AsteriskManager::RTCPSent:
	case AsteriskManager::NewAccountCode:
	case AsteriskManager::Newexten:
	case AsteriskManager::NewCallerid:
	case AsteriskManager::VarSet:
		break;
	default:
		if (false) {
			qDebug() << "==< Event:" << asterisk->eventValue(event);

			QMapIterator<QString, QVariant> header(headers);

			while (header.hasNext()) {
				header.next();

				qDebug() << header.key() << header.value();
			}

			qDebug("==>");
		}

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
			response["message"] = "Unauthorized access.";

			user->sendResponse(User::Invalid, false, response);

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
		actionDial(user, fields.contains("username") ? fields["username"].toString() : fields["number"].toString());

		break;
	case User::Hangup:
		actionHangup(user, fields.contains("username") ? fields["username"].toString() : user->username());

		break;
	case User::Transfer:
		actionTransfer(user, fields["destination"].toString());

		break;
	case User::SendDigit:
		actionSendDigit(user, fields["digit"].toChar());

		break;
	case User::Listen:
		actionListen(user, fields["username"].toString());

		break;
	case User::Whisper:
		actionWhisper(user, fields["username"].toString());

        break;
	case User::Barge:
		actionBarge(user, fields["username"].toString());

		break;
    case User::Pause:
		actionPause(user, fields["paused"].toBool(), fields["reason"].toString());

        break;
	default:
		QVariantMap response;
		response["message"] = "Invalid action command.";

		user->sendResponse(action, false, response);

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
