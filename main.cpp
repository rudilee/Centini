#include <QCoreApplication>

#include "centiniserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    CentiniServer centini;

    return a.exec();
}
