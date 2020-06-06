#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>

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
    void on_remove_clicked(bool checked = false);

    void done(int res);

private:
    void loadStations();
    void saveStations();

    Ui::ConfigDialog *ui;
};

#endif // CONFIGDIALOG_H
