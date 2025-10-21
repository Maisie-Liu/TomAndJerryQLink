#ifndef GAMEHELP_H
#define GAMEHELP_H

#include <QDialog>

namespace Ui {
class GameHelp;
}

class GameHelp : public QDialog
{
    Q_OBJECT

public:
    explicit GameHelp(QWidget *parent = nullptr);
    ~GameHelp();
    void changeEvent(QEvent* event);
private:
    Ui::GameHelp *ui;
};

#endif // GAMEHELP_H
