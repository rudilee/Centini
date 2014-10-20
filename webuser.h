#ifndef WEBUSER_H
#define WEBUSER_H

#include <QWebSocket>

#include "user.h"

class WebUser : public User
{
	Q_OBJECT

public:
	explicit WebUser(QObject *parent = 0);

	void setSocket(QWebSocket *socket);

	QString ipAddress();

	void disconnect();

private:
	QWebSocket *socket_;

	void sendMessage(QVariantMap fields);

public slots:
	void onSocketError(QAbstractSocket::SocketError socketError);
	void onSocketTextMessageReceived(QString message);
};

#endif // WEBUSER_H
