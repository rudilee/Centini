#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>

#include "desktopuser.h"

DesktopUser::DesktopUser(QObject *parent) :
	User(parent)
{
	timerId = startTimer(15000);
}

void DesktopUser::setUsername(QString username)
{
	User::setUsername(username);

	if (!username.isEmpty())
		killTimer(timerId);
}

void DesktopUser::setSocket(QTcpSocket *socket)
{
	socket_ = socket;

	connect(socket_, SIGNAL(connected()), SIGNAL(connected()));
	connect(socket_, SIGNAL(disconnected()), SIGNAL(disconnected()));
	connect(socket_, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(onSocketError(QAbstractSocket::SocketError)));
	connect(socket_, SIGNAL(readyRead()), SLOT(onSocketReadyRead()));
}

QString DesktopUser::ipAddress()
{
	return socket_->peerAddress().toString();
}

void DesktopUser::disconnect()
{
	socket_->flush();
	socket_->disconnectFromHost();
}

void DesktopUser::timerEvent(QTimerEvent *event)
{
	disconnect();
}

void DesktopUser::parseMessageBuffer()
{
	QVariantMap fields = QJsonDocument::fromJson(messageBuffer).object().toVariantMap();
	QString actionText = fields.take("action").toString();

	if (!actionText.isEmpty())
		emit actionReceived((User::Action) enumIndex("Action", actionText), fields);

	messageBuffer.clear();
}

void DesktopUser::sendMessage(QVariantMap fields)
{
	if (socket_->state() != QTcpSocket::ConnectedState)
		return;

	QJsonDocument message;
	message.setObject(QJsonObject::fromVariantMap(fields));

	socket_->write(message.toJson(/*QJsonDocument::Compact*/).append("\r\n"));
}

void DesktopUser::onSocketError(QAbstractSocket::SocketError socketError)
{
	emit error("Socket Connection Error.");
}

void DesktopUser::onSocketReadyRead()
{
	while (socket_->canReadLine()) {
		QByteArray line = socket_->readLine();

		if (line != "\r\n")
			messageBuffer += line;
		else
			parseMessageBuffer();
	}
}
