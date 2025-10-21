#ifndef ARCHIVE_H
#define ARCHIVE_H

#include <QWidget>
#include <QSoundEffect>

namespace Ui {
class Archive;
}

class Archive : public QWidget
{
    Q_OBJECT

public:
    explicit Archive(QWidget *parent = nullptr);
    ~Archive();

    void setText(const QString & str);
    void changeEvent(QEvent * event);

    QSoundEffect *se_click = nullptr;

signals:
    void loadIn();
    void remove();

private:
    Ui::Archive *ui;
};

#endif // ARCHIVE_H
