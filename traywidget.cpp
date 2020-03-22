#include "traywidget.h"

#include <QMenu>
#include <QApplication>
#include <QDebug>


struct RadioStation {
    QString name;
    QString url;
};

QVector<RadioStation> stations = {
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

TrayWidget::TrayWidget(QObject *parent)
    : QSystemTrayIcon(parent),
      m_player(this, QMediaPlayer::StreamPlayback)
{
    setIcon(QIcon("/home/mat/Radio-icon-white.png"));

    m_contextMenu = new QMenu;

    m_contextMenu->addAction(tr("Configure"), [this](bool) {
        qDebug() << "ConfigureMenu";
    });

    m_contextMenu->addSeparator();

    m_contextMenu->addAction(tr("About"), [this](bool) {
        qDebug() << "About";
    });

    m_contextMenu->addAction(tr("Exit"), [this](bool) {
        QApplication::quit();
    });

    setContextMenu(m_contextMenu);

    m_radioMenu = new QMenu;

    m_radioMenu->addAction(tr("Play/Pause"), [this](bool) {
        if (m_player.state() == QMediaPlayer::PlayingState) {
            m_player.stop();
        } else if (m_player.state() == QMediaPlayer::StoppedState) {
            m_player.play();
        }
    });

    m_radioMenu->addSeparator();

    for (RadioStation &station : stations) {
        m_radioMenu->addAction(station.name,  [this, station](bool) {
            m_player.setMedia(QUrl(station.url));
            m_player.setVolume(50);
            m_player.play();

            QTimer::singleShot(5000, this, &TrayWidget::updateTooltip);
        });
    }

    connect(this, &QSystemTrayIcon::activated,
        [this](QSystemTrayIcon::ActivationReason reason) {
            QAction *requestedAction = nullptr;

            switch (reason) {
                case DoubleClick:
                case Trigger:
                    requestedAction = m_radioMenu->exec(QCursor::pos());
                    break;
                default:
                    break;
            }
    });

    connect(&m_timer, &QTimer::timeout, this, &TrayWidget::updateTooltip);

    m_timer.setInterval(1000);

    m_timer.start();
}

TrayWidget::~TrayWidget()
{
    delete m_contextMenu;
}

void TrayWidget::updateTooltip()
{
    QString text;

    QString artist = m_player.metaData("AlbumArtist").toString();
    QString album = m_player.metaData("AlbumTitle").toString();
    QString title = m_player.metaData("Title").toString();

    if (!artist.isEmpty())
        text.append(artist + " - ");

    if (!title.isEmpty())
        text.append(title);

    if (!album.isEmpty())
        text.append(" (" + album + + ")");

    if (title != m_currentSong) {
        m_currentSong = title;

        if (!title.isEmpty())
            showMessage("Playing", text, QSystemTrayIcon::Information, 5000);
    }

    if (!artist.isEmpty() && !title.isEmpty())
        text.append(artist + " - " + title);

    setToolTip(text);
}

