#include "settingdialog.h"
#include "ui_settingdialog.h"
// #include "mainscene.h"

extern bool bgm,se;

#include <QDebug>
#include <QSoundEffect>

SettingDialog::SettingDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingDialog)
{
    se_click = new QSoundEffect();
    se_click->setSource(QUrl::fromLocalFile(":/res/mainScene/click.wav"));
    se_click->setLoopCount(1);

    ui->setupUi(this);
    connect(ui->btn_back,&QPushButton::clicked,this,[=](){
        if(se)
            se_click->play();
        reject();
    });
    connect(ui->rBtn_zh,&QAbstractButton::clicked,this,[=](){
        if(se)
            se_click->play();
        if(ui->rBtn_zh->isChecked())
            emit lset_zh();
    });
    connect(ui->rBtn_en,&QAbstractButton::clicked,this,[=](){
        if(se)
            se_click->play();
        if(ui->rBtn_en->isChecked())
            emit lset_en();
    });
    connect(ui->rBtn_effect,&QAbstractButton::clicked,this,[=](){
        if(se)
            se_click->play();
        se = ui->rBtn_effect->isChecked();
    });
    connect(ui->rBtn_bgm,&QAbstractButton::clicked,this,[=](){
        if(se)
            se_click->play();
        bgm = ui->rBtn_bgm->isChecked();
        if(bgm){
            emit playBGM();
        }
        else
            emit stopBGM();
    });
}

SettingDialog::~SettingDialog()
{
    delete se_click;
    delete ui;
}

// Handle the language-change event.
void SettingDialog::changeEvent(QEvent *event)
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

// Set the checked-state of "简体中文" button to true.
// No parameter. No return.
void SettingDialog::btnSet_zh()
{
    ui->rBtn_zh->setChecked(true);
}

// Set the checked-state of "English" button to true.
// No parameter. No return.
void SettingDialog::btnSet_en()
{
    ui->rBtn_en->setChecked(true);
}
