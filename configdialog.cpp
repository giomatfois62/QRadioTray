#include "configdialog.h"
#include "ui_configdialog.h"

#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QInputDialog>
#include <QNetworkReply>
#include <QFormLayout>
#include <QPushButton>
#include <QLabel>
#include <QDebug>

WebRadio WebRadio::fromJson(const QJsonObject &obj)
{
        WebRadio radio;
        
        radio.name = obj["name"].toString();
        radio.url = obj["url_resolved"].toString();
        radio.homepage = obj["homepage"].toString();
        radio.faviconUrl = obj["favicon"].toString();
        radio.tags = obj["tags"].toString().split(",");
        radio.country = obj["country"].toString();
        radio.countrycode = obj["countrycode"].toString();
        radio.state = obj["state"].toString();
        radio.language = obj["language"].toString();
        radio.codec = obj["codec"].toString();
        radio.bitrate = obj["bitrate"].toInt();
        
        return radio;
}

QJsonObject WebRadio::fromRadio(const WebRadio &radio)
{
        QJsonObject obj;
        
        obj.insert("name", QJsonValue(radio.name));
        obj.insert("url_resolved", QJsonValue(radio.url));
        obj.insert("homepage", QJsonValue(radio.homepage));
        obj.insert("favicon", QJsonValue(radio.faviconUrl));
        obj.insert("tags", QJsonValue(radio.tags.join(",")));
        obj.insert("country", QJsonValue(radio.country));
        obj.insert("countrycode", QJsonValue(radio.countrycode));
        obj.insert("state", QJsonValue(radio.state));
        obj.insert("language", QJsonValue(radio.language));
        obj.insert("codec", QJsonValue(radio.codec));
        obj.insert("bitrate", QJsonValue(radio.bitrate));

        return obj;
}

static QString stationsFileName()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
            QDir::separator() + "radio.json";
}

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog),
    manager(new QNetworkAccessManager(this))
{
    ui->setupUi(this);
    
    ui->radio->setLayout(new QFormLayout);
    ui->radios->setVisible(false);

    loadStations();
    
    ui->favourites->clear();
    ui->favourites->insertItems(0, favourites.keys());
    
    searchTimer.setInterval(1000);
    
    connect(&searchTimer, &QTimer::timeout, [&]() {
        QString name = ui->search->text();
        if (name.isEmpty()) {
            ui->radios->setVisible(false);
            return;
        }
        
        QUrl url("https://de1.api.radio-browser.info/json/stations/search?name=" + name);
    
        manager->get(QNetworkRequest(url));
    });
    
    connect(manager, &QNetworkAccessManager::finished, [&](QNetworkReply* reply) {
        qDebug() << "Got reply " << reply->error();
        searchTimer.stop();
       
        QString strReply = (QString)reply->readAll();
    
        QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());

        QJsonArray jsonArray = jsonResponse.array();
        
        qDebug() << "Received " << jsonArray.size() << " radios";
        
        for (const auto &val : jsonArray) {
            WebRadio radio = WebRadio::fromJson(val.toObject());
            radios[radio.name] = radio;
        }
        
        ui->radios->clear();
        ui->radios->insertItems(0, radios.keys());
        ui->radios->setVisible(true);
    });
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void ConfigDialog::on_add_clicked(bool)
{
    QString name, url, tags;

    name = QInputDialog::getText(this, "Station Name",
        "Name:", QLineEdit::Normal, "");

    if (name.isEmpty())
        return;

    url = QInputDialog::getText(this, "Station URL",
        "Url:", QLineEdit::Normal, "");

    if (url.isEmpty())
        return;

	tags = QInputDialog::getText(this, "Station Genre",
        "Genre:", QLineEdit::Normal, "");
    
    WebRadio radio;
    
    radio.name = name;
    radio.url = url;
    radio.tags = tags.split(",");

    favourites[name] = radio;
    ui->favourites->insertItem(ui->radios->count(), name);
}

void ConfigDialog::done(int res)
{
    if (res == QDialog::Accepted) {
        qDebug() << "Accepted";
        saveStations();

    } else {
        qDebug() << "Rejected";
    }

    QDialog::done(res);
}

void ConfigDialog::loadStations()
{
    QFile stationsFile(stationsFileName());

    stationsFile.open(QIODevice::ReadOnly | QIODevice::Text);

    QString jsonString = stationsFile.readAll();
    QJsonArray array = QJsonDocument::fromJson(jsonString.toUtf8()).array();

    favourites.clear();
    radios.clear();
    
    for (const auto &element : array) {
        WebRadio radio = WebRadio::fromJson(element.toObject());
        
        favourites[radio.name] = radio;
    }
}

void ConfigDialog::saveStations()
{
    QFile stationsFile(stationsFileName());
    stationsFile.open(QIODevice::WriteOnly | QIODevice::Text);

    QJsonArray array;
    
    for (const auto &radio : favourites)
        array.push_back(WebRadio::fromRadio(radio));

    stationsFile.write(QJsonDocument(array).toJson());
    stationsFile.close();
}

void ConfigDialog::on_search_textChanged(const QString &text)
{
    searchTimer.start();
}

void ConfigDialog::showRadio(const WebRadio &radio)
{
    QFormLayout *layout = static_cast<QFormLayout*>(ui->radio->layout());
    while(!layout->isEmpty())
        layout->removeRow(0);
    
    layout->addRow("name", new QLabel(radio.name));
    layout->addRow("url", new QLabel(radio.url));
    layout->addRow("homepage", new QLabel(radio.homepage));
    layout->addRow("faviconUrl", new QLabel(radio.faviconUrl));
    layout->addRow("tags", new QLabel(radio.tags.join(",")));
    layout->addRow("country", new QLabel(radio.country));
    layout->addRow("countrycode", new QLabel(radio.countrycode));
    layout->addRow("state", new QLabel(radio.state));
    layout->addRow("language", new QLabel(radio.language));
    layout->addRow("codec", new QLabel(radio.codec));
    layout->addRow("bitrate", new QLabel(QString::number(radio.bitrate)));
    
    QPushButton *remove = new QPushButton("Remove from Favourites");
    QPushButton *add = new QPushButton("Add to Favourites");
    
    connect(remove, &QPushButton::clicked, [=](bool){
            favourites.remove(radio.name);
            
            QList<QListWidgetItem*> items = ui->favourites->findItems(radio.name, Qt::MatchExactly);
            if (items.size()) {
                delete ui->favourites->takeItem(ui->favourites->row(items[0]));  
            }
            
            add->setEnabled(true);
            remove->setEnabled(false);
    });
    
    connect(add, &QPushButton::clicked, [=](bool){
            favourites[radio.name] = radio;
            ui->favourites->addItem(radio.name);
            add->setEnabled(false);
            remove->setEnabled(true);
    });
    
    if (favourites.contains(radio.name))
        add->setEnabled(false);
    else
        remove->setEnabled(false);
    
    layout->addRow(add);
    layout->addRow(remove);
}

void ConfigDialog::on_radios_currentTextChanged(const QString &text)
{
    if (text.isEmpty()) {
        return;
    }
    
    showRadio(radios[text]);
}

void ConfigDialog::on_favourites_currentTextChanged(const QString &text)
{
    if (text.isEmpty()) {
        return;
    }
    
    showRadio(favourites[text]);
}
