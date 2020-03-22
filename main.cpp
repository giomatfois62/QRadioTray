#include "traywidget.h"

#include <QApplication>
#include <QDebug>

#include "traywidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (!QSystemTrayIcon::isSystemTrayAvailable())
        return 1;

    TrayWidget w;
    w.show();

    return a.exec();
}
