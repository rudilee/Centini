#ifndef CALLBACKQUERY_H
#define CALLBACKQUERY_H

#include <QSqlQuery>
#include <QVariantMap>
#include <QFutureWatcher>

#include "user.h"

class CallbackQuery : public QSqlQuery
{
public:
	enum Type {
		CheckUserPassword
	};

	explicit CallbackQuery(QSqlDatabase database);

	void setQueryId(QString queryId);
	QString queryId();

	void setType(CallbackQuery::Type type);
	CallbackQuery::Type type();

	void setParameters(QVariantMap parameters);
	QVariantMap parameters();

	void setUser(User *user);
	User *user();

	QFutureWatcher<CallbackQuery *> *queryWatcher();

private:
	QString queryId_;
	Type type_;
	QVariantMap parameters_;
	User *user_;
	QFutureWatcher<CallbackQuery *> queryWatcher_;
};

#endif // CALLBACKQUERY_H
