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
#include <QClipboard>
#include <QToolTip>

static QVector<RadioStation> defaultStations = {
    {"Groove Salad", "http://ice4.somafm.com/groovesalad-128-mp3", {"chill"}},
    {"Boot Liquor", "http://ice4.somafm.com/bootliquor-128-mp3", {"country"}},
    {"Heavyweight Reggae", "http://ice2.somafm.com/reggae-128-mp3", {"reggae"}},
    {"Groove Salad Classic", "http://ice6.somafm.com/gsclassic-128-mp3", {"chill"}},
    {"DEF CON Radio", "http://ice2.somafm.com/defcon-128-mp3", {"electro"}},
    {"Seven Inch Soul", "http://ice2.somafm.com/7soul-128-mp3", {"rock"}},
    {"Left Coast 70s", "http://ice2.somafm.com/seventies-128-mp3", {"rock"}},
    {"Underground 80s", "http://ice6.somafm.com/u80s-128-mp3", {"electro"}},
    {"Secret Agent", "http://ice6.somafm.com/secretagent-128-mp3", {"rock", "retro"}},
    {"Lush", "http://ice2.somafm.com/lush-128-aac", {"chill"}},
    {"Fluid", "http://ice2.somafm.com/fluid-128-mp3", {"chill"}},
    {"Illinois Street Lounge", "http://ice2.somafm.com/illstreet-128-mp3", {"chill", "retro"}},
    {"Suburbs of Goa", "http://ice6.somafm.com/suburbsofgoa-128-mp3", {"electro"}},
    {"Metal Detector", "http://ice6.somafm.com/metal-128-mp3", {"rock"}},
    {"Black Rock FM", "http://ice2.somafm.com/brfm-128-mp3", {"chill", "electro"}}
};

static QString stationsFileName()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
            QDir::separator() + "radio.json";
}

RadioWidget::RadioWidget(QObject *parent) :
    QSystemTrayIcon(parent),
    m_player(this, QMediaPlayer::StreamPlayback)
{
    setIcon(QIcon::fromTheme("audio-headphones-symbolic",
                             QIcon(":/icons/radio.png")));

    loadStations();
    createMenu();

    connect(this, &QSystemTrayIcon::activated, this, &RadioWidget::onActivation);
    connect(this, &RadioWidget::currentStationChanged, this, &RadioWidget::updateTooltip);
    connect(&m_tooltipTimer, &QTimer::timeout, this, &RadioWidget::updateTooltip);
    connect(&m_playButton, &QAction::triggered, this, &RadioWidget::onPlayClicked);
    connect(&m_currentSongLabel, &QAction::triggered, this, &RadioWidget::copyToClipboard);

    m_tooltipTimer.setInterval(1000);
    m_tooltipTimer.start();
}

RadioWidget::~RadioWidget()
{

}

void RadioWidget::setCurrentStation(const RadioStation &station)
{
    if (station == m_currentStation)
        return;

    m_currentStation = station;

    m_player.setMedia(QUrl(station.url));
    m_player.setVolume(100);
    m_player.play();

    m_playButton.setText(tr("Pause ") + station.name);

    emit currentStationChanged(station);
}

void RadioWidget::configure()
{
    ConfigDialog dialog;
    int res = dialog.exec();

    if (res == QDialog::Accepted) {
        loadStations();
        createMenu();
    }
}

void RadioWidget::updateTooltip()
{
    QString artist = m_player.metaData("AlbumArtist").toString();
    QString title = m_player.metaData("Title").toString();

    Song song = {.artist = artist, .title = title};

    setCurrentSong(song);
}

void RadioWidget::setCurrentSong(const Song &song)
{
    if (song == m_currentSong)
        return;

    m_currentSong = song;

    QString text;

    if (!song.title.isEmpty()) {

        if (!song.artist.isEmpty())
            text = QString("%1 - ").arg(song.artist);

        text += song.title;

        showMessage(m_currentStation.name, text, QSystemTrayIcon::Information, 5000);
        m_currentSongLabel.setText(text);
        setToolTip(text);
    }

    emit currentSongChanged(song);
}

void RadioWidget::createDefaultStationsFile()
{
    QDir().mkpath(QFileInfo(stationsFileName()).absolutePath());

    QFile stationsFile(stationsFileName());
    stationsFile.open(QIODevice::WriteOnly | QIODevice::Text);

    // save default list
    QJsonArray array;

    for (auto &station : defaultStations) {
        QJsonObject object;
        object["name"] = station.name;
        object["url_resolved"] = station.url;
        object["tags"] = station.categories.join(",");

        array.push_back(object);
    }

    stationsFile.write(QJsonDocument(array).toJson());
    stationsFile.close();
}

void RadioWidget::onActivation(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case Trigger:
        m_menu.exec(QCursor::pos() + QPoint(0,10));
        break;
    default:
        break;
    }
}

void RadioWidget::onPlayClicked(bool)
{
    if (m_player.state() == QMediaPlayer::PlayingState) {
        m_player.stop();

        m_playButton.setText(tr("Play ") + m_currentStation.name);
    } else if (m_player.state() == QMediaPlayer::StoppedState) {
        m_player.play();

        if (!m_currentStation.name.isEmpty())
            m_playButton.setText(tr("Pause ") + m_currentStation.name);
    }
}

void RadioWidget::copyToClipboard(bool)
{
    QClipboard *clipboard = QGuiApplication::clipboard();

    if (!m_currentSongLabel.text().isEmpty())
        clipboard->setText(m_currentSongLabel.text());
}

void RadioWidget::createMenu()
{
    m_menu.clear();

    m_playButton.setText(tr("No station"));
    m_menu.addAction(&m_playButton);

    m_currentSongLabel.setText(tr("No song"));
    m_menu.addAction(&m_currentSongLabel);

    m_menu.addSeparator();
    
    for (auto &station : m_stations) {
        m_menu.addAction(station.name, [=](bool) {
            setCurrentStation(station);
        });
    }

    /* 
    QStringList categories;
    QMap<QString, QMenu*> submenus;
    
    for (auto &station : m_stations)
        for (auto &category : station.categories)
            if (!categories.contains(category, Qt::CaseSensitive))
                categories << category;

    for (auto &category : categories)
        submenus[category] = m_menu.addMenu(category);

    for (auto &station : m_stations) {
        for (auto &category : station.categories) {
            submenus[category]->addAction(station.name, [=](bool) {
                setCurrentStation(station);
            });
        }
    }
    */

    m_menu.addSeparator();

    m_menu.addAction(tr("Configure"), [this](bool) { configure(); });
    m_menu.addAction(tr("Exit"), [](bool) { QApplication::quit(); });

    setContextMenu(&m_menu);
}

void RadioWidget::loadStations()
{
    QFile stationsFile(stationsFileName());

    if (!stationsFile.exists()) {
        createDefaultStationsFile();
    }

    stationsFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QString jsonString = stationsFile.readAll();
    QJsonArray array = QJsonDocument::fromJson(jsonString.toUtf8()).array();

    m_stations.clear();
    m_currentStation = {};
    m_currentSong = {};

    for (auto element : array) {
        QJsonObject object = element.toObject();

        RadioStation station;
        station.name = object["name"].toString();
        station.url = object["url_resolved"].toString();
        station.categories = object["tags"].toString().split(",");

        if (!station.name.isEmpty() && !station.url.isEmpty())
            m_stations.push_back(station);
    }
}
