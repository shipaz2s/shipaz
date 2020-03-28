#include "serv.h"
#include <QCoreApplication>

int main(int argc, char** argv){
    QCoreApplication app(argc, argv);
    serv myserv(2323, &app);

    return app.exec();
}

