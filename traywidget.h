#ifndef TRAYWIDGET_H
#define TRAYWIDGET_H

#include <QWidget>
#include <QSystemTrayIcon>
#include <QMediaPlayer>
#include <QTimer>
#include <QMenu>

struct RadioStation {
    QString name;
    QString url;
    QStringList categories;

    bool operator==(const RadioStation &other) const {
        return url == other.url;
    }

    void operator=(const RadioStation &other) {
        url = other.url;
        name = other.name;
        categories = other.categories;
    }
};

struct Song {
    QString artist;
    QString title;

    bool operator==(const Song &other) const {
        return title == other.title;
    }

    void operator=(const Song &other) {
        title = other.title;
        artist = other.artist;
    }
};

class RadioWidget : public QSystemTrayIcon
{
    Q_OBJECT

public:
    RadioWidget(QObject *parent = nullptr);
    ~RadioWidget();

    void setCurrentStation(const RadioStation &station);
    RadioStation currentStation();

    void configure();

signals:
    void currentStationChanged(RadioStation station);
    void currentSongChanged(Song song);

private slots:
    void onActivation(QSystemTrayIcon::ActivationReason reason);
    void onPlayClicked(bool);
    void copyToClipboard(bool);

private:
    void createMenu();
    void loadStations();
    void updateTooltip();
    void setCurrentSong(const Song &song);
    void createDefaultStationsFile();

    QMenu m_menu;
    QAction m_currentSongLabel;
    QAction m_playButton;

    QMediaPlayer m_player;
    QTimer m_tooltipTimer;
    QVector<RadioStation> m_stations;
    Song m_currentSong;
    RadioStation m_currentStation;
};

#endif // TRAYWIDGET_H
