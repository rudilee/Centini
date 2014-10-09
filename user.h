#ifndef USER_H
#define USER_H

#include <QObject>
#include <QTcpSocket>

class User : public QObject
{
	Q_OBJECT

	Q_ENUMS(Level)
	Q_ENUMS(QueueState)
	Q_ENUMS(PhoneState)
	Q_ENUMS(Action)
	Q_ENUMS(Event)

public:
	explicit User(QObject *parent = 0);
	~User();

	enum Level {
		Agent,
		Supervisor,
		Manager
	};

	enum QueueState {
		None,
		Joined,
		Paused
	};

	enum PhoneState {
		Clear,
		Ringing,
		Busy
	};

	enum Action {
		Invalid = -1,
		Login,
		Logout,
		Dial,
		Hangup,
		Spy,
		Whisper,
		JoinQueue,
		PauseQueue,
		LeaveQueue
	};

	enum Event {
		ActionReady,
		LoggedIn,
		LoggedOut,
		PeerChanged,
		QueueChanged,
		QueueStateChanged,
		PhoneStateChanged
	};

	virtual void setUsername(QString username);
	QString username();

	void setFullname(QString fullname);
	QString fullname() const;

	void setLevel(Level level);
	Level level() const;

	void setPeer(QString peer);
	QString peer();

	void setQueue(QString queue);
	QString queue();

	void setQueueState(QueueState queueState);
	QueueState queueState() const;

	void setPhoneState(PhoneState phoneState);
	PhoneState phoneState();

	virtual QString ipAddress() { return QString(); }

	void initializeClient(qintptr socketDescriptor);
	void sendResponse(User::Action action, QVariantMap fields);
	void sendEvent(User::Event event, QVariantMap fields);
	void clearSession();
	virtual void disconnect() {}

	QString enumText(QString enumName, int index);
	int enumIndex(QString enumName, QString text);

private:
	QString username_, fullname_, peer_, queue_;
	Level level_;
	QueueState queueState_;
	PhoneState phoneState_;

	QTcpSocket *client_;
	QByteArray messageBuffer;

	virtual void sendMessage(QVariantMap fields) { Q_UNUSED(fields) }

signals:
	void connected();
	void disconnected();
	void error(QString message);
	void actionReceived(User::Action action, QVariantMap fields);
	void queueStateChanged(User::QueueState queueState);
	void phoneStateChanged(User::PhoneState phoneState);
	void peerChanged(QString peer);
};

#endif // USER_H
