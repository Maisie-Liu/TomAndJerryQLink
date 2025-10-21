#include "pausedialog.h"
#include "ui_pausedialog.h"
// #include "archivedialog.h"
// #include "mainscene.h"

extern bool bgm,se;

#include <QMessageBox>
#include <QPixmap>
#include <QIcon>
#include <QSoundEffect>
#include <QTimer>

PauseDialog::PauseDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PauseDialog)
{
    se_click = new QSoundEffect();
    se_click->setSource(QUrl::fromLocalFile(":/res/mainScene/click.wav"));
    se_click->setLoopCount(1);

    ui->setupUi(this);

    setFixedSize(400,350);

    // setWindowTitle("猫和老鼠连连看 游戏暂停");
    QPixmap Icon(":/res/Game/img/Tom.png");
    setWindowIcon(QIcon(Icon));
    connect(ui->btn_continue,&QPushButton::clicked,this,[=](){
        if(se)
            se_click->play();
        emit continuing();
        saved = false;
        this->hide();
    });

    connect(ui->btn_save,&QPushButton::clicked,this,[=](){
        if(se)
            se_click->play();
        emit saving();
        saved = true;
    });

    connect(ui->btn_setting,&QPushButton::clicked,this,[=](){
        emit setting();
        if(se)
            se_click->play();
    });

    connect(ui->btn_load,&QPushButton::clicked,this,[=](){
        if(se)
            se_click->play();
        emit showArchiveDialog();
    });

    connect(ui->btn_back,&QPushButton::clicked,this,[=](){
        if(se)
            se_click->play();
        if(!saved){
            if(QMessageBox::Yes == QMessageBox::question(this,tr("猫和老鼠连连看"),tr("游戏尚未保存，是否返回主菜单？"))){
                emit back();
            }
        }
        else{
            QTimer::singleShot(1000,this,[=](){
                emit back();
            });

        }
    });

    connect(ui->btn_help,&QPushButton::clicked,this,&PauseDialog::help);
}

PauseDialog::~PauseDialog()
{
    delete se_click;
    delete ui;
}

// Handle the language-change event.
void PauseDialog::changeEvent(QEvent *event)
{
    QDialog::changeEvent(event);
    switch(event->type()){
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}


