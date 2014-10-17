#include <QtConcurrentRun>
#include <QSqlError>

#include "futurequery.h"

FutureQuery::FutureQuery(QObject *parent) :
    QObject(parent),
    query_()
{
    connect(&futureWatcher, SIGNAL(finished()), SIGNAL(finished()));
}

QSqlQuery *FutureQuery::query()
{
    return &query_;
}

void FutureQuery::setUsername(QString username)
{
    username_ = username;
}

QString FutureQuery::username()
{
    return username_;
}

void FutureQuery::exec()
{
//    future = QtConcurrent::run([](QSqlQuery *query) {
//        qDebug() << "Thread:" << thread();

//        if (!query->exec())
//            qWarning() << "Query gagal dijalankan, error:" << query.lastError().text();
//    }, &query_);

//    futureWatcher.setFuture(future);
}
