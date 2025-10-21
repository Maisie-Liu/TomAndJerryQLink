#include "archivedialog.h"
#include "archive.h"
#include "qevent.h"
#include "ui_archivedialog.h"

extern bool bgm,se;

#include <QPushButton>
#include <QDir>
#include <QFile>
#include <QFont>
#include <QStandardPaths>
#include <QPixmap>
#include <QIcon>
#include <QEvent>
#include <QMessageBox>
#include <QSoundEffect>

ArchiveDialog::ArchiveDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ArchiveDialog)
{
    // Click sound effect init.
    se_click = new QSoundEffect();
    se_click->setSource(QUrl::fromLocalFile(":/res/mainScene/click.wav"));
    se_click->setLoopCount(1);

    // Window setting.
    QPixmap Icon(":/res/Game/img/Tom.png");
    setWindowIcon(QIcon(Icon));
    ui->setupUi(this);
    ui->scrollArea->setWidgetResizable(true);

    // Process the click signal of back button.
    connect(ui->btn_back,&QPushButton::clicked,this,[=](){
        if(se)
            se_click->play();
        this->reject();
    });
}

ArchiveDialog::~ArchiveDialog()
{
    if(archives != nullptr){
        for(int i = 0 ; i < nameList.count() ; ++i){
            ui->verticalLayout_scrollArea->removeWidget(&(archives[i]));
        }
        delete [] archives;
        archives = nullptr;
    }
    delete ui;
}

/* Init the archives or no-archive label.
 * No parameter. No return.
*/
void ArchiveDialog::init()
{
    QString archiveDirPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/archive/";
    QDir * folder = new QDir(archiveDirPath);

    if(!folder->exists() || folder->isEmpty()){
        if(archives != nullptr){
            for(int i = 0 ; i < nameList.count() ; ++i){
                ui->verticalLayout_scrollArea->removeWidget(&(archives[i]));
            }
            delete [] archives;
            archives = nullptr;
        }
        if(lbl_null == nullptr){
            QFont font;
            font.setFamily("楷体");
            font.setPointSize(18);
            lbl_null = new QLabel ();
            lbl_null->setFixedHeight(100);
            lbl_null->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
            lbl_null->setFont(font);
            lbl_null->setStyleSheet("color: rgb(0,0,0);");
            lbl_null->setAlignment(Qt::AlignCenter);
            lbl_null->setText(tr("暂无存档！"));
            ui->verticalLayout_scrollArea->addWidget(lbl_null);
            delete folder;
        }
    }
    else{
        if(lbl_null != nullptr){
            ui->verticalLayout_scrollArea->removeWidget(lbl_null);
            delete lbl_null;
            lbl_null = nullptr;
        }
        QStringList nnameList = folder->entryList(QDir::Files | QDir::NoDotAndDotDot,QDir::Time);
        delete folder;
        if(nnameList == nameList)
            return;
        if(archives != nullptr){
            for(int i = 0 ; i < nameList.count() ; ++i){
                ui->verticalLayout_scrollArea->removeWidget(&(archives[i]));
            }
            delete [] archives;
            archives = nullptr;
        }
        nameList = nnameList;
        archives = new Archive[nameList.count()];
        for(int i = 0 ; i < nameList.count() ; ++i){
            QString text = nameList.at(i)  ,theme , mode , year , month , day , hour,min,sec;
            text.remove(".json");
            theme = text.section('_',0,0);
            mode = text.section('_',1,1);
            year = text.section('_',2,2);
            month = text.section('_',3,3);
            day = text.section('_',4,4);
            hour = text.section('_',5,5);
            min = text.section('_',6,6);
            sec = text.section('_',7,7);
            if(theme == "CatFishing")
                theme = tr("小猫钓鱼");
            else if(theme == "ChristmasEve")
                theme = tr("圣诞前夜");
            else if(theme == "HeavenlyPuss")
                theme = tr("天堂之旅");

            if(mode == "single")
                mode = tr("单人模式");
            else if(mode == "double")
                mode = tr("双人模式");

            archives[i].setText(theme + " " + mode +"\n"
                                + year + "-" + month + "-" +day + " "
                                + hour + ":" + min + ":" + sec);
            ui->verticalLayout_scrollArea->addWidget(&(archives[i]));
            connect(&archives[i],&Archive::loadIn,this,[=](){
                emit loading(i);
                this->accept();
            });

            connect(&archives[i],&Archive::remove,this,[=](){
                if(QMessageBox::Yes == QMessageBox::question(this,tr("猫和老鼠连连看"),tr("是否删除存档？"))){
                    if (QFile::remove(archiveDirPath + nameList.at(i))) {
                        qDebug() <<archiveDirPath + nameList.at(i);
                        qDebug() << "Remove successfully!";
                    }
                    else {
                        qDebug() << "Remove failed.";
                    }
                    init();
                }
            });
        }
    }
}

// Handle the language-change event.
void ArchiveDialog::changeEvent(QEvent *event)
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
