#ifndef ARCHIVEDIALOG_H
#define ARCHIVEDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QLayout>
#include <QLabel>
#include <QEvent>
#include <QSoundEffect>

#include "archive.h"

namespace Ui {
class ArchiveDialog;
}

class ArchiveDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ArchiveDialog(QWidget *parent = nullptr);
    Archive * archives = nullptr;
    QLabel * lbl_null = nullptr;
    QStringList nameList;
    QSoundEffect *se_click = nullptr;
    ~ArchiveDialog();
    void init();
    void changeEvent(QEvent * event);

signals:
    void loading(int i);
    void back();
    void renew();

private:
    Ui::ArchiveDialog *ui;
};

#endif // ARCHIVEDIALOG_H
