#ifndef RESULTDIALOG_H
#define RESULTDIALOG_H

#include <QDialog>
#include <QKeyEvent>
#include <QSoundEffect>
#include <QMediaPlayer>

namespace Ui {
class ResultDialog;
}

class ResultDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ResultDialog(QWidget *parent = nullptr);
    ~ResultDialog();
    bool win  = false;
    bool mode = false;
    int score_1 = -1 ,  score_2 = -1;
    void setMode(const bool & nmode);
    void setWin(const bool & nwin);
    void setScore(const int & nscore_1 , const int & nscore_2);
    void init(const int & style,const bool & nmode, const bool & nwin,const int & nscore_1 , const int & nscore_2);
    void keyPressEvent(QKeyEvent *event);
    void changeEvent(QEvent * event);
    QSoundEffect *se_click = nullptr ,* bgm_player = nullptr;

signals:
    void back();

private:
    Ui::ResultDialog *ui;
};

#endif // RESULTDIALOG_H
