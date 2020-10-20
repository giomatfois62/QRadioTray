#ifndef TRAYWIDGET_H
#define TRAYWIDGET_H

#include <QWidget>
#include <QSystemTrayIcon>
#include <QMediaPlayer>
#include <QTimer>

struct RadioStation {
	QString name;
	QString url;
	QStringList categories;
};

class TrayWidget : public QSystemTrayIcon
{
	Q_OBJECT

	public:
		TrayWidget(QObject *parent = nullptr);
		~TrayWidget();
		
        void setCurrentStation(const RadioStation &station);
		void updateTooltip();

private slots:
        void onActivation(QSystemTrayIcon::ActivationReason reason);

	private:
        void createContextMenu();
        void loadStations();

		QMenu *m_contextMenu;
		QMediaPlayer m_player;
		QTimer m_timer;
		QString m_currentSong;
		QVector<RadioStation> m_stations;
		RadioStation m_currentStation;
		QAction *m_currentSongLabel;
		QAction *m_controlButton;
};

#endif // TRAYWIDGET_H
