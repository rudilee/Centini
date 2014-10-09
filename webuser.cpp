#include <QJsonDocument>
#include <QJsonObject>

#include "webuser.h"

WebUser::WebUser(QObject *parent) :
	User(parent)
{
	qDebug("Web User created");
}

void WebUser::setSocket(QWebSocket *socket)
{
	socket_ = socket;

	connect(socket_, SIGNAL(connected()), SIGNAL(connected()));
	connect(socket_, SIGNAL(disconnected()), SIGNAL(disconnected()));
	connect(socket_, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(onSocketError(QAbstractSocket::SocketError)));
	connect(socket_, SIGNAL(textMessageReceived(QString)), SLOT(onSocketTextMessageReceived(QString)));
}

QString WebUser::ipAddress()
{
	return socket_->peerAddress().toString();
}

void WebUser::disconnect()
{
	qDebug() << QString("WebSocket on %1 disconnecting").arg(ipAddress());

	socket_->flush();
	socket_->close();
}

void WebUser::parseMessage(QString message)
{
	QVariantMap fields = QJsonDocument::fromJson(message.toLatin1()).object().toVariantMap();
	QString actionText = fields.take("action").toString();

	if (!actionText.isEmpty())
		emit actionReceived((User::Action) enumIndex("Action", actionText), fields);
}

void WebUser::sendMessage(QVariantMap fields)
{
	if (socket_->state() != QAbstractSocket::ConnectedState)
		return;

	QJsonDocument message;
	message.setObject(QJsonObject::fromVariantMap(fields));

	socket_->sendTextMessage(message.toJson(/*QJsonDocument::Compact*/));
}

void WebUser::onSocketError(QAbstractSocket::SocketError socketError)
{
	emit error("Socket Connection Error.");
}

void WebUser::onSocketTextMessageReceived(QString message)
{
	if (!message.isEmpty())
		parseMessage(message);
}
