#include "game.h"
#include "objects.h"
#include "player.h"
#include "prop.h"
#include "pausedialog.h"
#include "resultdialog.h"
#include "gamehelp.h"

extern bool bgm,se;

#include <QApplication>
#include <QTimer>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QString>
#include <QPen>
#include <QDir>
#include <QCborMap>
#include <QCborValue>
#include <QFile>
#include <QStandardPaths>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTextStream>
#include <QPushButton>
#include <QFont>
#include <QDialog>
#include <QIcon>
#include <QSoundEffect>

#include <ctime>
#include <cstdlib>
#include <vector>

using namespace Qt::StringLiterals;


Game::Game(const bool & Mode, const int &row_num, const int &col_num, const int & Style) : mode(Mode) , col(col_num) ,
    row(row_num) , type_num((col_num + row_num) / 4 + 1) , style(Style)
{
    // Init click sound effect.
    se_click = new QSoundEffect();
    se_click->setSource(QUrl::fromLocalFile(":/res/mainScene/click.wav"));
    se_click->setLoopCount(1);

    // Init eliminate sound effect.
    se_eliminate = new QSoundEffect();
    se_eliminate->setSource(QUrl::fromLocalFile(":/res/Game/bgm/eliminate.wav"));
    se_eliminate->setLoopCount(1);

    // Init prop-getting sound effect.
    se_prop = new QSoundEffect();
    se_prop->setSource(QUrl::fromLocalFile(":/res/Game/bgm/gainProp.wav"));
    se_prop->setLoopCount(1);

    // Init background music.
    bgm_player = new QSoundEffect;
    QString style_dir;
    switch (style) {
    case 0:
        style_dir = "CatFishing";
        break;
    case 1:
        style_dir = "ChristmasEve";
        break;
    case 2:
        style_dir = "HeavenlyPuss";
        break;
    default:
        break;
    }
    bgm_player->setSource(QUrl::fromLocalFile(":/res/"+ style_dir + "/bgm/bgm_game.wav"));
    bgm_player->setLoopCount(QSoundEffect::Infinite);
    if(bgm)
        bgm_player->play();

    // Basic window setting.
    setWindowTitle(tr("猫和老鼠连连看"));
    setFocusPolicy(Qt::StrongFocus);
    QPixmap Icon(":/res/Game/img/Tom.png");
    setWindowIcon(QIcon(Icon));

    // Init prop_num and player
    // mode is true: single
    if(mode){
        prop_num = 4;
        player_0 = new Player;
    }

    //mode is false: two
    else{
        prop_num = 5;
        player_1 = new Player;
        player_2 = new Player;
        player_1->opponent = player_2;
        player_2->opponent = player_1;
    }

    // Init clock
    clock = col * row * 2;

    // Init the map
    setMinimumSize(ScreenWidth , ScreenHeight);
    left = (w - col) / 2 ;
    right = left + col -1;
    top = (h - row) / 2 ;
    bottom = top + row - 1 ;
    cur_num = col * row;

    map = new Objects[w * h];
    srand(time(nullptr));
    mapInit();

    // check whether the blocks are solvable
    while(!isSolveable()){
        shuffle();
    }

    // Timing.
    QTimer * timer =  new QTimer(this), * timer_clk =  new QTimer(this);

    // PaintEvent update.
    connect (timer, &QTimer::timeout, this, QOverload<>::of(&Game::update));

    // Game timer.
    connect(timer_clk,&QTimer::timeout,this,[=](){
        if(game_flag){
            if(clock > 0){
                --clock;
                if(clock % prop_interval == 1)
                    generateProp();
                if(hint_flag){
                    if(hint_clk > 1)
                        --hint_clk;
                    else{
                        map[hint1].isHighlighted = map[hint2].isHighlighted = false;
                        hint1 = hint2 = -1;
                        hint_flag = false;
                    }
                }
                if(mode){
                    if(flash_flag){
                        if(flash_clk > 1)
                            --flash_clk;
                        else
                            flash_flag = false;
                    }
                }
                else{
                    if(player_1->freeze_flag){
                        if(player_1->freeze_clk > 1)
                            --player_1->freeze_clk;
                        else{
                            player_1->freeze_flag = false;
                        }
                    }
                    if(player_2->freeze_flag){
                        if(player_2->freeze_clk > 1)
                            --player_2->freeze_clk;
                        else{
                            player_2->freeze_flag = false;
                        }
                    }
                    if(player_1->dizzy_flag){
                        if(player_1->dizzy_clk > 1)
                            --player_1->dizzy_clk;
                        else{
                            player_1->dizzy_flag = false;
                        }
                    }
                    if(player_2->dizzy_flag){
                        if(player_2->dizzy_clk > 1)
                            --player_2->dizzy_clk;
                        else{
                            player_2->dizzy_flag = false;
                        }
                    }
                }
            }
            else{
                game_flag = hint_flag = flash_flag = false;
                fail();
            }
        }
    });
    timer->start(30);
    timer_clk->start(1000);

    // Init player property
    if(mode){
        player_0->loc = 0;
        map[0].state = Objects::player;
    }
    else{
        player_1->loc = 0;
        map[0].state = Objects::player;
        player_2->loc = w * h - 1;
        map[player_2->loc].state = Objects::player;
    }

    // Init figures
    figInit();

    // Init pause button
    btn_pause = new QPushButton(this);
    btn_pause->setGeometry(ScreenWidth - btn_width , ScreenHeight - btn_height,btn_width , btn_height);
    btn_pause->setStyleSheet("border-image: url(:/res/mainScene/optionBtn.PNG);");
    QFont font;
    font.setFamily("楷体");
    font.setPointSize(20);
    btn_pause->setFont(font);
    btn_pause->setText(tr("暂停"));

    // Connect signals and slots
    connect(this , &Game::highlight , this , [=](){
        hint();
    });

    // Init pauseDialog
    pauseDialog = new PauseDialog();
    pauseDialog->hide();
    connect(btn_pause,&QPushButton::clicked,this,[=](){
        if(se)
            se_click->play();
        game_flag = false;
        pauseDialog->exec();
    });
    connect(pauseDialog,&PauseDialog::continuing,this,[=](){
        game_flag = true;
    });
    connect(pauseDialog,&QDialog::rejected,this,[=](){
        game_flag = true;
    });
    connect(pauseDialog,&PauseDialog::back,this,[=](){
        delete pauseDialog;
        pauseDialog = nullptr;
        emit backMain();
    });
    connect(pauseDialog,&PauseDialog::saving,this,[=](){
        if(archive()){
            qDebug() << "Archive successfully!";
            pauseDialog->saved = true;
        }
        else{
            qDebug() << "Archive failed.";
        }
    });

    connect(pauseDialog,&PauseDialog::showArchiveDialog,this,&Game::showArchiveDialog);

    connect(pauseDialog,&PauseDialog::setting,this,&Game::showSettingDialog);

    // Init helpDialog
    helpDialog = new GameHelp(this);
    helpDialog->hide();

    connect(pauseDialog,&PauseDialog::help,this,[=](){
        if(se)
            se_click->play();
        helpDialog->exec();
    });

    // Init resultDialog
    resultDialog = new ResultDialog(this);
    resultDialog->hide();

    connect(resultDialog,&ResultDialog::rejected,this,[=](){
        emit backMain();
    });
}

Game::Game(const int & i)
{
    // Init click sound effect..
    se_click = new QSoundEffect();
    se_click->setSource(QUrl::fromLocalFile(":/res/mainScene/click.wav"));
    se_click->setLoopCount(1);

    // Init eliminate sound effect.
    se_eliminate = new QSoundEffect();
    se_eliminate->setSource(QUrl::fromLocalFile(":/res/Game/bgm/eliminate.wav"));
    se_eliminate->setLoopCount(1);

    // Init prop-getting sound effect.
    se_prop = new QSoundEffect();
    se_prop->setSource(QUrl::fromLocalFile(":/res/Game/bgm/gainProp.wav"));
    se_prop->setLoopCount(1);

    // Basic window setting.
    setFocusPolicy(Qt::StrongFocus);

    setWindowTitle(tr("猫和老鼠连连看"));
    QPixmap Icon(":/res/Game/img/Tom.png");
    setWindowIcon(QIcon(Icon));
    setMinimumSize(ScreenWidth , ScreenHeight);
    srand(time(nullptr));

    // Load the game. 'i' indicates the position of the archive file in the dictionary.
    load(i);

    // Timing.
    QTimer * timer =  new QTimer(this), * timer_clk =  new QTimer(this);

    // PaintEvent update.
    connect (timer, &QTimer::timeout, this, QOverload<>::of(&Game::update));

    // Game timer.
    connect(timer_clk,&QTimer::timeout,this,[=](){
        if(game_flag){
            if(clock > 0){
                --clock;
                if(clock % prop_interval == 1)
                    generateProp();
                if(hint_flag){
                    if(hint_clk > 1)
                        --hint_clk;
                    else{
                        map[hint1].isHighlighted = map[hint2].isHighlighted = false;
                        hint1 = hint2 = -1;
                        hint_flag = false;
                    }
                }
                if(mode){
                    if(flash_flag){
                        if(flash_clk > 1)
                            --flash_clk;
                        else
                            flash_flag = false;
                    }
                }
                else{
                    if(player_1->freeze_flag){
                        if(player_1->freeze_clk > 1)
                            --player_1->freeze_clk;
                        else{
                            player_1->freeze_flag = false;
                        }
                    }
                    if(player_2->freeze_flag){
                        if(player_2->freeze_clk > 1)
                            --player_2->freeze_clk;
                        else{
                            player_2->freeze_flag = false;
                        }
                    }
                    if(player_1->dizzy_flag){
                        if(player_1->dizzy_clk > 1)
                            --player_1->dizzy_clk;
                        else{
                            player_1->dizzy_flag = false;
                        }
                    }
                    if(player_2->dizzy_flag){
                        if(player_2->dizzy_clk > 1)
                            --player_2->dizzy_clk;
                        else{
                            player_2->dizzy_flag = false;
                        }
                    }
                }
            }
            else{
                game_flag = hint_flag = flash_flag = false;
                fail();
            }
        }
    });
    timer->start(30);
    timer_clk->start(1000);

    // Init pause button
    btn_pause = new QPushButton(this);
    btn_pause->setGeometry(ScreenWidth - btn_width , ScreenHeight - btn_height,btn_width , btn_height);
    btn_pause->setStyleSheet("border-image: url(:/res/mainScene/optionBtn.PNG);");
    QFont font;
    font.setFamily("楷体");
    font.setPointSize(20);
    btn_pause->setFont(font);
    btn_pause->setText(tr("暂停"));

    // Handle signals and slots
    connect(this , &Game::highlight , this , [=](){
        hint();
    });

    // Init pauseDialog
    pauseDialog = new PauseDialog(/*this*/);
    pauseDialog->hide();
    connect(btn_pause,&QPushButton::clicked,this,[=](){
        game_flag = false;
        pauseDialog->exec();
    });
    connect(pauseDialog,&PauseDialog::continuing,this,[=](){
        game_flag = true;
    });
    connect(pauseDialog,&QDialog::rejected,this,[=](){
        game_flag = true;
    });
    connect(pauseDialog,&PauseDialog::back,this,[=](){
        delete pauseDialog;
        pauseDialog = nullptr;
        emit backMain();
    });
    connect(pauseDialog,&PauseDialog::saving,this,[=](){
        if(archive()){
            qDebug() << "Archive successfully!";
            pauseDialog->saved = true;
        }
        else{
            qDebug() << "Archive failed.";
        }
    });

    connect(pauseDialog,&PauseDialog::showArchiveDialog,this,&Game::showArchiveDialog);

    connect(pauseDialog,&PauseDialog::setting,this,&Game::showSettingDialog);

    // Init helpDialog
    helpDialog = new GameHelp(this);
    helpDialog->hide();

    connect(pauseDialog,&PauseDialog::help,this,[=](){
        if(se)
            se_click->play();
        helpDialog->exec();
    });

    // Init resultDialog
    resultDialog = new ResultDialog(this);
    resultDialog->hide();

    connect(resultDialog,&ResultDialog::rejected,this,[=](){
        emit backMain();
    });
}

