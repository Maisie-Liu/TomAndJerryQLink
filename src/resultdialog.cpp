#include "resultdialog.h"
#include "ui_resultdialog.h"

extern bool bgm,se;

#include <QString>
#include <QPushButton>
#include <QKeyEvent>
#include <QPixmap>
#include <QIcon>
#include <QSoundEffect>
#include <QTimer>

ResultDialog::ResultDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ResultDialog)
{
    se_click = new QSoundEffect();
    se_click->setSource(QUrl::fromLocalFile(":/res/mainScene/click.wav"));
    se_click->setLoopCount(1);

    bgm_player = new QSoundEffect;

    ui->setupUi(this);
    QPixmap Icon(":/res/Game/img/Tom.png");
    setWindowIcon(QIcon(Icon));
    setAttribute(Qt::WA_DeleteOnClose);

    connect(ui->btn_back,&QPushButton::clicked,this,[=](){
        if(se){
            se_click->play();

        }
        QTimer::singleShot(1000,this,[=](){
          reject();
        });
    });
}

ResultDialog::~ResultDialog()
{
    delete se_click;
    if(bgm)
        bgm_player->stop();
    delete bgm_player;
    delete ui;
}

void ResultDialog::setMode(const bool &nmode)
{
    mode = nmode;
}

void ResultDialog::setWin(const bool &nwin)
{
    win = nwin;
}

void ResultDialog::init(const int &style, const bool &nmode, const bool &nwin, const int &nscore_1, const int &nscore_2)
{
    mode = nmode;
    win = nwin;
    score_1 = nscore_1;
    score_2 = nscore_2;
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
    bgm_player->setSource(QUrl::fromLocalFile(":/res/"+ style_dir + "/bgm/bgm_result.wav"));
    bgm_player->setLoopCount(QSoundEffect::Infinite);
    if(bgm)
        bgm_player->play();

    if(mode){
        //lbl_result shows whether you win.
        if(win)
            ui->lbl_result->setText(tr("恭喜你！游戏成功！"));
        else
            ui->lbl_result->setText(tr("真遗憾！游戏失败！"));

        //lbl_score shows your scores
        QString str = QString(tr("最终得分：%1")).arg(score_1);
        ui->lbl_score->setText(str);
    }
    else{
        //lbl_result shows who wins.
        if(score_1 == score_2)
            ui->lbl_result->setText(tr("平局！"));
        else{
            win = score_1 > score_2;
            if(win)
                ui->lbl_result->setText(tr("最终获胜：Tom"));
            else
                ui->lbl_result->setText(tr("最终获胜：Jerry"));
        }

        //lbl_score shows your scores
        QString str = QString(tr("Tom最终得分：%1 \nJerry最终得分：%2")).arg(score_1).arg(score_2);
        ui->lbl_score->setText(str);
    }
}

void ResultDialog::keyPressEvent(QKeyEvent *event)
{
    parent()->event((QEvent*)event);
}

// Handle the language-change event.
void ResultDialog::changeEvent(QEvent *event)
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
