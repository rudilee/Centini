#ifndef DESKTOPUSER_H
#define DESKTOPUSER_H

#include <QTcpSocket>

#include "user.h"

class DesktopUser : public User
{
	Q_OBJECT

public:
	explicit DesktopUser(QObject *parent = 0);

	void setUsername(QString username);

	void setSocket(QTcpSocket *socket);

	QString ipAddress();

	void disconnect();

protected:
	int timerId;

	void timerEvent(QTimerEvent *event);

private:
	QTcpSocket *socket_;
	QByteArray messageBuffer;

	void parseMessageBuffer();
	void sendMessage(QVariantMap fields);

private slots:
	void onSocketError(QAbstractSocket::SocketError socketError);
	void onSocketReadyRead();
};

#endif // DESKTOPUSER_H