Game::~Game()
{
    delete se_click;
    delete se_eliminate;
    delete se_prop;

    if(bgm)
        bgm_player->stop();
    delete bgm_player;

    if(map != nullptr){
        delete [] map;
    }
    if(player_0 != nullptr){
        delete player_0;
    }
    if(player_1 != nullptr){
        delete player_1;
    }
    if(player_2 != nullptr){
        delete player_2;
    }
    if(pauseDialog != nullptr)
    delete pauseDialog;
}

void Game::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Resize
    painter.scale(width() / (ScreenWidth + 0.0), height() / (ScreenHeight + 0.0));

    btn_pause->setGeometry((ScreenWidth - btn_width) * (width() / (ScreenWidth + 0.0)) ,
                            (ScreenHeight - btn_height) * (height() / (ScreenHeight + 0.0)),
                            btn_width * (width() / (ScreenWidth + 0.0)) ,
                            btn_height * (height() / (ScreenHeight + 0.0)));

    // Draw background image
    painter.drawPixmap(0,0,ScreenWidth,ScreenHeight,pix_background);
    painter.drawPixmap(character_width,0,ScreenHeight,ScreenHeight,pix_back);

    // Draw character image
    painter.drawPixmap(0,ScreenHeight - character_height,character_width,character_height,pix_character_1);
    painter.drawPixmap(0,0,character_width,ScreenHeight - character_height,pix_infoboard_1);
    if(!mode){
        painter.drawPixmap(ScreenWidth - character_width,0,character_width,ScreenHeight - character_height,pix_infoboard_2);
        painter.drawPixmap(ScreenWidth - character_width,ScreenHeight - character_height,character_width,character_height,pix_character_2);
    }

    // Draw game area
    for(int i = 0 ; i < w * h ; ++i){
        switch (map[i].state) {
        case Objects::block:
            // Draw the undercoat, which indicate whether the block is selected or highlighted.
            if(map[i].isHighlighted){
                painter.drawPixmap(map[i].pos.x(),map[i].pos.y(),step,step,pix_Highlighted);
            }

            if(mode){
                if(map[i].isSelected){
                    painter.drawPixmap(map[i].pos.x(),map[i].pos.y(),step,step,pix_TomSelect);
                }
                else{
                    if(map[i].isHighlighted){
                        painter.drawPixmap(map[i].pos.x(),map[i].pos.y(),step,step,pix_Highlighted);
                    }
                    else{
                        painter.drawPixmap(map[i].pos.x(),map[i].pos.y(),step,step,pix_Normal);
                    }
                }
            }
            else{
                if(map[i].isSelected_1){
                    painter.drawPixmap(map[i].pos.x(),map[i].pos.y(),step,step,pix_TomSelect);
                }
                // If select together, only show the undercoat of player1.
                // Whoever eliminate the block firstly can gain the point, and the other would have to select other blocks.
                else if(map[i].isSelected_2){
                    painter.drawPixmap(map[i].pos.x(),map[i].pos.y(),step,step,pix_JerrySelect);
                }
                else{
                    if(map[i].isHighlighted){
                        painter.drawPixmap(map[i].pos.x(),map[i].pos.y(),step,step,pix_Highlighted);
                    }
                    else
                        painter.drawPixmap(map[i].pos.x(),map[i].pos.y(),step,step,pix_Normal);
                }
            }

            // Draw the item
            switch (map[i].type) {
            case Block::type1:
                 painter.drawPixmap(map[i].pos.x(),map[i].pos.y(),step,step,pix_type1);
                break;
            case Block::type2:
                 painter.drawPixmap(map[i].pos.x(),map[i].pos.y(),step,step,pix_type2);
                break;
            case Block::type3:
                 painter.drawPixmap(map[i].pos.x(),map[i].pos.y(),step,step,pix_type3);
                break;
            case Block::type4:
                 painter.drawPixmap(map[i].pos.x(),map[i].pos.y(),step,step,pix_type4);
                break;
            case Block::type5:
                 painter.drawPixmap(map[i].pos.x(),map[i].pos.y(),step,step,pix_type5);
                break;
            case Block::type6:
                 painter.drawPixmap(map[i].pos.x(),map[i].pos.y(),step,step,pix_type6);
                break;
            default:
                break;
            }
            break;

        // Draw the props
        case Objects::prop:
            switch(map[i].kind) {
            case Prop::ExtraTime:
                 painter.drawPixmap(map[i].pos.x(),map[i].pos.y(),step,step,pix_ExtraTime);
                break;
            case Prop::Shuffle:
                 painter.drawPixmap(map[i].pos.x(),map[i].pos.y(),step,step,pix_Shuffle);
                break;
            case Prop::Flash:
                 painter.drawPixmap(map[i].pos.x(),map[i].pos.y(),step,step,pix_Flash);
                break;
            case Prop::Hint:
                 painter.drawPixmap(map[i].pos.x(),map[i].pos.y(),step,step,pix_Hint);
                break;
            case Prop::Freeze:
                 painter.drawPixmap(map[i].pos.x(),map[i].pos.y(),step,step,pix_Freeze);
                break;
            case Prop::Dizzy:
                 painter.drawPixmap(map[i].pos.x(),map[i].pos.y(),step,step,pix_Dizzy);
                break;
            default:
                break;
            }
            break;

        // Draw the players
        case Objects::player:
            if(mode){
                if(flash_flag)
                    painter.drawPixmap(map[i].pos.x(),map[i].pos.y(),step,step,pix_FlashEffect);
                painter.drawPixmap(map[i].pos.x(),map[i].pos.y(),step,step,pix_player_0);
            }
            else{
                if(i == player_1->loc){
                    if(player_1->dizzy_flag){
                        painter.drawPixmap(map[i].pos.x(),map[i].pos.y(),step,step,pix_DizzyEffect);
                    }
                    painter.drawPixmap(map[i].pos.x(),map[i].pos.y(),step,step,pix_player_1);
                    if(player_1->freeze_flag){
                        painter.drawPixmap(map[i].pos.x(),map[i].pos.y() + step / 2,step,step / 2,pix_FreezeEffect);
                    }
                }
                else{
                    if(player_2->dizzy_flag){
                        painter.drawPixmap(map[i].pos.x(),map[i].pos.y(),step,step,pix_DizzyEffect);
                    }
                    painter.drawPixmap(map[i].pos.x(),map[i].pos.y(),step,step,pix_player_2);
                    if(player_2->freeze_flag){
                        painter.drawPixmap(map[i].pos.x(),map[i].pos.y() + step / 2,step,step / 2,pix_FreezeEffect);
                    }
                }
            }
            break;
        default:
            break;
        }
    }

    // Draw connecting lines
    QPen pen_Tom = QPen(QColor(6,15,143)),
        pen_Jerry = QPen(QColor(97,44,0));

    if(mode){
        //painter.setBrush(brush_Tom);
        if(player_0->drawPath_flag){
            // int * draw_path = new int[4];
            // for(int i = 0 ; i < draw_path_edge ; ++i){
            //     draw_path[i] = player_0->path[i];
            // }
            painter.setPen(pen_Tom);
            for(int i = 0 , j = 1 ; j < draw_path_edge ; ++i , ++j){
                painter.drawLine(QPoint(map[draw_path[i]].pos.x() + step / 2,map[draw_path[i]].pos.y() + step / 2) ,
                                 QPoint(map[draw_path[j]].pos.x() + step / 2,map[draw_path[j]].pos.y() + step / 2));
            }
            QTimer::singleShot(100,this,[=](){
                player_0->drawPath_flag = false;
                draw_path_edge = -1;
            });
        }
    }
    else{
        if(player_1->drawPath_flag){
            painter.setPen(pen_Tom);
            for(int i = 0 , j = 1 ; j < draw_path_edge ; ++i , ++j){
                painter.drawLine(QPoint(map[draw_path[i]].pos.x() + step / 2,map[draw_path[i]].pos.y() + step / 2) ,
                                 QPoint(map[draw_path[j]].pos.x() + step / 2,map[draw_path[j]].pos.y() + step / 2));
            }
            QTimer::singleShot(100,this,[=](){
                player_1->drawPath_flag = false;
                draw_path_edge = -1;
            });
        }
        if(player_2->drawPath_flag){
            painter.setPen(pen_Jerry);
            for(int i = 0 , j = 1 ; j < draw_path_edge ; ++i , ++j){
                painter.drawLine(QPoint(map[draw_path[i]].pos.x() + step / 2,map[draw_path[i]].pos.y() + step / 2) ,
                                 QPoint(map[draw_path[j]].pos.x() + step / 2,map[draw_path[j]].pos.y() + step / 2));
            }
            QTimer::singleShot(100,this,[=](){
                player_2->drawPath_flag = false;
                draw_path_edge = -1;
            });
        }
    }

    // Draw game state
    if(mode){
        painter.setPen(pen_Tom);
        //clock
        QString str_clock = QString(str_clock_o).arg(clock);
        painter.drawText(20,30,str_clock);

        //score
        QString str_score = QString(str_score_o).arg(player_0->score);
        painter.drawText(20,50,str_score);

        //prop
        if(hint_flag){
            QString str_hint = QString(str_hint_o).arg(hint_clk);
            painter.drawText(20,70,str_hint);
        }
        if(flash_flag){
            QString str_flash = QString(str_flash_o).arg(flash_clk);
            painter.drawText(20,90,str_flash);
        }
        if(player_0->warn_flag){
            painter.drawText(20,110,str_link_1);
            painter.drawText(20,130,str_link_2);
        }
    }
    else{
        //player_1
        painter.setPen(pen_Tom);
        //clock
        QString str_clock = QString(str_clock_o).arg(clock);
        painter.drawText(20,30,str_clock);

        //score
        QString str_score = QString(str_score_o).arg(player_1->score);
        painter.drawText(20,50,str_score);

        //prop
        if(hint_flag){
            QString str_hint = QString(str_hint_o).arg(hint_clk);
            painter.drawText(20,70,str_hint);
        }
        if(player_1->dizzy_flag){
            QString str_dizzy = QString(str_dizzy_o).arg(player_1->dizzy_clk);
            painter.drawText(20,90,str_dizzy);
        }
        if(player_1->freeze_flag){
            QString str_freeze = QString(str_freeze_o).arg(player_1->freeze_clk);
            painter.drawText(20,110,str_freeze);
        }
        if(player_1->warn_flag){
            painter.drawText(20,130,str_link_1);
            painter.drawText(20,150,str_link_2);
        }

        //player_2
        painter.setPen(pen_Jerry);
        //clock
        QString str_clock_2 = QString(str_clock_o).arg(clock);
        painter.drawText(ScreenWidth - character_width + 20,30,str_clock_2);

        //score
        QString str_score_2 = QString(str_score_o).arg(player_2->score);
        painter.drawText(ScreenWidth - character_width + 20,50,str_score_2);

        //prop
        if(hint_flag){
            QString str_hint = QString(str_hint_o).arg(hint_clk);
            painter.drawText(ScreenWidth - character_width + 20,70,str_hint);
        }
        if(player_2->dizzy_flag){
            QString str_dizzy = QString(str_dizzy_o).arg(player_2->dizzy_clk);
            painter.drawText(ScreenWidth - character_width + 20,90,str_dizzy);
        }
        if(player_2->freeze_flag){
            QString str_freeze = QString(str_freeze_o).arg(player_2->freeze_clk);
            painter.drawText(ScreenWidth - character_width + 20,110,str_freeze);
        }
        if(player_2->warn_flag){
            painter.drawText((ScreenWidth - character_width + 20) * (width() / (ScreenWidth + 0.0)),130,str_link_1);
            painter.drawText((ScreenWidth - character_width + 20) * (width() / (ScreenWidth + 0.0)),150,str_link_2);
        }
    }
}

