#ifndef FUTUREQUERY_H
#define FUTUREQUERY_H

#include <QObject>
#include <QFutureWatcher>
#include <QSqlQuery>

class FutureQuery : public QObject
{
    Q_OBJECT

public:
    explicit FutureQuery(QObject *parent = 0);

    QSqlQuery *query();

    void setUsername(QString username);
    QString username();

    void exec();

private:
    QFutureWatcher<void> futureWatcher;
    QFuture<void> future;
    QSqlQuery query_;
    QString username_;

signals:
    void finished();
};

#endif // FUTUREQUERY_H
