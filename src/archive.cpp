#include "archive.h"
#include "ui_archive.h"

extern bool se;

Archive::Archive(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Archive)
{
    // Click sound effect init.
    se_click = new QSoundEffect();
    se_click->setSource(QUrl::fromLocalFile(":/res/mainScene/click.wav"));
    se_click->setLoopCount(1);

    ui->setupUi(this);
    connect(ui->btn_loadIn,&QPushButton::clicked,this,[=](){
        emit loadIn();
        if(se)
            se_click->play();
    });
    connect(ui->btn_remove,&QPushButton::clicked,this,[=](){
        emit remove();
        if(se)
            se_click->play();
    });
}

Archive::~Archive()
{
    delete ui;
}

// Set the text on the archive block to str.
void Archive::setText(const QString &str)
{
    ui->lbl_info->setText(str);
}

// Handle the language-change event.
void Archive::changeEvent(QEvent *event)
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