void Game::keyPressEvent(QKeyEvent *event)
{    
    // Game is going on
    if(game_flag){
        // Single-player mode
        if(mode){
            map[player_0->loc].state = Objects::null;
            switch(event->key()){
            case Qt::Key_W:
                if(player_0->loc >= w){
                    if(player_0->selected != -1)
                        recordPath(player_0->loc,W,player_0);
                    if(map[player_0->loc - w].state == Objects::prop){
                        player_0->loc -= w;
                        gainProp(player_0->loc , player_0->opponent);
                    }
                    else if(map[player_0->loc - w].state == Objects::block)
                        eliminate(player_0->loc - w,W , player_0);
                    else
                        player_0->loc -= w;
                }
                break;
            case Qt::Key_S:
                if(player_0->loc < w * h - w){
                    if(player_0->selected != -1)
                        recordPath(player_0->loc,S,player_0);
                    if(map[player_0->loc + w].state == Objects::prop){
                        player_0->loc += w;
                        gainProp(player_0->loc, player_0->opponent);
                    }
                    else if(map[player_0->loc + w].state == Objects::block)
                        eliminate(player_0->loc + w,S, player_0);
                    else
                        player_0->loc += w;
                }
                break;
            case Qt::Key_A:
                if(player_0->loc % w > 0){
                    if(player_0->selected != -1)
                        recordPath(player_0->loc,A,player_0);
                    if(map[player_0->loc - 1].state == Objects::prop){
                        --player_0->loc;
                        gainProp(player_0->loc, player_0->opponent);
                    }
                    else if(map[player_0->loc - 1].state == Objects::block)
                        eliminate(player_0->loc - 1,A, player_0);
                    else
                        --player_0->loc;
                }
                break;
            case Qt::Key_D:
                if(player_0->loc % w < w - 1){
                    if(player_0->selected != -1)
                        recordPath(player_0->loc,D,player_0);
                    if(map[player_0->loc + 1].state == Objects::prop){
                        ++player_0->loc;
                        gainProp(player_0->loc, player_0->opponent);
                    }
                    else if(map[player_0->loc + 1].state == Objects::block)
                        eliminate(player_0->loc + 1,D, player_0);
                    else
                        ++player_0->loc;
                }
                break;
            };
            map[player_0->loc].state = Objects::player;
        }

        // Two-player mode
        else{
            map[player_1->loc].state = Objects::null;
            map[player_2->loc].state = Objects::null;
            switch(event->key()){
            //player_1
            case Qt::Key_W:
                if(player_1->freeze_flag)
                    break;

                if(player_1->dizzy_flag){
                    if(player_1->loc < w * h - w){
                        if(player_1->selected != -1)
                            recordPath(player_1->loc,S,player_1);
                        if(map[player_1->loc + w].state == Objects::prop){
                            player_1->loc += w;
                            gainProp(player_1->loc, player_1->opponent);
                        }
                        else if(map[player_1->loc + w].state == Objects::block)
                            eliminate(player_1->loc + w,S, player_1);
                        else
                            player_1->loc += w;
                    }
                }
                else{
                    if(player_1->loc >= w){
                        if(player_1->selected != -1)
                            recordPath(player_1->loc,W,player_1);
                        if(map[player_1->loc - w].state == Objects::prop){
                            player_1->loc -= w;
                            gainProp(player_1->loc, player_1->opponent);
                        }
                        else if(map[player_1->loc - w].state == Objects::block)
                            eliminate(player_1->loc - w,W, player_1);
                        else
                            player_1->loc -= w;
                    }
                }
                break;

            case Qt::Key_S:
                if(player_1->freeze_flag)
                    break;

                if(player_1->dizzy_flag){
                    if(player_1->loc >= w){
                        if(player_1->selected != -1)
                            recordPath(player_1->loc,W,player_1);
                        if(map[player_1->loc - w].state == Objects::prop){
                            player_1->loc -= w;
                            gainProp(player_1->loc, player_1->opponent);
                        }
                        else if(map[player_1->loc - w].state == Objects::block)
                            eliminate(player_1->loc - w,W, player_1);
                        else
                            player_1->loc -= w;
                    }
                }
                else{
                    if(player_1->loc < w * h - w){
                        if(player_1->selected != -1)
                            recordPath(player_1->loc,S,player_1);
                        if(map[player_1->loc + w].state == Objects::prop){
                            player_1->loc += w;
                            gainProp(player_1->loc, player_1->opponent);
                        }
                        else if(map[player_1->loc + w].state == Objects::block)
                            eliminate(player_1->loc + w,S, player_1);
                        else
                            player_1->loc += w;
                    }
                }
                break;

            case Qt::Key_A:
                if(player_1->freeze_flag)
                    break;

                if(player_1->dizzy_flag){
                    if(player_1->loc % w < w - 1){
                        if(player_1->selected != -1)
                            recordPath(player_1->loc,D,player_1);
                        if(map[player_1->loc + 1].state == Objects::prop){
                            ++player_1->loc;
                            gainProp(player_1->loc, player_1->opponent);
                        }
                        else if(map[player_1->loc + 1].state == Objects::block)
                            eliminate(player_1->loc + 1,D, player_1);
                        else
                            ++player_1->loc;
                    }
                }
                else{
                    if(player_1->loc % w > 0){
                        if(player_1->selected != -1)
                            recordPath(player_1->loc,A,player_1);
                        if(map[player_1->loc - 1].state == Objects::prop){
                            --player_1->loc;
                            gainProp(player_1->loc, player_1->opponent);
                        }
                        else if(map[player_1->loc - 1].state == Objects::block)
                            eliminate(player_1->loc - 1,A, player_1);
                        else
                            --player_1->loc;
                    }
                }
                break;

            case Qt::Key_D:
                if(player_1->freeze_flag)
                    break;

                if(player_1->dizzy_flag){
                    if(player_1->loc % w > 1){
                        if(player_1->selected != -1)
                            recordPath(player_1->loc,A,player_1);
                        if(map[player_1->loc - 1].state == Objects::prop){
                            --player_1->loc;
                            gainProp(player_1->loc, player_1->opponent);
                        }
                        else if(map[player_1->loc - 1].state == Objects::block)
                            eliminate(player_1->loc - 1,A, player_1);
                        else
                            --player_1->loc;
                    }
                }
                else{
                    if(player_1->loc % w < w - 1){
                        if(player_1->selected != -1)
                            recordPath(player_1->loc,D,player_1);
                        if(map[player_1->loc + 1].state == Objects::prop){
                            ++player_1->loc;
                            gainProp(player_1->loc, player_1->opponent);
                        }
                        else if(map[player_1->loc + 1].state == Objects::block)
                            eliminate(player_1->loc + 1,D, player_1);
                        else
                            ++player_1->loc;
                    }
                }
                break;

            //player_2
            case Qt::Key_Up:
                if(player_2->freeze_flag)
                    break;

                if(player_2->dizzy_flag){
                    if(player_2->loc < w * h - w){
                        if(player_2->selected != -1)
                            recordPath(player_2->loc,S,player_2);
                        if(map[player_2->loc + w].state == Objects::prop){
                            player_2->loc += w;
                            gainProp(player_2->loc, player_2->opponent);
                        }
                        else if(map[player_2->loc + w].state == Objects::block)
                            eliminate(player_2->loc + w,S, player_2);
                        else
                            player_2->loc += w;
                    }
                }
                else{
                    if(player_2->loc >= w){
                        if(player_2->selected != -1)
                            recordPath(player_2->loc,W,player_2);
                        if(map[player_2->loc - w].state == Objects::prop){
                            player_2->loc -= w;
                            gainProp(player_2->loc, player_2->opponent);
                        }
                        else if(map[player_2->loc - w].state == Objects::block)
                            eliminate(player_2->loc - w,W, player_2);
                        else
                            player_2->loc -= w;
                    }
                }
                break;

            case Qt::Key_Down:
                if(player_2->freeze_flag)
                    break;

                if(player_2->dizzy_flag){
                    if(player_2->loc >= w){
                        if(player_2->selected != -1)
                            recordPath(player_2->loc,W,player_2);
                        if(map[player_2->loc - w].state == Objects::prop){
                            player_2->loc -= w;
                            gainProp(player_2->loc, player_2->opponent);
                        }
                        else if(map[player_2->loc - w].state == Objects::block)
                            eliminate(player_2->loc - w,W, player_2);
                        else
                            player_2->loc -= w;
                    }
                }
                else{
                    if(player_2->loc < w * h - w){
                        if(player_2->selected != -1)
                            recordPath(player_2->loc,S,player_2);
                        if(map[player_2->loc + w].state == Objects::prop){
                            player_2->loc += w;
                            gainProp(player_2->loc, player_2->opponent);
                        }
                        else if(map[player_2->loc + w].state == Objects::block)
                            eliminate(player_2->loc + w,S, player_2);
                        else
                            player_2->loc += w;
                    }
                }
                break;

            case Qt::Key_Left:
                if(player_2->freeze_flag)
                    break;

                if(player_2->dizzy_flag){
                    if(player_2->loc % w < w - 1){
                        if(player_2->selected != -1)
                            recordPath(player_2->loc,D,player_2);
                        if(map[player_2->loc + 1].state == Objects::prop){
                            ++player_2->loc;
                            gainProp(player_2->loc, player_2->opponent);
                        }
                        else if(map[player_2->loc + 1].state == Objects::block)
                            eliminate(player_2->loc + 1,D, player_2);
                        else
                            ++player_2->loc;
                    }
                }
                else{
                    if(player_2->loc % w > 0){
                        if(player_2->selected != -1)
                            recordPath(player_2->loc,A,player_2);
                        if(map[player_2->loc - 1].state == Objects::prop){
                            --player_2->loc;
                            gainProp(player_2->loc, player_2->opponent);
                        }
                        else if(map[player_2->loc - 1].state == Objects::block)
                            eliminate(player_2->loc - 1,A, player_2);
                        else
                            --player_2->loc;
                    }
                }
                break;

            case Qt::Key_Right:
                if(player_2->freeze_flag)
                    break;

                if(player_2->dizzy_flag){
                    if(player_2->loc % w > 1){
                        if(player_2->selected != -1)
                            recordPath(player_2->loc,A,player_2);
                        if(map[player_2->loc - 1].state == Objects::prop){
                            --player_2->loc;
                            gainProp(player_2->loc, player_2->opponent);
                        }
                        else if(map[player_2->loc - 1].state == Objects::block)
                            eliminate(player_2->loc - 1,A, player_2);
                        else
                            --player_2->loc;
                    }
                }
                else{
                    if(player_2->loc % w < w - 1){
                        if(player_2->selected != -1)
                            recordPath(player_2->loc,D,player_2);
                        if(map[player_2->loc + 1].state == Objects::prop){
                            ++player_2->loc;
                            gainProp(player_2->loc, player_2->opponent);
                        }
                        else if(map[player_2->loc + 1].state == Objects::block)
                            eliminate(player_2->loc + 1,D, player_2);
                        else
                            ++player_2->loc;
                    }
                }
                break;

            };
            map[player_1->loc].state = Objects::player;
            map[player_2->loc].state = Objects::player;
        }


    }
}

