#ifndef TRAYWIDGET_H
#define TRAYWIDGET_H

#include <QWidget>
#include <QSystemTrayIcon>
#include <QMediaPlayer>
#include <QTimer>

struct RadioStation {
    QString name;
    QString url;
};

class TrayWidget : public QSystemTrayIcon
{
    Q_OBJECT

public:
    TrayWidget(QObject *parent = nullptr);
    ~TrayWidget();

    void play(const QString &url);
    void pause();

    void updateTooltip();

private:
    QMenu *m_contextMenu;
    QMenu *m_radioMenu;
    QMediaPlayer m_player;
    QTimer m_timer;
    QString m_currentSong;
    QVector<RadioStation> m_stations;
};
#endif // TRAYWIDGET_H
