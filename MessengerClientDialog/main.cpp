#include <QApplication>
#include "clientwnd.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ClientWnd wnd("localhost",2323);

    wnd.show();

    return a.exec();
}