void Game::mousePressEvent(QMouseEvent *ev)
{
    // Enable condition: Flash is triggered and it is single-player mode, as well as the game is going on.
    if(flash_flag && mode && game_flag){
        // Gain and tansfer the positon user clicked
        int px = (ev->pos().x() - character_width * (width() / (ScreenWidth + 0.0))) / (step * (width() / (ScreenWidth + 0.0)))  , py = ev->pos().y() / (step * (height() / (ScreenHeight + 0.0))) , p;
        p = py * w + px;

        player_0->turnings = 0;
        map[player_0->loc].state = Objects::null;
        if(p >= 0 && p < w * h && canMoveTo(p)){
            if(map[p].state == Objects::block){
                // It is null or player a step from the block.
                if(map[move(p,W)].state == Objects::null || map[move(p,W)].state == Objects::player){
                    player_0->loc = move(p , W);
                    if(player_0->selected == -1)
                        eliminate(p,S,player_0);
                    else
                        eliminate(p,m,player_0);
                }
                else if(map[move(p,S)].state == Objects::null || map[move(p,S)].state == Objects::player){
                    player_0->loc = move(p , S);
                    if(player_0->selected == -1)
                        eliminate(p,W,player_0);
                    else
                        eliminate(p,m,player_0);
                }
                else if(map[move(p,A)].state == Objects::null || map[move(p,A)].state == Objects::player){
                    player_0->loc = move(p , A);
                    if(player_0->selected == -1)
                        eliminate(p,D,player_0);
                    else
                        eliminate(p,m,player_0);
                }
                else if(map[move(p,D)].state == Objects::null || map[move(p,D)].state == Objects::player){
                    player_0->loc = move(p , D);
                    if(player_0->selected == -1)
                        eliminate(p,A,player_0);
                    else
                        eliminate(p,m,player_0);
                }

                // It is pro a step from the block.
                else if(map[move(p,W)].state == Objects::prop){
                    player_0->loc = move(p , W);
                    gainProp(move(p,W),player_0);
                    if(player_0->selected == -1)
                        eliminate(p,S,player_0);
                    else
                        eliminate(p,m,player_0);
                }
                else if(map[move(p,S)].state == Objects::prop){
                    player_0->loc = move(p , S);
                    gainProp(move(p,S),player_0);
                    if(player_0->selected == -1)
                        eliminate(p,W,player_0);
                    else
                        eliminate(p,m,player_0);
                }
                else if(map[move(p,A)].state == Objects::prop){
                    player_0->loc = move(p , A);
                    gainProp(move(p,A),player_0);
                    if(player_0->selected == -1)
                        eliminate(p,D,player_0);
                    else
                        eliminate(p,m,player_0);
                }
                else if(map[move(p,D)].state == Objects::prop){
                    player_0->loc = move(p , D);
                    gainProp(move(p,D),player_0);
                    if(player_0->selected == -1)
                        eliminate(p,A,player_0);
                    else
                        eliminate(p,m,player_0);
                }
            }

            // It is prop.
            else if(map[p].state == Objects::prop){
                gainProp(p,player_0);
                player_0->loc = p;
            }

            // It is null or player
            else
                player_0->loc = p;
        }
        map[player_0->loc].state = Objects::player;
    }
}

// Handle the language-change event.
void Game::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    switch(event->type()){
    case QEvent::LanguageChange:
        retranslateUi(this);
        break;
    default:
        break;
    }
}

// Retranslate the ui.
void Game::retranslateUi(Game * game)
{
    game->setWindowTitle(QApplication::translate("Game","猫和老鼠连连看",nullptr));
    btn_pause->setText(QApplication::translate("Game","暂停",nullptr));
    str_clock_o = QApplication::translate("Game","剩余时长：%1 秒",nullptr);
    str_score_o = QApplication::translate("Game","得分：%1 分",nullptr);
    str_hint_o = QApplication::translate("Game","提示 剩余时长：%1 秒",nullptr);
    str_flash_o = QApplication::translate("Game","瞬移 剩余时长：%1 秒",nullptr);
    str_dizzy_o = QApplication::translate("Game","晕眩 剩余时长：%1 秒",nullptr);
    str_freeze_o = QApplication::translate("Game","冰冻 剩余时长：%1 秒",nullptr);
    str_link_1 = QApplication::translate("Game","请用两次以内的折线",nullptr);
    str_link_2 = QApplication::translate("Game","连接！",nullptr);
}

