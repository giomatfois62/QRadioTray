#include "traywidget.h"

#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QLockFile>

#include "traywidget.h"

void showErrorMessage(const QString &msg)
{
    QMessageBox msgBox;

    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText(msg);
    msgBox.exec();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setQuitOnLastWindowClosed(false);

    QCoreApplication::setApplicationName("QRadioTray");

    /* https://evileg.com/en/post/147/
     *
     * Trying to close the Lock File, if the attempt is unsuccessful for 100 milliseconds,
     * then there is a Lock File already created by another process.
     / Therefore, we throw a warning and close the program
     * */
    QLockFile lockFile(QDir::temp().absoluteFilePath("<uniq id>.lock"));

    if(!lockFile.tryLock(100)){
        showErrorMessage("The application is already running.\n"
                       "Allowed to run only one instance of the application.");

        return 1;
    }

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        showErrorMessage("No system tray is available in this system.");

        return 1;
    }

    RadioWidget w;
    w.show();

    return a.exec();
}
