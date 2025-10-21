#ifndef PAUSEDIALOG_H
#define PAUSEDIALOG_H

#include <QDialog>
#include <QKeyEvent>
#include <QSoundEffect>

namespace Ui {
class PauseDialog;
}

class PauseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PauseDialog(QWidget *parent = nullptr);
    ~PauseDialog();
    bool saved = false;
    void changeEvent(QEvent * event);
    QSoundEffect *se_click = nullptr;

signals:
    void continuing();
    void saving();
    void back();
    void showArchiveDialog();
    void setting();
    void help();
private:
    Ui::PauseDialog *ui;
};

#endif // PAUSEDIALOG_H