bool Game::loadGame(SaveFormat saveFormat , int index)
{
    //Get archive dir path and determine whether there are any documents.
    QString archiveDirPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/archive/";
    QDir * folder = new QDir(archiveDirPath);
    if(!folder->exists() || folder->isEmpty()){
        qDebug() << "No archive.";
        return true;
    }

    QStringList nameList = folder->entryList(QDir::Files | QDir::NoDotAndDotDot,QDir::Time);

    QFile loadFile(saveFormat == Json ?
                       archiveDirPath + "/" + nameList.at(index)/*_L1*/
                                      : archiveDirPath + "/" + nameList.at(index)/*_L1*/);

    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }

    QByteArray saveData = loadFile.readAll();

    QJsonDocument loadDoc(saveFormat == Json
                              ? QJsonDocument::fromJson(saveData)
                              : QJsonDocument(QCborValue::fromCbor(saveData).toMap().toJsonObject()));

    read(loadDoc.object());
    figInit();

    if(bgm && bgm_player != nullptr){
        bgm_player->stop();
        delete bgm_player;
    }
    bgm_player = new QSoundEffect;
    QString style_dir;
    switch (style) {
    case 0:
        style_dir = "CatFishing";
        break;
    case 1:
        style_dir = "ChristmasEve";
        break;
    case 2:
        style_dir = "HeavenlyPuss";
        break;
    default:
        break;
    }
    bgm_player->setSource(QUrl::fromLocalFile(":/res/"+ style_dir + "/bgm/bgm_game.wav"));
    bgm_player->setLoopCount(QSoundEffect::Infinite);
    if(bgm)
        bgm_player->play();

    QTextStream(stdout) << "Loaded save for " << nameList.at(index)
                        << " using " << (saveFormat != Json ? "CBOR" : "JSON") << "...\n";
    loadFile.close();
    return true;
}

