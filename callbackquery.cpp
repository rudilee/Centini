#include "callbackquery.h"

CallbackQuery::CallbackQuery(QSqlDatabase database) :
	QSqlQuery(database)
{
}

void CallbackQuery::setQueryId(QString queryId)
{
	queryId_ = queryId;
}

QString CallbackQuery::queryId()
{
	return queryId_;
}

void CallbackQuery::setType(CallbackQuery::Type type)
{
	type_ = type;
}

CallbackQuery::Type CallbackQuery::type()
{
	return type_;
}

void CallbackQuery::setParameters(QVariantMap parameters)
{
	parameters_ = parameters;
}

QVariantMap CallbackQuery::parameters()
{
	return parameters_;
}

void CallbackQuery::setUser(User *user)
{
	user_ = user;
}

User *CallbackQuery::user()
{
	return user_;
}

QFutureWatcher<CallbackQuery *> *CallbackQuery::queryWatcher()
{
	return &queryWatcher_;
}
