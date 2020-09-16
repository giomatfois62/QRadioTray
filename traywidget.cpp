#include "traywidget.h"
#include "configdialog.h"

#include <QMenu>
#include <QApplication>
#include <QSettings>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

static QVector<RadioStation> defaultStations = {
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

static QString stationsFileName()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
            QDir::separator() + "radio.json";
}

TrayWidget::TrayWidget(QObject *parent) :
	QSystemTrayIcon(parent),
    m_radioMenu(new QMenu),
	m_player(this, QMediaPlayer::StreamPlayback)
{

    QIcon icon = QIcon::fromTheme("audio-headphones", QIcon(":/icons/radio.png"));
    QPixmap pixmap = icon.pixmap(QSize(22,22));
    setIcon(QIcon(pixmap));

    loadStations();
    createContextMenu();

    connect(this, &QSystemTrayIcon::activated, this, &TrayWidget::onActivation);
	connect(&m_timer, &QTimer::timeout, this, &TrayWidget::updateTooltip);

    m_timer.setInterval(1000);
	m_timer.start();
}

TrayWidget::~TrayWidget()
{
    delete m_contextMenu;
    delete m_radioMenu;
}

void TrayWidget::setCurrentStation(const RadioStation &station)
{
    m_currentStation = station;

    m_player.setMedia(QUrl(station.url));
    m_player.setVolume(100);
    m_player.play();

    QTimer::singleShot(5000, this, &TrayWidget::updateTooltip);
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
        		showMessage(m_currentStation.name, text,
			QSystemTrayIcon::Information, 5000);
	}

	if (!artist.isEmpty() && !title.isEmpty())
		text.append(artist + " - " + title);

    setToolTip(text);
}

void TrayWidget::onActivation(QSystemTrayIcon::ActivationReason reason)
{
    QAction *requestedAction = nullptr;

    switch (reason) {
        case DoubleClick:
        case Trigger:
                requestedAction = m_radioMenu->exec(QCursor::pos());
                break;
        default:
                break;
    }
}

void TrayWidget::createContextMenu()
{
    m_contextMenu = new QMenu;

    m_contextMenu->clear();

    m_contextMenu->addAction(tr("Play/Pause"), [this](bool) {
        if (m_player.state() == QMediaPlayer::PlayingState) {
                m_player.stop();
        } else if (m_player.state() == QMediaPlayer::StoppedState) {
                m_player.play();
        }
    });

    m_contextMenu->addSeparator();

    for (auto &station : m_stations) {
        m_contextMenu->addAction(station.name, [&](bool) {
            setCurrentStation(station);
        });
    }

    m_contextMenu->addSeparator();

    m_contextMenu->addAction(tr("Configure"), [this](bool) {
        qDebug() << "ConfigureMenu";

        ConfigDialog dialog;
        int res = dialog.exec();

        if (res == QDialog::Accepted) {
            loadStations();
		createContextMenu();
	}
    });



    m_contextMenu->addAction(tr("About"), [](bool) {
        qDebug() << "About";
    });

    m_contextMenu->addAction(tr("Exit"), [](bool) {
        QApplication::quit();
    });

    setContextMenu(m_contextMenu);
}

void TrayWidget::loadStations()
{
    m_radioMenu->clear();

    m_radioMenu->addAction(tr("Play/Pause"), [this](bool) {
        if (m_player.state() == QMediaPlayer::PlayingState) {
                m_player.stop();
        } else if (m_player.state() == QMediaPlayer::StoppedState) {
                m_player.play();
        }
    });

    m_radioMenu->addSeparator();

    QFile stationsFile(stationsFileName());

    if (!stationsFile.exists()) {
        QDir().mkpath(QFileInfo(stationsFileName()).absolutePath());

        stationsFile.open(QIODevice::WriteOnly | QIODevice::Text);

        // save default list
        QJsonArray array;

        for (auto &station : defaultStations) {
            QJsonObject object;
            object["name"] = station.name;
            object["url"] = station.url;

            array.push_back(object);
        }

        stationsFile.write(QJsonDocument(array).toJson());
        stationsFile.close();
    }

    stationsFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QString jsonString = stationsFile.readAll();
    QJsonArray array = QJsonDocument::fromJson(jsonString.toUtf8()).array();

    m_stations.clear();

    for (auto element : array) {
        QJsonObject object = element.toObject();

        RadioStation station;
        station.name = object["name"].toString();
        station.url = object["url"].toString();

        qDebug() << "   " << station.name << " - " << station.url;

        if (!station.name.isEmpty() && !station.url.isEmpty())
                m_stations.push_back(station);
    }

    for (auto &station : m_stations) {
        m_radioMenu->addAction(station.name, [&](bool) {
            setCurrentStation(station);
        });
    }
}
