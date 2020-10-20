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
#include <QDebug>

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

TrayWidget::TrayWidget(QObject *parent) :
	QSystemTrayIcon(parent),
    m_contextMenu(nullptr),
	m_player(this, QMediaPlayer::StreamPlayback),
	m_currentSongLabel(nullptr),
	m_controlButton(nullptr)
{

    QIcon icon = QIcon::fromTheme("audio-headphones-symbolic", QIcon(":/icons/radio.png"));
    //QPixmap pixmap = icon.pixmap(QSize(22,22));
    setIcon(icon);

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
}

void TrayWidget::setCurrentStation(const RadioStation &station)
{
    m_currentStation = station;

    m_player.setMedia(QUrl(station.url));
    m_player.setVolume(100);
    m_player.play();

	m_controlButton->setText("Pause " + station.name);

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

	m_currentSongLabel->setText(text);
}

void TrayWidget::onActivation(QSystemTrayIcon::ActivationReason reason)
{
    //QAction *requestedAction = nullptr;

    switch (reason) {
        case DoubleClick:
        case Trigger:
                //requestedAction = m_radioMenu->exec(QCursor::pos());
                break;
        default:
                break;
    }
}

void TrayWidget::createContextMenu()
{
    if (m_contextMenu)
        delete m_contextMenu;

    m_contextMenu = new QMenu;

    m_contextMenu->clear();

	if (m_controlButton)
        delete m_controlButton;
	m_controlButton = new QAction("No station");
	connect(m_controlButton, &QAction::triggered, [&](bool) {
			        if (m_player.state() == QMediaPlayer::PlayingState) {
							m_player.stop();

							m_controlButton->setText("Play " + m_currentStation.name);
					} else if (m_player.state() == QMediaPlayer::StoppedState) {
							m_player.play();
							if (!m_currentStation.name.isEmpty())
								m_controlButton->setText("Pause " + m_currentStation.name);
					}
			});
	m_contextMenu->addAction(m_controlButton);

	if (m_currentSongLabel)
        delete m_currentSongLabel;
	m_currentSongLabel = new QAction("No song");
	connect(m_currentSongLabel, &QAction::triggered, [&](bool) {
            	
				//QToolTip::showText(QPoint(0,0), QString("copied to clipboard"), nullptr, QRect(), 3000);

				QClipboard *clipboard = QGuiApplication::clipboard();
				if (!m_currentSongLabel->text().isEmpty())
					clipboard->setText(m_currentSongLabel->text());
			});
	m_contextMenu->addAction(m_currentSongLabel);

    m_contextMenu->addSeparator();

	QStringList categories;
	QMap<QString, QMenu*> menus;

	for (auto &station : m_stations)
		for (auto &category : station.categories)
			if (!categories.contains(category, Qt::CaseSensitive)) {
				categories << category;
			}

	for (auto &category : categories)
		menus[category] = new QMenu(category);

	for (auto &station : m_stations) {
		for (auto &category : station.categories)
			menus[category]->addAction(station.name, [&](bool) {
            	setCurrentStation(station);
			});
	}	

    for (auto &menu : menus.values()) {
		m_contextMenu->addMenu(menu);
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

    m_contextMenu->addAction(tr("Exit"), [](bool) {
        QApplication::quit();
    });

    setContextMenu(m_contextMenu);
}

void TrayWidget::loadStations()
{
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
			object["categories"] = station.categories.join(";");

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
		station.categories = object["categories"].toString().split(";");

        qDebug() << "   " << station.name << " - " << station.url << " - " << station.categories; 

        if (!station.name.isEmpty() && !station.url.isEmpty())
                m_stations.push_back(station);
    }
}
