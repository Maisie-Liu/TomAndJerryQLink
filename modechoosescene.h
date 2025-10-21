#ifndef MODECHOOSESCENE_H
#define MODECHOOSESCENE_H

#include <QWidget>
#include <QSoundEffect>
#include <QRect>

#include "game.h"

namespace Ui {
class ModeChooseScene;
}

class ModeChooseScene : public QWidget
{
    Q_OBJECT

public:
    explicit ModeChooseScene(QWidget *parent = nullptr);
    ~ModeChooseScene();
    void init();
    Game * game = nullptr;
    void changeEvent(QEvent * event);
    QSoundEffect *se_click = nullptr;
    QRect geo_game;

signals:
    void back();
    void game_back();
    void showArchiveDialog();
    void showSettingDialog();
    void main_bgm_stop();

private:
    Ui::ModeChooseScene *ui;
};

#endif // MODECHOOSESCENE_H
