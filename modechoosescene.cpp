#include "modechoosescene.h"
#include "ui_modechoosescene.h"

extern bool bgm,se;

#include <QDebug>
#include <QSoundEffect>

ModeChooseScene::ModeChooseScene(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ModeChooseScene)
{
    se_click = new QSoundEffect();
    se_click->setSource(QUrl::fromLocalFile(":/res/mainScene/click.wav"));
    se_click->setLoopCount(1);

    ui->setupUi(this);

    QPixmap Icon(":/res/Game/img/Tom.png");
    setWindowIcon(QIcon(Icon));

    ui->stkW_styleChoose->setCurrentIndex(0);

    connect(ui->btn_p1_after,&QPushButton::clicked,this,[=](){
        if(se)
            se_click->play();
        ui->stkW_styleChoose->setCurrentIndex(1);
    });

    connect(ui->btn_p2_after,&QPushButton::clicked,this,[=](){
        if(se)
            se_click->play();
        ui->stkW_styleChoose->setCurrentIndex(2);
    });

    connect(ui->btn_p3_forward,&QPushButton::clicked,this,[=](){
        if(se)
            se_click->play();
        ui->stkW_styleChoose->setCurrentIndex(1);
    });

    connect(ui->btn_p2_forward,&QPushButton::clicked,this,[=](){
        if(se)
            se_click->play();
        ui->stkW_styleChoose->setCurrentIndex(0);
    });

    connect(ui->btn_back,&QPushButton::clicked,this,[=](){
        if(se)
            se_click->play();
        emit back();
    });

    connect(ui->btn_confirm,&QPushButton::clicked,this,[=](){
        if(se)
            se_click->play();
        if(bgm)
            emit main_bgm_stop();
        game = new Game(ui->rBtn_single->isChecked(),ui->cb_row->currentIndex() * 2 + 4,
                        ui->cb_col->currentIndex() * 2 + 4,ui->stkW_styleChoose->currentIndex());
        game->setGeometry(this->geometry());
        this->hide();
        game->show();
        connect(game,&Game::showArchiveDialog,this,&ModeChooseScene::showArchiveDialog);
        connect(game,&Game::showSettingDialog,this,&ModeChooseScene::showSettingDialog);
        connect(game,&Game::backMain,this,[=](){
            geo_game = game->geometry();
            delete game;
            game = nullptr;
            emit game_back();
        });
    });

}

ModeChooseScene::~ModeChooseScene()
{
    delete se_click;
    delete ui;
}

/* Make sure that every time user open the modeChooseScene, it will be in the initial state.
 * No parameter. No return.
*/
void ModeChooseScene::init()
{
    //col and row: default 4
    ui->cb_row->setCurrentIndex(0);
    ui->cb_col->setCurrentIndex(0);

    //default: single player
    ui->rBtn_single->setChecked(true);
    ui->rBtn_two->setChecked(false);

    ui->stkW_styleChoose->setCurrentIndex(0);
}

// Handle the language-change event.
void ModeChooseScene::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    switch(event->type()){
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
