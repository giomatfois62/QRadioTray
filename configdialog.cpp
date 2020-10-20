#include "configdialog.h"
#include "ui_configdialog.h"

#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QInputDialog>
#include <QDebug>

static QString stationsFileName()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
            QDir::separator() + "radio.json";
}

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);

    loadStations();
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void ConfigDialog::on_add_clicked(bool)
{
    QString name, url, categories;

    name = QInputDialog::getText(this, "Station Name",
        "Name:", QLineEdit::Normal, "");

    if (name.isEmpty())
        return;

    url = QInputDialog::getText(this, "Station URL",
        "Url:", QLineEdit::Normal, "");

    if (url.isEmpty())
        return;

	categories = QInputDialog::getText(this, "Station Genre",
        "Genre:", QLineEdit::Normal, "");

    int current = ui->stations->rowCount();

    ui->stations->setRowCount(current + 1);
    ui->stations->setItem(current, 0, new QTableWidgetItem(name));
    ui->stations->setItem(current, 1, new QTableWidgetItem(categories));
	ui->stations->setItem(current, 2, new QTableWidgetItem(url));
}

void ConfigDialog::on_remove_clicked(bool)
{
    int index = ui->stations->currentRow();

    if (index >= 0) {
        ui->stations->removeRow(index);
    }
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

    int current = 0;

    for (auto element : array) {
        QJsonObject object = element.toObject();

        QString name = object["name"].toString();
        QString url = object["url"].toString();
		QString categories = object["categories"].toString();

        ui->stations->setRowCount(current + 1);
        ui->stations->setItem(current, 0, new QTableWidgetItem(name));
        ui->stations->setItem(current, 2, new QTableWidgetItem(url));
		ui->stations->setItem(current, 1, new QTableWidgetItem(categories));

        current++;
    }
}

void ConfigDialog::saveStations()
{
    QFile stationsFile(stationsFileName());

    stationsFile.open(QIODevice::WriteOnly | QIODevice::Text);

    // save default list
    QJsonArray array;

    for (int i = 0; i < ui->stations->rowCount(); ++i) {
        QJsonObject object;
        object["name"] = ui->stations->item(i,0)->text();
        object["url"] = ui->stations->item(i,2)->text();
		object["categories"] = ui->stations->item(i,1)->text();

        array.push_back(object);
    }

    stationsFile.write(QJsonDocument(array).toJson());
    stationsFile.close();
}
