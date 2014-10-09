#include <QCoreApplication>
#include <QDateTime>
#include <iostream>

#include "centiniserver.h"

void simpleLoggingHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
	QString logType;

	switch (type) {
		case QtDebugMsg:
			logType = "INFO";
			break;
		case QtCriticalMsg:
			logType = "ERROR";
			break;
		case QtWarningMsg:
			logType = "WARNING";
			break;
		case QtFatalMsg:
			logType = "FATAL";
			break;
	}

	std::cout << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz").toLatin1().data()
			  << QString(" [%1] ").arg(logType).toLatin1().data()
//              << QString(" (%1:%2:%3) ").arg(QString(context.file), QString(context.function), QString::number(context.line)).toLatin1().data()
			  << QString(message).replace("\"", "").trimmed().toLatin1().data()
			  << std::endl;

	if (type == QtFatalMsg)
		abort();
}

int main(int argc, char *argv[])
{
	qInstallMessageHandler(simpleLoggingHandler);

	QCoreApplication::setOrganizationName("Kanaya Hijab");
	QCoreApplication::setApplicationName("Centini Server");
	QCoreApplication::setApplicationVersion("0.2.0");

	QCoreApplication app(argc, argv);

	(new CentiniServer)->run();

	return app.exec();
}
