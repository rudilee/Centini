#ifndef USER_H
#define USER_H

#include <QObject>
#include <QTcpSocket>
#include <QDateTime>

class User : public QObject
{
	Q_OBJECT

	Q_ENUMS(Level)
	Q_ENUMS(QueueState)
	Q_ENUMS(PhoneState)
	Q_ENUMS(Action)
    Q_ENUMS(Request)
	Q_ENUMS(Event)

public:
	explicit User(QObject *parent = 0);
	~User();

	enum Level {
		Agent,
		Supervisor,
        Manager,
        Administrator
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
        InvalidAction = -1,
		Login,
		Logout,
		Dial,
		Hangup,
		Transfer,
		SendDigit,
		Listen,
		Whisper,
		Barge,
		Pause,
	};

    enum Request {
        InvalidRequest = -1,
        Status,
        ChangePassword
    };

	enum Event {
		ActionReady,
		LoggedIn,
		LoggedOut,
		PeerChanged,
		QueueStateChanged,
		PhoneStateChanged
	};

	void setUsername(QString username);
	QString username();

	void setFullname(QString fullname);
	QString fullname() const;

	void setLevel(Level level);
	Level level() const;

	QStringList groups();

	void setPeer(QString peer);
	QString peer();

	void addQueue(QString queue);
	void removeQueue(QString queue);
	void setQueues(QStringList queues);
    QStringList queues();

	void setQueueState(QString queue, QueueState queueState);
	QueueState queueState() const;

	void setPauseReason(QString pauseReason);
	QString pauseReason();

	void setPhoneState(PhoneState phoneState);
	PhoneState phoneState();

	void setLastCall(QDateTime lastCall);
	QDateTime lastCall();

	virtual QString ipAddress() { return QString(); }

	void startPause();
    void finishPause();

	void sendResponse(User::Action action, bool success, QVariantMap fields = QVariantMap());
    void sendResponse(User::Request request, bool success, QVariantMap fields = QVariantMap());
	void sendEvent(User::Event event, QVariantMap fields);
	virtual void disconnect() {}

	QString levelText(int index = -1);
	QString phoneStateText(int index = -1);
	QString queueStateText(int index = -1);

	int levelIndex(QString text);
	int actionIndex(QString text);
    int requestIndex(QString text);

protected:
	int timerId;

	void timerEvent(QTimerEvent *event);

private:
	uint sessionId, pauseId;
	QString username_, fullname_, peer_, pauseReason_;
	Level level_;
	QStringList groups_;
	QueueState queueState_;
	PhoneState phoneState_;
	QDateTime lastCall_;
	QStringList queues_;

	QTcpSocket *client_;
	QByteArray messageBuffer;

	QString enumText(QString enumName, int index);
	int enumIndex(QString enumName, QString text);

	void startSession();
	void finishSession();

	void populateGroups();

	virtual void sendMessage(QVariantMap fields) { Q_UNUSED(fields) }

signals:
	void connected();
	void disconnected();
	void error(QString message);
	void actionReceived(User::Action action, QVariantMap fields);
    void requestReceived(User::Request request, QVariantMap fields);
	void queueStateChanged(User::QueueState queueState);
	void phoneStateChanged(User::PhoneState phoneState);
	void peerChanged(QString peer);
};

#endif // USER_H
