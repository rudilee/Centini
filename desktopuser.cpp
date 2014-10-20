#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>

#include "desktopuser.h"

DesktopUser::DesktopUser(QObject *parent) :
	User(parent)
{
	qDebug("Desktop User created");
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
	qDebug() << QString("TCP Socket on %1 disconnecting").arg(ipAddress());

	socket_->flush();
	socket_->disconnectFromHost();
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
	// TODO: proses input yg lebih dari 1 baris, yg ini gak bener
	while (socket_->canReadLine()) {
		QByteArray line = socket_->readLine();

		if (line != "\r\n") {
			messageBuffer += line;
		} else {
			if (!parseMessageFields(QJsonDocument::fromJson(messageBuffer).object().toVariantMap()))
				socket_->write("Unrecognized command..\n");

			messageBuffer.clear();
		}
	}
}
