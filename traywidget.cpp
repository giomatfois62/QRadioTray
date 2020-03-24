#include "traywidget.h"

#include <QMenu>
#include <QApplication>
#include <QSettings>
#include <QStandardPaths>
#include <QFile>
#include <QDebug>

TrayWidget::TrayWidget(QObject *parent) : 
	QSystemTrayIcon(parent),
	m_player(this, QMediaPlayer::StreamPlayback)
{
	setIcon(QIcon(":/icons/Radio-icon-black.png"));

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

	QSettings settings;

	for(QString &group : settings.childGroups()) {
		settings.beginGroup(group);

		RadioStation station;
		station.name = settings.value("name").toString();
		station.url = settings.value("url").toString();

		qDebug() << group << " " << station.name << " - " << station.url;

		if (!station.name.isEmpty() && !station.url.isEmpty())
        		m_stations.push_back(station);

		settings.endGroup();
	}

	
	for (int i = 0; i < m_stations.size(); ++i) {
		m_radioMenu->addAction(m_stations[i].name,  [=](bool) {
        		m_currentStation = m_stations[i];
        		m_player.setMedia(QUrl(m_stations[i].url));
        		m_player.setVolume(100);
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
        		showMessage(m_currentStation.name, text, 
			QSystemTrayIcon::Information, 5000);
	}
	
	if (!artist.isEmpty() && !title.isEmpty())
		text.append(artist + " - " + title);
	
	setToolTip(text);
}
