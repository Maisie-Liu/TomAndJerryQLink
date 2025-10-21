#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include <QWidget>
#include <QDialog>
#include <QSoundEffect>

namespace Ui {
class SettingDialog;
}

class SettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingDialog(QWidget *parent = nullptr);
    ~SettingDialog();

    void changeEvent(QEvent * event);

    void btnSet_zh();
    void btnSet_en();

    QSoundEffect *se_click = nullptr;

signals:
    void lset_zh();
    void lset_en();
    void playBGM();
    void stopBGM();

private:
    Ui::SettingDialog *ui;
};

#endif // SETTINGDIALOG_H
