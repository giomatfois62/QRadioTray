#include "traywidget.h"

#include <QApplication>
#include <QSettings>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QLockFile>
#include <QDebug>

#include "traywidget.h"

QVector<RadioStation> defaultStations = {
    {"Groove Salad", "http://ice4.somafm.com/groovesalad-128-mp3"},
    {"Boot Liquor", "http://ice4.somafm.com/bootliquor-128-mp3"},
    {"Heavyweight Reggae", "http://ice2.somafm.com/reggae-128-mp3"},
    {"Groove Salad Classic", "http://ice6.somafm.com/gsclassic-128-mp3"},
    {"DEF CON Radio", "http://ice2.somafm.com/defcon-128-mp3"},
    {"Seven Inch Soul", "http://ice2.somafm.com/7soul-128-mp3"},
    {"Left Coast 70s", "http://ice2.somafm.com/seventies-128-mp3"},
    {"Underground 80s", "http://ice6.somafm.com/u80s-128-mp3"},
    {"Secret Agent", "http://ice6.somafm.com/secretagent-128-mp3"},
    {"Lush", "http://ice2.somafm.com/lush-128-aac"},
    {"Fluid", "http://ice2.somafm.com/fluid-128-mp3"},
    {"Illinois Street Lounge", "http://ice2.somafm.com/illstreet-128-mp3"},
    {"Suburbs of Goa", "http://ice6.somafm.com/suburbsofgoa-128-mp3"},
    {"Metal Detector", "http://ice6.somafm.com/metal-128-mp3"},
    {"Black Rock FM", "http://ice2.somafm.com/brfm-128-mp3"}
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("QRadioTray");
    QCoreApplication::setApplicationName("QRadioTray");

    QLockFile lockFile(QDir::temp().absoluteFilePath("<uniq id>.lock"));

    /* https://evileg.com/en/post/147/
     *
     * Trying to close the Lock File, if the attempt is unsuccessful for 100 milliseconds,
     * then there is a Lock File already created by another process.
     / Therefore, we throw a warning and close the program
     * */
    if(!lockFile.tryLock(100)){
        QMessageBox msgBox;

        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("The application is already running.\n"
                       "Allowed to run only one instance of the application.");
        msgBox.exec();

        return 1;
    }

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox msgBox;

        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("No system tray is available in this system.");
        msgBox.exec();

        return 1;
    }

    // create default config
    if (!QFile(QSettings().fileName()).exists()) {
        QSettings settings;

        for (int i = 0; i < defaultStations.size(); ++i) {
            settings.beginGroup("Radio"+QString::number(i).rightJustified(3, '0'));
            settings.setValue("name", defaultStations[i].name);
            settings.setValue("url", defaultStations[i].url);
            settings.endGroup();
        }
    }

    TrayWidget w;
    w.show();

    return a.exec();
}
