#ifndef GAME_H
#define GAME_H

#include <QWidget>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QSoundEffect>
#include <QMediaPlayer>

#include "player.h"
#include "objects.h"
#include "pausedialog.h"
#include "resultdialog.h"
#include "gamehelp.h"

QT_FORWARD_DECLARE_CLASS(QTextStream)

class Game : public QWidget
{
    Q_OBJECT
public:
    // explicit Game(QWidget *parent = nullptr);
    Game(const bool & Mode , const int & row_num , const int & col_num , const int & Style);
    Game(const int &i);
    ~Game();

    void paintEvent(QPaintEvent * );
    void keyPressEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *ev);
    void changeEvent(QEvent * event);
    void retranslateUi(Game *game);

    enum SaveFormat { Json, Binary };
    bool loadGame(SaveFormat saveFormat, int index);
    bool saveGame(SaveFormat saveFormat,QString dirPath) const;
    void read(const QJsonObject &json);
    void load(const int & i);
    QJsonObject toJson() const;
    bool archive() const;

    void mapInit();
    void figInit();
    bool isSolveable() const;
    int* getPath() const;
    bool search_line(const int &  centre , const int & target ,const Direction & dirct, int * & path , int & pp) const;
    bool search_1(const int &  centre , const int &  target , const Direction & dirct ,int *&path,int & pp) const;
    bool search_2(const int &  centre ,const int & target , const Direction & dirct ,int *&path,int & pp) const;
    void shuffle();
    void generateProp();
    void gainProp(int target , Player *& player);
    void recordPath(const int & pl, const Direction & cur_dirct, Player *& player);
    void eliminate(const int & target , const Direction & dirct , Player *& player);
    void hint();
    bool getFlashPath(const int & from , const int & to, Player *&player);
    bool flash_search_line(const int & from , const int & to , const Direction & dirct, Player *&player);
    bool flash_search_1(const int & from , const int & to , const Direction & drt, Player *&player);
    bool flash_search_2(const int & from , const int & to , const Direction & drt, Player *&player);
    void fail();
    void win();

    int move(int i ,const Direction & drct) const;
    bool turn(const Direction & cur_dirct , Player *& player) const;
    bool canMoveTo(const int & to) const;
    bool findWayOut(const int & basis) const;
    int getX(const int & i) const;
    int getY(const int & i) const;
    bool inBlockArea(int i, Direction dirct) const;
    Direction rotate_cw(const Direction & dirct) const;
    Direction rotate_ccw(const Direction & dirct) const;

    bool mode;
    int clock , prop_num ,col, row, type_num , style,
        /*w , h ,*/ left , right , top , bottom,
        /*selected = -1 ,*/ cur_num /*, turnings , score = 0*/ ,
        hint1 = -1 , hint2 = -1 , prop_interval = 6 ,
        hint_clk = 0 , flash_clk = 0 , draw_path_edge = -1;

    const int ScreenHeight = 480 , ScreenWidth = 800 , step = 40 , w = 12 , h = 12 ,
        character_width = 160 , character_height = 300 , btn_width = 100 , btn_height = 60;
    Objects * map = nullptr;
    QPixmap pix_player_0 , pix_player_1,pix_player_2,
        pix_type1,pix_type2,pix_type3, pix_type4,pix_type5,pix_type6 ,
        pix_ExtraTime , pix_Shuffle , pix_Flash,pix_Hint , pix_Freeze , pix_Dizzy,
        pix_Normal , pix_TomSelect , pix_JerrySelect,pix_Highlighted,
        pix_DizzyEffect,pix_FreezeEffect, pix_background , pix_character_1,
        pix_character_2, pix_infoboard_1, pix_infoboard_2 , pix_back , pix_FlashEffect;

    Player * player_0 = nullptr , * player_1 = nullptr , * player_2 = nullptr;

    int draw_path[4] = {-1};
    bool  hint_flag = false , flash_flag = false , game_flag = true;
    QPushButton * btn_pause = nullptr;
    PauseDialog * pauseDialog = nullptr;
    ResultDialog * resultDialog = nullptr;
    GameHelp * helpDialog = nullptr;

    QString str_clock_o = tr("剩余时长：%1 秒"),
            str_score_o = tr("得分：%1 分"),
            str_hint_o = tr("提示 剩余时长：%1 秒"),
            str_flash_o = tr("瞬移 剩余时长：%1 秒"),
            str_dizzy_o =tr("晕眩 剩余时长：%1 秒"),
            str_freeze_o = tr("冰冻 剩余时长：%1 秒"),
            str_link_1 = tr("请用两次以内的折线"),
            str_link_2 = tr("连接！");

    QSoundEffect *se_click = nullptr , * se_eliminate = nullptr , * se_prop = nullptr , * bgm_player = nullptr;

signals:
    void highlight();
    void backMain();
    void showArchiveDialog();
    void showSettingDialog();
};

#endif // GAME_H
