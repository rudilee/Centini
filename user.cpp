#include <QMetaEnum>
#include <QQueue>

#include "user.h"

User::User(QObject *parent) :
	QObject(parent),
	client_(new QTcpSocket)
{
	timerId = startTimer(15000);
}

User::~User()
{
	delete client_;
}

void User::setUsername(QString username)
{
	username_ = username;

	if (!username.isEmpty())
		killTimer(timerId);
}

QString User::username()
{
	return username_;
}

void User::setFullname(QString fullname)
{
	fullname_ = fullname;
}

QString User::fullname() const
{
	return fullname_;
}

void User::setLevel(User::Level level)
{
	level_ = level;
}

User::Level User::level() const
{
	return level_;
}

void User::setPeer(QString peer)
{
	peer_ = peer;

	QVariantMap fields;
	fields["peer"] = peer;

	sendEvent(User::PeerChanged, fields);

	emit peerChanged(peer);
}

QString User::peer()
{
	return peer_;
}

void User::setQueue(QString queue)
{
	queue_ = queue;

	QVariantMap fields;
	fields["queue"] = queue;

	sendEvent(User::QueueChanged, fields);
}

QString User::queue()
{
	return queue_;
}

void User::setQueueState(User::QueueState queueState)
{
	queueState_ = queueState;

	emit queueStateChanged(queueState);
}

User::QueueState User::queueState() const
{
	return queueState_;
}

void User::setPhoneState(User::PhoneState phoneState)
{
	phoneState_ = phoneState;

	emit phoneStateChanged(phoneState);
}

User::PhoneState User::phoneState()
{
	return phoneState_;
}

void User::initializeClient(qintptr socketDescriptor)
{
	client_->setSocketDescriptor(socketDescriptor);
}

void User::sendResponse(User::Action action, QVariantMap fields)
{
	fields["type"] = "Response";
	fields["response"] = enumText("Action", action);

	sendMessage(fields);
}

void User::sendEvent(User::Event event, QVariantMap fields)
{
	fields["type"] = "Event";
	fields["event"] = enumText("Event", event);

	sendMessage(fields);
}

void User::clearSession()
{
	username_.clear();
	fullname_.clear();
	peer_.clear();
	queue_.clear();
}

QString User::enumText(QString enumName, int index)
{
	const QMetaObject *object = metaObject();

	return object->enumerator(object->indexOfEnumerator(enumName.toLatin1().data())).valueToKey(index);
}

int User::enumIndex(QString enumName, QString text)
{
	const QMetaObject *object = metaObject();

	return object->enumerator(object->indexOfEnumerator(enumName.toLatin1().data())).keysToValue(text.toLatin1().data());
}

void User::timerEvent(QTimerEvent *event)
{
	disconnect();
}
