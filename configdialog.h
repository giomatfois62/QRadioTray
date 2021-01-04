#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QTimer>


struct WebRadio {
    QString name;
    QString url;
    QString homepage;
    QString faviconUrl;
    QStringList tags;
    QString country;
    QString countrycode;
    QString state;
    QString language;
    QString codec;
    int bitrate;
    
    static WebRadio fromJson(const QJsonObject &);
    static QJsonObject fromRadio(const WebRadio &);
    
    bool operator==(const WebRadio &other) const {
        return name == other.name;
    }
};

namespace Ui {
class ConfigDialog;
}

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget *parent = nullptr);
    ~ConfigDialog();

public slots:
    void on_add_clicked(bool checked = false);
    void on_search_textChanged(const QString &text);
    void on_radios_currentTextChanged(const QString &text);
    void on_favourites_currentTextChanged(const QString &text);

    void done(int res);

private:
    void loadStations();
    void saveStations();
    void showRadio(const WebRadio &radio);

    Ui::ConfigDialog *ui;
    
    QNetworkAccessManager *manager;
    QTimer searchTimer;
    
    QMap<QString, WebRadio> radios;
    QMap<QString, WebRadio> favourites;
};

#endif // CONFIGDIALOG_H
