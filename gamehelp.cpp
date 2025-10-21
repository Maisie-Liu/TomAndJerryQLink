#include "gamehelp.h"
#include "ui_gamehelp.h"

GameHelp::GameHelp(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::GameHelp)
{
    ui->setupUi(this);
    this->setFixedSize(560,560);
    ui->stackedWidget->setCurrentIndex(0);
    connect(ui->btn_after,&QPushButton::clicked,this,[=](){
        if(ui->stackedWidget->currentIndex() == 2)
            ui->stackedWidget->setCurrentIndex(0);
        else
            ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex() + 1);
    });

    connect(ui->btn_forward,&QPushButton::clicked,this,[=](){
        if(ui->stackedWidget->currentIndex() == 0)
            ui->stackedWidget->setCurrentIndex(2);
        else
            ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex() - 1);
    });

    connect(ui->btn_back,&QPushButton::clicked,this,&QDialog::reject);
}

GameHelp::~GameHelp()
{
    delete ui;
}

// Handle the language-change event.
void GameHelp::changeEvent(QEvent *event)
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
