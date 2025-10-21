#include "mainscene.h"
#include "ui_mainscene.h"
#include "modechoosescene.h"
#include "archivedialog.h"
#include "settingdialog.h"

#include <QPushButton>
#include <QMessageBox>
#include <QStackedWidget>
#include <QTranslator>
#include <QSoundEffect>

bool bgm = false , se = false;

MainScene::MainScene(QWidget *parent, QTranslator *tslt)
    : QWidget(parent)
    , ui(new Ui::MainScene)
{
    ui->setupUi(this);

    // Window setting
    QPixmap Icon(":/res/Game/img/Tom.png");
    setWindowIcon(QIcon(Icon));

    // Init the click sound effect
    se_click = new QSoundEffect();
    se_click->setSource(QUrl::fromLocalFile(":/res/mainScene/click.wav"));
    se_click->setLoopCount(1);

    // Init bgm
    player = new QSoundEffect;;
    player->setSource(QUrl::fromLocalFile(":/res/mainScene/main.wav"));
    player->setLoopCount(QSoundEffect::Infinite);

    // Init settingDialog
    settingDialog = new SettingDialog();
    settingDialog->reject();

    // Init tanslator
    translator = tslt;

    // Init archiveDialog
    archiveDialog = new ArchiveDialog();
    archiveDialog->hide();

    // Init modeChooseScene
    modeChooseScene = new ModeChooseScene;
    modeChooseScene->hide();

    // Process signals
    connect(ui->btn_exit,&QPushButton::clicked,this,[=](){
        if(se)
            se_click->play();
        if(QMessageBox::Yes == QMessageBox::question(this,tr("猫和老鼠连连看"),tr("是否退出游戏？"))){
            this->close();
        }
    });

    connect(ui->btn_start,&QPushButton::clicked,this,[=](){
        if(se)
            se_click->play();
        modeChooseScene->setGeometry(this->geometry());
        modeChooseScene->init();
        this->hide();
        modeChooseScene->show();
    });

    connect(ui->btn_load,&QPushButton::clicked,this,[=](){
        if(se)
            se_click->play();
        archiveDialog->init();
        archiveDialog->exec();
    });

    connect(ui->btn_setting,&QPushButton::clicked,this,[=](){
        if(se)
            se_click->play();
        settingDialog->exec();
    });

    connect(archiveDialog,&QDialog::rejected,this,[=](){
        archiveDialog->hide();
    });

    connect(archiveDialog,&ArchiveDialog::loading,this,&MainScene::load);

    connect(modeChooseScene,&ModeChooseScene::back,this,[=](){
        this->setGeometry(modeChooseScene->geometry());
        modeChooseScene->hide();
        this->show();
    });

    connect(modeChooseScene,&ModeChooseScene::game_back,this,[=](){
        this->setGeometry(modeChooseScene->geo_game);
        this->show();
        if(bgm)
            player->play();
    });

    connect(modeChooseScene,&ModeChooseScene::showArchiveDialog,this,[=](){
        archiveDialog->init();
        archiveDialog->exec();
    });

    connect(modeChooseScene,&ModeChooseScene::showSettingDialog,this,[=](){
        settingDialog->exec();
    });

    connect(settingDialog,&SettingDialog::lset_en,this,[=](){
        if(language){
            if(translator != nullptr){
                qApp->removeTranslator(translator);
                delete translator;
                translator = nullptr;
            }
            translator = new QTranslator;
            if (translator->load(":/i18n/TomAndJerryQLink_en_US.qm")) {
                qApp->installTranslator(translator);
                ui->retranslateUi(this);
                language = false;
                qDebug() << "Changing language succeeded.";
            }
            else
                qDebug() << "Changing language failed.";
        }
    });

    connect(settingDialog,&SettingDialog::lset_zh,this,[=](){
        if(!language){
            if(translator != nullptr){
                qApp->removeTranslator(translator);
                delete translator;
                translator = nullptr;
            }
            translator = new QTranslator;
            if (translator->load(":/i18n/TomAndJerryQLink_zh_CN.qm")) {
                qApp->installTranslator(translator);
                ui->retranslateUi(this);
                language = true;
                qDebug() << "Changing language succeeded.";
            }
            else
                qDebug() << "Changing language failed.";
        }
    });

    connect(settingDialog,&SettingDialog::playBGM,this,[=](){
        if(modeChooseScene->game == nullptr){
            player->play();
        }
        else
            modeChooseScene->game->bgm_player->play();
    });

    connect(settingDialog,&SettingDialog::stopBGM,this,[=](){
        if(modeChooseScene->game == nullptr)
            player->stop();
        else
            modeChooseScene->game->bgm_player->stop();
    });

    connect(modeChooseScene,&ModeChooseScene::main_bgm_stop,player,&QSoundEffect::stop);
}

MainScene::~MainScene()
{
    delete ui;
    delete translator;
    delete se_click;
    if(bgm)
        player->stop();
    delete player;
    delete settingDialog;
    delete archiveDialog;
    if(modeChooseScene!= nullptr)
        delete modeChooseScene;
}

/* load game.
 * Parameter: i - the sequential number of the archive file in the dictionary.
 * No return.
*/
void MainScene::load(int i)
{

    if(modeChooseScene->game != nullptr){
        modeChooseScene->game->load(i);
        return;
    }

    if(bgm)
        player->stop();
    modeChooseScene->game = new Game(i);
    modeChooseScene->game->setGeometry(this->geometry());
    this->hide();
    modeChooseScene->game->show();

    connect(modeChooseScene->game,&Game::backMain,this,[=](){
        this->setGeometry(modeChooseScene->game->geometry());
        delete modeChooseScene->game;
        modeChooseScene->game = nullptr;
        if(bgm)
            player->play();
        this->show();
    });
    connect(modeChooseScene->game,&Game::showArchiveDialog,this,[=](){
        archiveDialog->init();
        archiveDialog->exec();
    });
    connect(modeChooseScene->game,&Game::showSettingDialog,this,[=](){
        settingDialog->exec();
    });
}