bool Game::saveGame(SaveFormat saveFormat ,QString dirPath) const
{
    QDateTime::currentDateTime();
    QString style_dir , mode_str;
    switch (style) {
    case 0:
        style_dir = "CatFishing_";
        break;
    case 1:
        style_dir = "ChristmasEve_";
        break;
    case 2:
        style_dir = "HeavenlyPuss_";
        break;
    default:
        break;
    }
    if(mode)
        mode_str = "single_";
    else{
        mode_str = "double_";
    }
    QFile saveFile(saveFormat == Json ?
                       dirPath + style_dir + mode_str + QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss") +".json"/*_L1*/
                                      : dirPath  + style_dir + mode_str + QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss") + ".dat"/*_L1*/);

    if (!saveFile.open(QIODevice::WriteOnly)) {
        qDebug() << saveFile.errorString();
        qWarning("Couldn't open save file.");
        return false;
    }

    QJsonObject gameObject = toJson();
    saveFile.write(saveFormat == Json ? QJsonDocument(gameObject).toJson()
                                      : QCborValue::fromJsonValue(gameObject).toCbor());
    saveFile.close();
    return true;
}

void Game::read(const QJsonObject &json)
{
    if (const QJsonValue v = json["column_num"]; v.isDouble())
        col = v.toInt();
    if (const QJsonValue v = json["row_num"]; v.isDouble())
        row = v.toInt();
    if (const QJsonValue v = json["type_num"]; v.isDouble())
        type_num = v.toInt();
    if (const QJsonValue v = json["prop_num"]; v.isDouble())
        prop_num = v.toInt();
    if (const QJsonValue v = json["mode"]; v.isBool())
        mode = v.toBool();
    if (const QJsonValue v = json["style"]; v.isDouble())
        style = v.toInt();

    if (const QJsonValue v = json["left_edge"]; v.isDouble())
        left = v.toInt();
    if (const QJsonValue v = json["right_edge"]; v.isDouble())
        right = v.toInt();
    if (const QJsonValue v = json["top_edge"]; v.isDouble())
        top = v.toInt();
    if (const QJsonValue v = json["bottom_edge"]; v.isDouble())
        bottom = v.toInt();

    if (const QJsonValue v = json["remaining_num"]; v.isDouble())
        cur_num = v.toInt();

    if (const QJsonValue v = json["hint1"]; v.isDouble())
        hint1 = v.toInt();
    if (const QJsonValue v = json["hint2"]; v.isDouble())
        hint2 = v.toInt();

    if (const QJsonValue v = json["clock"]; v.isDouble())
        clock = v.toInt();
    if (const QJsonValue v = json["prop_interval"]; v.isDouble())
        prop_interval = v.toInt();
    if (const QJsonValue v = json["hint_clock"]; v.isDouble())
        hint_clk  = v.toInt();
    if (const QJsonValue v = json["flash_clock"]; v.isDouble())
        flash_clk = v.toInt();

    if (const QJsonValue v = json["game_flag"]; v.isDouble())
        game_flag = v.toBool();
    if (const QJsonValue v = json["hint_flag"]; v.isDouble())
        hint_flag = v.toBool();
    if (const QJsonValue v = json["flash_flag"]; v.isDouble())
        flash_flag = v.toBool();

    if(mode){
        if(player_1 != nullptr)
            delete player_1;
        if(player_2 != nullptr)
            delete player_2;
        if (const QJsonValue v = json["player_0"]; v.isObject()) {
            if(player_0 == nullptr)
                player_0 = new Player;
            *player_0 = Player::fromJson(v.toObject());
            if (const QJsonValue v = json["player_0->path"]; v.isArray()){
                const QJsonArray playerPath = v.toArray();
                int i = 0;
                for(const QJsonValue &turn : playerPath){
                    player_0->path[i++] = turn.toInt();
                }
            }
        }
    }
    else{
        if(player_0 != nullptr)
            delete player_0;
        if (const QJsonValue v = json["player_1"]; v.isObject()) {
            if(player_1 == nullptr)
                player_1 = new Player;
            *player_1 = Player::fromJson(v.toObject());
            if (const QJsonValue v = json["player_1->path"]; v.isArray()){
                const QJsonArray playerPath = v.toArray();
                int i = 0;
                for(const QJsonValue &turn : playerPath){
                    player_1->path[i++] = turn.toInt();
                }
            }
        }
        if (const QJsonValue v = json["player_2"]; v.isObject()) {
            if(player_2 == nullptr)
                player_2 = new Player;
            *player_2 = Player::fromJson(v.toObject());
            if (const QJsonValue v = json["player_2->path"]; v.isArray()){
                const QJsonArray playerPath = v.toArray();
                int i = 0;
                for(const QJsonValue &turn : playerPath){
                    player_2->path[i++] = turn.toInt();
                }
            }
        }
        player_1->opponent = player_2;
        player_2->opponent = player_1;
    }

    if (const QJsonValue v = json["map_objects"]; v.isArray()) {
        const QJsonArray map_objects = v.toArray();
        int i = 0;
        if(map != nullptr)
            delete [] map;
        map = new Objects[w * h];
        for(const QJsonValue &m_obj : map_objects){
            map[i++] = Objects::fromJson(m_obj.toObject());
        }
    }
}

void Game::load(const int &i)
{
    loadGame(Json,i);
    game_flag = true;
}

QJsonObject Game::toJson() const
{
    QJsonObject json;

    // Game
    json["column_num"] = col;
    json["row_num"] = row;
    json["type_num"] = type_num;
    json["prop_num"] = prop_num;
    json["mode"] = mode;
    json["style"] = style;
    json["left_edge"] = left;
    json["right_edge"] = right;
    json["top_edge"] = top;
    json["bottom_edge"] = bottom;
    json["remaining_num"] = cur_num;
    json["hint1"] = hint1;
    json["hint2"] = hint2;
    json["clock"] = clock;
    json["prop_interval"] = prop_interval;
    json["hint_clock"] = hint_clk;
    json["flash_clock"] = flash_clk;
    json["game_flag"] = game_flag;
    json["hint_flag"] = hint_flag;
    json["flash_flag"] = flash_flag;

    // player
    if(mode){
        json["player_0"] = player_0->toJson();

        QJsonArray playerPath;
        if(player_0->turnings){
            for(int i = 0 ; i < player_0->turnings; ++i){
                playerPath.append(player_0->path[i]);
            }
        }
        json["player_0->path"] = playerPath;
    }
    else{
        json["player_1"] = player_1->toJson();
        QJsonArray playerPath_1;
        if(player_1->turnings){
            for(int i = 0 ; i < player_1->turnings; ++i){
                playerPath_1.append(player_1->path[i]);
            }
        }
        json["player_1->path"] = playerPath_1;

        json["player_2"] = player_2->toJson();
        QJsonArray playerPath_2;
        if(player_2->turnings){
            for(int i = 0 ; i < player_2->turnings; ++i){
                playerPath_2.append(player_2->path[i]);
            }
        }
        json["player_2->path"] = playerPath_2;
    }

    // map
    QJsonArray map_objects;
    for(int i = 0 ; i < w * h ; ++i)
        map_objects.append(map[i].toJson());
    json["map_objects"] = map_objects;

    return json;
}

bool Game::archive() const
{
    QString archiveDirPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/archive/";
    QDir * folder = new QDir(archiveDirPath);
    qDebug() << "Archive path:" << archiveDirPath;
    if(folder->exists()){
        qDebug() << "The dictionary 'archive' exists.";
    }
    else{
        if(folder->mkpath(".")){
            qDebug() << "The dictionary 'archive' has been created successfully!";
        }
        else{
            qDebug() << "The dictionary 'archive' fails to be created.";
            return false;
        }
    }

    //write & read the archive file
    if(saveGame(Json , archiveDirPath)){
        qDebug() << "Archive successfully!";
        return true;
    }
    else{
        qDebug() << "Archive falied.";
        return false;
    }
}

void Game::mapInit()
{
    for(int i = 0 , i_b , tmp_b , tmp_w; i < w * h ; ++i){
        //position
        map[i].pos.setX(character_width + (i % w) * step);
        map[i].pos.setY((i / w) * step);

        //state
        //the state of map[i] has been assigned or map[i] is not in the block area
        if(map[i].state != Objects::null || i % w < left || i / w < top || i % w > right || i / w > bottom)
            continue;

        //map[i] is in the block area
        //each block has 4 kinds of serial number, which can be transformated as follows:
        //whole-area sequential number <-> whole-area coordinate <-> block-area coordinate <-> block-area sequential number.

        //if the type of map[tmp_w] has been assigned, then assign the value of tmp again.
        do{
            tmp_b = rand() % (col * row);
            tmp_w = tmp_b % col + left + (tmp_b / col + top) * w;
        }while(map[tmp_w].state != Objects::null);

        map[i].state = Objects::block;
        map[tmp_w].state = Objects::block;

        //swap the states of two blocks
        switch ((tmp_b / 2) % type_num + 1) {
        case 1:
            map[i].type = Block::type1;
            break;
        case 2:
            map[i].type = Block::type2;
            break;
        case 3:
            map[i].type = Block::type3;
            break;
        case 4:
            map[i].type = Block::type4;
            break;
        case 5:
            map[i].type = Block::type5;
            break;
        case 6:
            map[i].type = Block::type6;
            break;
        default:
            break;
        };

        i_b = i % w - left + (i / w - top) * col;
        switch ((i_b / 2) % type_num + 1) {
        case 1:
            map[tmp_w].type = Block::type1;
            break;
        case 2:
            map[tmp_w].type = Block::type2;
            break;
        case 3:
            map[tmp_w].type = Block::type3;
            break;
        case 4:
            map[tmp_w].type = Block::type4;
            break;
        case 5:
            map[tmp_w].type = Block::type5;
            break;
        case 6:
            map[tmp_w].type = Block::type6;
            break;
        default:
            break;
        };
    }
}

void Game::figInit()
{
    // Background, character and infomation board images
    QString pix_path , style_dir;
    switch (style) {
    case 0:
        style_dir = "CatFishing";
        break;
    case 1:
        style_dir = "ChristmasEve";
        break;
    case 2:
        style_dir = "HeavenlyPuss";
        break;
    default:
        break;
    }
    pix_path = QString(":/res/%1/img/background.png").arg(style_dir);
    pix_background.load(pix_path);
    pix_background = pix_background.scaled(ScreenWidth,ScreenHeight,Qt::IgnoreAspectRatio);

    pix_path = QString(":/res/%1/img/Tom.png").arg(style_dir);
    pix_character_1.load(pix_path);
    pix_character_1 = pix_character_1.scaled(character_width,character_height,Qt::IgnoreAspectRatio);
    pix_path = QString(":/res/Game/img/infoBoard.png");
    pix_infoboard_1.load(pix_path);
    pix_infoboard_1 = pix_infoboard_1.scaled(character_width,ScreenHeight - character_height,Qt::IgnoreAspectRatio);
    if(!mode){
        pix_path = QString(":/res/%1/img/Jerry.png").arg(style_dir);
        pix_character_2.load(pix_path);
        pix_character_2 = pix_character_2.scaled(character_width,character_height,Qt::IgnoreAspectRatio);

        pix_path = QString(":/res/Game/img/infoBoard.png");
        pix_infoboard_2.load(pix_path);
        pix_infoboard_2 = pix_infoboard_2.scaled(character_width,ScreenHeight - character_height,Qt::IgnoreAspectRatio);
    }
    pix_path = QString(":/res/Game/img/back.PNG");
    pix_back.load(pix_path);
    pix_back = pix_back.scaled(ScreenHeight,ScreenHeight);

    // players
    pix_path = QString(":/res/Game/img/Tom.png");
    pix_player_0.load(pix_path);
    pix_player_0 = pix_player_0.scaled(step,step,Qt::IgnoreAspectRatio);
    pix_player_1.load(pix_path);
    pix_player_1 = pix_player_1.scaled(step,step,Qt::IgnoreAspectRatio);
    pix_path = QString(":/res/Game/img/Jerry.png");
    pix_player_2.load(pix_path);
    pix_player_2 = pix_player_2.scaled(step,step,Qt::IgnoreAspectRatio);

    // Blocks
    pix_path = QString(":/res/%1/img/type1.png").arg(style_dir);
    pix_type1.load(pix_path);
    pix_type1 = pix_type1.scaled(step,step,Qt::IgnoreAspectRatio);
    pix_path = QString(":/res/%1/img/type2.png").arg(style_dir);
    pix_type2.load(pix_path);
    pix_type2 = pix_type2.scaled(step,step,Qt::IgnoreAspectRatio);
    pix_path = QString(":/res/%1/img/type3.png").arg(style_dir);
    pix_type3.load(pix_path);
    pix_type3 = pix_type3.scaled(step,step,Qt::IgnoreAspectRatio);
    pix_path = QString(":/res/%1/img/type4.png").arg(style_dir);
    pix_type4.load(pix_path);
    pix_type4 = pix_type4.scaled(step,step,Qt::IgnoreAspectRatio);
    pix_path = QString(":/res/%1/img/type5.png").arg(style_dir);
    pix_type5.load(pix_path);
    pix_type5 = pix_type5.scaled(step,step,Qt::IgnoreAspectRatio);
    pix_path = QString(":/res/%1/img/type6.png").arg(style_dir);
    pix_type6.load(pix_path);
    pix_type6 = pix_type6.scaled(step,step,Qt::IgnoreAspectRatio);

    // Props
    pix_path = QString(":/res/prop/img/ExtraTime.PNG");
    pix_ExtraTime.load(pix_path);
    pix_ExtraTime = pix_ExtraTime.scaled(step,step,Qt::IgnoreAspectRatio);
    pix_path = QString(":/res/prop/img/Shuffle.png");
    pix_Shuffle.load(pix_path);
    pix_Shuffle = pix_Shuffle.scaled(step,step,Qt::IgnoreAspectRatio);
    pix_path = QString(":/res/prop/img/Flash.PNG");
    pix_Flash.load(pix_path);
    pix_Flash = pix_Flash.scaled(step,step,Qt::IgnoreAspectRatio);
    pix_path = QString(":/res/prop/img/Hint.png");
    pix_Hint.load(pix_path);
    pix_Hint = pix_Hint.scaled(step,step,Qt::IgnoreAspectRatio);
    pix_path = QString(":/res/prop/img/Freeze.png");
    pix_Freeze.load(pix_path);
    pix_Freeze = pix_Freeze.scaled(step,step,Qt::IgnoreAspectRatio);
    pix_path = QString(":/res/prop/img/Dizzy.PNG");
    pix_Dizzy.load(pix_path);
    pix_Dizzy = pix_Dizzy.scaled(step,step,Qt::IgnoreAspectRatio);

    // Undercoats
    pix_path = QString(":/res/Game/img/Highlighted.PNG");
    pix_Highlighted.load(pix_path);
    pix_Highlighted = pix_Highlighted.scaled(step,step);
    pix_path = QString(":/res/Game/img/TomSelected.PNG");
    pix_TomSelect.load(pix_path);
    pix_TomSelect = pix_TomSelect.scaled(step,step);
    pix_path = QString(":/res/Game/img/Normal.PNG");
    pix_Normal.load(pix_path);
    pix_Normal = pix_Normal.scaled(step,step);
    pix_path = QString(":/res/Game/img/JerrySelected.PNG");
    pix_JerrySelect.load(pix_path);
    pix_JerrySelect = pix_JerrySelect.scaled(step,step);
    pix_path = QString(":/res/prop/img/DizzyEffect.png");
    pix_DizzyEffect.load(pix_path);
    pix_DizzyEffect = pix_DizzyEffect.scaled(step,step,Qt::IgnoreAspectRatio);
    pix_path = QString(":/res/prop/img/FreezeEffect.png");
    pix_FreezeEffect.load(pix_path);
    pix_FreezeEffect = pix_FreezeEffect.scaled(step,step/2,Qt::IgnoreAspectRatio);
    pix_path = QString(":/res/prop/img/FlashEffect.png");
    pix_FlashEffect.load(pix_path);
    pix_FlashEffect = pix_FlashEffect.scaled(step,step,Qt::IgnoreAspectRatio);
}

// Determine whether the current map can be solved.
// No parameter.
// Return true when the map can be solve, otherwise false.
bool Game::isSolveable() const
{
    int* advised_path = getPath();
    if(advised_path[0] == -1){
        delete [] advised_path;
        return false;
    }
    delete [] advised_path;
    return true;
}

/* Find a pair of block that can be connected according to the rule.
 * No parameter.
 * Return an int array containing a possible path to connect the pair of block. Obviously, the first and the last number are the two blocks we found.
 */
int *Game::getPath() const
{
    int * path = new int[4]{-1,-1,-1,-1};
    int pp = 0;
    std::vector<Direction> dirct;
    for(int i = top * w + left; i <= bottom * w + right; ++i){
        // Traverse all the blocks and skip null and player
        if(getX(i) > right || getX(i) < left || map[i].state != Objects::block) continue;

        // decide searching direction
        if(map[move(i,W)].state != Objects::block)
            dirct.push_back(W);
        if(map[move(i,S)].state != Objects::block)
            dirct.push_back(S);
        if(map[move(i,A)].state != Objects::block)
            dirct.push_back(A);
        if(map[move(i,D)].state != Objects::block)
            dirct.push_back(D);

        // Search all stored directions
        for(std::vector<Direction>::iterator it = dirct.begin(); it != dirct.end(); ++it){
            // Search with 0 turn
            if(search_line(i,i,*it,path,pp)){
                path[pp++] = i;
                return path;
            }
            // Search with 1 turn
            if(search_1(i,i,*it,path,pp)){
                path[pp++] = i;
                return path;
            }
            // Search with 2 turns
            if(search_2(i,i,*it,path,pp)){
                path[pp++] = i;
                return path;
            }
        }
        dirct.clear();
    }
    return path;
}

/* Assistant function of getPath(). Search whether there is a pair can be connected with no turn.
 * Parameter:
 *  centre - the pos of block where we start our search (its type can be different);
 *  target - the pos of block we wish to eliminate (only use its type);
 *  dirct  - the direction we move on to search;
 *  path   - the int array that contain the pos of starting, turning and ending blocks (consist a completed path);
 *  pp     - the subscript of path where the next data going to fill;
 *  Return true if a pair can be connected with no turn is found, otherwise false.
 */
bool Game::search_line(const int &centre, const int &target, const Direction &dirct, int *&path, int &pp) const
{
    int i;
    for(i = move(centre,dirct);
         inBlockArea(i,dirct) && (map[i].state != Objects::block);
         i = move(i,dirct)){}
    if(map[i].state == Objects::block && map[i].type == map[target].type){
        path[pp++] = i;
        return true;
    }
    return false;
}

/* Assistant function of getPath(). Search whether there is a pair can be connected with one turn.
 * Parameter:
 *  centre - the pos of block where we start our search (its type can be different);
 *  target - the pos of block we wish to eliminate (only use its type);
 *  dirct  - the direction we move on to search;
 *  path   - the int array that contain the pos of starting, turning and ending blocks (consist a completed path);
 *  pp     - the subscript of path where the next data going to fill;
 *  Return true if a pair can be connected with one turn is found, otherwise false.
 */
bool Game::search_1(const int &centre, const int &target, const Direction &dirct, int *&path, int &pp) const
{
    int i;
    for(i = move(centre,dirct);
         inBlockArea(i,dirct) && (map[i].state != Objects::block);
         i = move(i,dirct)){
        if(search_line(i,target,rotate_cw(dirct),path,pp)){
            path[pp++] = i;
            return true;
        }
        if(search_line(i,target,rotate_ccw(dirct),path,pp)){
            path[pp++] = i;
            return true;
        }
    }
    return false;
}

/* Assistant function of getPath(). Search whether there is a pair can be connected with two turns.
 * Parameter:
 *  centre - the pos of block where we start our search (its type can be different);
 *  target - the pos of block we wish to eliminate (only use its type);
 *  dirct  - the direction we move on to search
 *  path   - the int array that contain the pos of starting, turning and ending blocks (consist a completed path);
 *  pp     - the subscript of path where the next data going to fill;
 *  Return true if a pair can be connected with two turns is found, otherwise false.
 */
bool Game::search_2(const int &centre, const int &target, const Direction &dirct, int *&path, int &pp) const
{
    int i;
    for(i = move(centre,dirct);
         i >= 0 && i < w * h && map[i].state != Objects::block;
         i = move(i,dirct)){
        if(search_1(i,target,rotate_cw(dirct),path,pp)){
            path[pp++] = i;
            return true;
        }
        if(search_1(i,target,rotate_ccw(dirct),path,pp)){
            path[pp++] = i;
            return true;
        }
        if(!inBlockArea(i,dirct)) break;
    }
    return false;
}

/* Shuffle the map. Only change among the objects with "Block" state. (That will not cause trouble to solvable judgement.)
 * No parameter.
 * No return.
*/
void Game::shuffle()
{
    // If the block is highlighted or selected, then erase the effect.
    if(hint_flag){
        map[hint1].isHighlighted = map[hint2].isHighlighted = false;
        hint1 = hint2 = -1;
    }

    if(mode){
        player_0->turnings = 0;
        if(player_0->selected != -1){
            map[player_0->selected].isSelected = false;
            player_0->selected = -1;
        }

    }
    else{
        player_1->turnings = 0;
        if(player_1->selected != -1){
            map[player_1->selected].isSelected_1 = false;
            player_1->selected = -1;
        }
        player_2->turnings = 0;
        if(player_2->selected != -1){
            map[player_2->selected].isSelected_2 = false;
            player_2->selected = -1;
        }
    }
    for(int i = w * top + left ,j_w , j_b ; i <= w * bottom + right ; ++i){
        if(getX(i) < left || getX(i) > right || map[i].state != Objects::block) continue;
        Objects::State tmp_state;
        Block::Type tmp_type;
        do{
            j_b = rand() % (col * row);
            j_w = j_b % col + left + (j_b / col + top) * w;
        }while(map[j_w].state != Objects::block);

        tmp_state = map[i].state;
        tmp_type = map[i].type;

        map[i].state = map[j_w].state;
        map[i].type = map[j_w].type;

        map[j_w].state = tmp_state;
        map[j_w].type = tmp_type;
    }

    // If "Hint" still takes effect, then highlight the blocks again.
    if(hint_flag){
        emit highlight();
    }
}

/* Randomly set a prop's position and type on a null object.
 * No parameter.
 * No return.
*/
void Game::generateProp()
{
    //position
    int loc;
    do{
        loc = rand() % (w * h);
    }while(map[loc].state != Objects::null || (inBlockArea(loc,W) && inBlockArea(loc,A)));

    //type
    int propType = rand() % prop_num;
    map[loc].state = Objects::prop;
    if(mode){
        switch (propType) {
        case 0:
            map[loc].kind = Prop::ExtraTime;
            break;
        case 1:
            map[loc].kind = Prop::Shuffle;
            break;
        case 2:
            map[loc].kind = Prop::Flash;
            break;
        case 3:
            map[loc].kind = Prop::Hint;
            break;
        default:
            break;
        }
    }
    else{
        switch (propType) {
        case 0:
            map[loc].kind = Prop::ExtraTime;
            break;
        case 1:
            map[loc].kind = Prop::Shuffle;
            break;
        case 2:
            map[loc].kind = Prop::Freeze;
            break;
        case 3:
            map[loc].kind = Prop::Hint;
            break;
        case 4:
            map[loc].kind = Prop::Dizzy;
            break;
        default:
            break;
        }
    }

}

/* Handle the effect of different props.
 *
 * Parameter:
 * target - the pos of the prop;
 * player - who gains the prop.
 *
 * No return.
*/
void Game::gainProp(int target , Player *&player)
{
    if(se)
        se_prop->play();
    switch (map[target].kind) {
    case Prop::ExtraTime:
        clock += 30;
        qDebug() << "add 30s!";
        break;
    case Prop::Shuffle:
        shuffle();
        qDebug() << "Shuffle!";
        break;
    case Prop::Flash:
        flash_flag = true;
        flash_clk += 5;
        qDebug() << "Flash!";
        break;
    case Prop::Hint:
        hint_clk += 10;
        if(!hint_flag){
            hint_flag = true;
            emit highlight();
        }
        qDebug() <<"Hint!";
        break;
    case Prop::Dizzy:
        player->dizzy_clk += 10;
        player->dizzy_flag = true;
        qDebug() << "Dizzy!";
        break;
    case Prop::Freeze:
        player->freeze_clk += 3;
        player->freeze_flag = true;
        qDebug() << "Freeze!";
    default:
        break;
    };
    return;
}

/* Record player's path in its member int array 'path'. Warn if the player takes more than two turns.
 * Parameters:
 * pl         - the pos of the player;
 * cur_direct - the current direction, which is used to detect whether the player takes a turn;
 * player     - who the function aimed at.
 * No return.
 */
void Game::recordPath(const int &pl, const Direction &cur_dirct, Player *&player)
{
    // The player takes a turn.
    if(turn(cur_dirct,player)){
        player->path[player->turnings++] = pl;
        player->last_dirct = cur_dirct;
    }

    // The player has taken more than 2 turns.
    if(player->turnings >= 4){
        player->warn_flag = true;
        QTimer::singleShot(1000,this,[=](){
            player->warn_flag = false;
        });
        if(mode)
            map[player->selected].isSelected = false;
        else{
            if(player == player_2)
                map[player->selected].isSelected_2 = false;
            else
                map[player->selected].isSelected_1 = false;
        }
        player->selected = -1;
        player->turnings = 0;
    }
}

/* The processing logic of the player accessing a block.
 * Parameters:
 * target - the pos of block the player accesses;
 * dirct  - in which direction the player access the block, which will be useful in path drawing;
 * player - who the function takes effect on.
 * No return.
*/
void Game::eliminate(const int &target, const Direction &dirct, Player *&player)
{
    // The player choose the first block.
    if(map[target].state == Objects::block && player->selected == -1){
        player->selected = target;
        if(mode)
            map[target].isSelected = true;
        else{
            if(player == player_2)
                map[target].isSelected_2 = true;
            else
                map[target].isSelected_1 = true;
        }
        player->last_dirct = dirct;
        player->path[player->turnings++] = target;
    }

    // The player choose the second block.
    else if(map[target].state == Objects::block && player->selected != -1){
        //can be eiliminated
        if((map[target].type == map[player->selected].type && target != player->selected)
            && (dirct != m || (dirct == m && getFlashPath(player->selected , target , player)))){
            //draw the path
            if(dirct != m){
                player->path[player->turnings++] = target;
            }
            player->drawPath_flag = true;
            draw_path_edge = player->turnings;
            for(int i = 0 ; i < draw_path_edge ; ++i){
                draw_path[i] = player->path[i];
            }
            player->turnings = 0;

            if(se)
                se_eliminate->play();

            map[target].state = map[player->selected].state = Objects::null;
            map[target].type = map[player->selected].type = Block::invalid;
            if(mode)
                map[target].isSelected = map[player->selected].isSelected = false;
            else{
                map[target].isSelected_2 = map[player->selected].isSelected_2 = false;
                map[target].isSelected_1 = map[player->selected].isSelected_1 = false;
            }
            if(target == hint1 || target == hint2 || player->selected == hint1 || player->selected == hint2){
                map[hint1].isHighlighted = false;
                map[hint2].isHighlighted = false;
                hint1 = hint2 = -1;
                if(hint_flag && isSolveable()){
                    emit highlight();
                }
            }
            cur_num -= 2;
            if(!mode){
                if(player->opponent->selected == player->selected)
                    player->opponent->selected = -1;
            }
            player->selected = -1;
            player->score += 10;

            if(!isSolveable()){
                game_flag = false;
                win();
            }

        }
        else{
            if(mode)
                map[target].isSelected = map[player->selected].isSelected = false;
            else{
                if(player == player_2)
                    map[target].isSelected_2 = map[player->selected].isSelected_2 = false;
                else
                    map[target].isSelected_1 = map[player->selected].isSelected_1 = false;
            }
            player->selected = -1;
            player->turnings = 0;
        }
    }
}

/* Find a pair of block which can be eliminated.
 * No parameter.
 * No return.
*/
void Game::hint()
{
    int* advised_path;
    advised_path = getPath();
    hint1 = advised_path[0];
    map[hint1].isHighlighted = true;
    for(hint2 = 1 ; hint2 < 4 ; ++hint2){
        if(advised_path[hint2] == -1) break;
    }
    hint2 = advised_path[hint2-1];
    map[hint2].isHighlighted = true;
    delete [] advised_path;
}

/* Find a possible path of the blocks selected by flashing player.
 * Parameters:
 * from - the pos of the starting block.
 * to   - the pos of the ending block.
 * player - who the function takes effect on.
 * Return true if the path exists, otherwise false.
*/
bool Game::getFlashPath(const int &from, const int &to , Player*& player)
{
    player->turnings = 0;

    //decide searching direction
    std::vector<Direction> dirct;
    if(map[move(from,W)].state != Objects::block)
        dirct.push_back(W);
    if(map[move(from,S)].state != Objects::block)
        dirct.push_back(S);
    if(map[move(from,A)].state != Objects::block)
        dirct.push_back(A);
    if(map[move(from,D)].state != Objects::block)
        dirct.push_back(D);

    for(std::vector<Direction>::iterator it = dirct.begin(); it != dirct.end(); ++it){
        //search with 0 turn
        if(flash_search_line(from , to , *it , player)){
            player->path[player->turnings++] = from;
            return true;
        }

        //search with 1 turn
        if(flash_search_1(from , to , *it , player)){
            player->path[player->turnings++] = from;
            return true;
        }

        //search with 2 turn
        if(flash_search_2(from , to , *it , player)){
            player->path[player->turnings++] = from;
            return true;
        }
    }
    return false;
}

/* Assistant function of getFlashPath. Judge whether the two blocks can be connected with no turn.
 * Parameters:
 * from - the pos of the starting block.
 * to   - the pos of the ending block.
 * dirct - the direction of searching.
 * player - who the function record path to.
 * Return true if the path is found, otherwise false.
*/
bool Game::flash_search_line(const int &from, const int &to, const Direction &dirct, Player*& player)
{
    int i;
    for(i = move(from , dirct);
         inBlockArea(i,dirct) && map[i].state != Objects::block && i != to;
         i = move(i,dirct)){}
    if(i == to){
        player->path[player->turnings++] = to;
        return true;
    }
    return false;
}

/* Assistant function of getFlashPath. Judge whether the two blocks can be connected with one turn.
 * Parameters:
 * from - the pos of the starting block.
 * to   - the pos of the ending block.
 * dirct - the direction of searching.
 * player - who the function record path to.
 * Return true if the path is found, otherwise false.
*/
bool Game::flash_search_1(const int &from, const int &to, const Direction &drt, Player*& player)
{
    const int turn1 = getY(from) * w + getX(to) , turn2 = getY(to) * w + getX(from) ,
        original_turnings = player->turnings;
    if(map[turn1].state != Objects::block
        && (flash_search_line(turn1,to,rotate_cw(drt),player) || flash_search_line(turn1,to,rotate_ccw(drt),player))
        && flash_search_line(from,turn1,drt,player))
        return true;
    else {
        player->turnings = original_turnings;
    }
    if(map[turn2].state != Objects::block
        && (flash_search_line(turn2,to,rotate_cw(drt),player) || flash_search_line(turn2,to,rotate_ccw(drt),player))
        && flash_search_line(from,turn2,drt,player))
        return true;
    else{
        player->turnings = original_turnings;
        return false;
    }
}

/* Assistant function of getFlashPath. Judge whether the two blocks can be connected with two turns.
 * Parameters:
 * from - the pos of the starting block.
 * to   - the pos of the ending block.
 * dirct - the direction of searching.
 * player - who the function record path to.
 * Return true if the path is found, otherwise false.
*/
bool Game::flash_search_2(const int &from, const int &to, const Direction &drt, Player*& player)
{
    for(int turn1 = move(from , drt) ; turn1 >= 0 && turn1 < w * h && map[turn1].state != Objects::block; turn1 = move(turn1 , drt)){
        if(flash_search_1(turn1 , to , rotate_cw(drt),player)){
            player->path[player->turnings++] = turn1;
            return true;
        }
        if(flash_search_1(turn1 , to , rotate_ccw(drt),player)){
            player->path[player->turnings++] = turn1;
            return true;
        }
        if(!inBlockArea(turn1,drt)) break;
    }
    return false;
}

/* The processing logic when the player fails the game. Stop the bgm and pass the score and failure result to resultDialog.
 * No parameter.
 * No return.
*/
void Game::fail()
{
    if(bgm)
        bgm_player->stop();
    int score_1 = -1 , score_2 = -1;
    if(mode)
        score_1 = player_0->score;
    else{
        score_1 = player_1->score;
        score_2 = player_2->score;
    }
    resultDialog->init(style,mode,false,score_1 , score_2);
    resultDialog->open();
}

/* The processing logic when the player wins the game. Stop the bgm and pass the score and success result to resultDialog.
 * No parameter.
 * No return.
*/
void Game::win()
{
    if(bgm)
        bgm_player->stop();
    int score_1 = -1 , score_2 = -1;
    if(mode)
        score_1 = player_0->score;
    else{
        score_1 = player_1->score;
        score_2 = player_2->score;
    }
    resultDialog->init(style,mode,true,score_1 , score_2);
    resultDialog->open();
}

/* Move the position toward specific direction. If is beyond boundary, the position will not move.
 * Parameters:
 * i  - the former position;
 * drct - the moving direction.
 * Return the pos after moving.
*/
int Game::move(int i, const Direction &drct) const
{
    switch (drct) {
    case W:
        if(i >= w)
            i -= w;
        break;
    case S:
        if(i < w * h - w)
            i += w;
        break;
    case A:
        if(i % w > 0)
            i -= 1;
        break;
    case D:
        if(i % w < w - 1)
            i += 1;
        break;
    default:
        break;
    }
    return i;
}

/* Judge whether the player takes a turn.
 * Parameters:
 * cur_dirct - the current direction the player moves on;
 * player    - who the funcition takes effect on.
 * Return true if the player takes a turn, otherwise false.
*/
bool Game::turn(const Direction &cur_dirct, Player *&player) const
{
    if(cur_dirct == W || cur_dirct == S){
        if(player->last_dirct == W || player->last_dirct == S)
            return false;
        else
            return true;
    }
    else{
        if(player->last_dirct == W || player->last_dirct == S)
            return true;
        else
            return false;
    }
}

/* Determine whether a player can flash to the pos.
 * Parameter: to - the pos of destination.
 * Return true if a player can flash to the pos, otherwise false.
*/
bool Game::canMoveTo(const int &to) const
{
    if(map[move(to,W)].state != Objects::block)
        return true;
    if(map[move(to,A)].state != Objects::block)
        return true;
    if(map[move(to,S)].state != Objects::block)
        return true;
    if(map[move(to,D)].state != Objects::block)
        return true;

    return false;
}

//Function "findWayOut" is of no use because open square only appears beside the blocks that can be eliminated.
// bool Game::findWayOut(const int &basis) const
// {
//     if(!inBlockArea(basis , W) || !inBlockArea(basis , A))
//         return true;
//     if(map[move(basis,W)].state != Objects::block && findWayOut(move(basis,W)))
//         return true;
//     if(map[move(basis,S)].state != Objects::block && findWayOut(move(basis,S)))
//         return true;
//     if(map[move(basis,A)].state != Objects::block && findWayOut(move(basis,A)))
//         return true;
//     if(map[move(basis,D)].state != Objects::block && findWayOut(move(basis,D)))
//         return true;
//     return false;
// }

/* Get the X coordinate of a position.
 * Parameter: i - the position.
 * Return the X coordinate of a position.
*/
int Game::getX(const int &i) const
{
    return i % w;
}

/* Get the Y coordinate of a position.
 * Parameter: i - the position.
 * Return the Y coordinate of a position.
*/
int Game::getY(const int &i) const
{
    return i / w;
}

/* Judge whether the pos is in block-area in specific direction (just two cases - vertically or horizontally).
 * Parameters:
 * i - the position;
 * dirct - the direction that is going be judged.
 * Return true if the pos is in block-area, otherwise false.
*/
bool Game::inBlockArea(int i, Direction dirct) const
{
    switch (dirct) {
    case W:
        return getY(i) >= top && getY(i) <= bottom;
    case S:
        return getY(i) >= top && getY(i) <= bottom;
    case A:
        return getX(i) >= left && getX(i) <= right;
    case D:
        return getX(i) >= left && getX(i) <= right;
    default:
        break;
    }
    return false;
}

/* Rotate the direction clockwise.
 * Parameter: dirct - the current direction.
 * Return the direction that has been rotated clockwise.
*/
Direction Game::rotate_cw(const Direction &dirct) const
{
    switch (dirct) {
    case W:
        return D;
    case S:
        return A;
    case D:
        return S;
    case A:
        return W;
    default:
        return m;
    }
}

/* Rotate the direction counterclockwise.
 * Parameter: dirct - the current direction.
 * Return the direction that has been rotated counterclockwise.
*/
Direction Game::rotate_ccw(const Direction &dirct) const
{
    switch (dirct) {
    case W:
        return A;
    case S:
        return D;
    case A:
        return S;
    case D:
        return W;
    default:
        return m;
    }
}
