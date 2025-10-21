#ifndef MAINSCENE_H
#define MAINSCENE_H

#include <QWidget>
#include <QPushButton>
#include <QTranslator>
#include <QSoundEffect>
#include <QMediaPlayer>

#include "modechoosescene.h"
#include "archivedialog.h"
#include "settingdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainScene;
}
QT_END_NAMESPACE

class MainScene : public QWidget
{
    Q_OBJECT

public:
    MainScene(QWidget *parent = nullptr,QTranslator * tslt = nullptr);
    ~MainScene();
    void load(int i);
    ArchiveDialog * archiveDialog = nullptr;
    SettingDialog * settingDialog = nullptr;
    QTranslator * translator = nullptr;
    QSoundEffect *se_click = nullptr,*player = nullptr;

    //true: zh; false: en
    bool language = true;


private:
    Ui::MainScene *ui;
    ModeChooseScene * modeChooseScene = nullptr;

};
#endif // MAINSCENE_H
