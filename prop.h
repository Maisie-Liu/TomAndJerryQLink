#ifndef PROP_H
#define PROP_H

#include <QWidget>

class Prop : public QWidget
{
    Q_OBJECT
public:
    explicit Prop(QWidget *parent = nullptr);
    enum Kind{
        ExtraTime,
        Shuffle,
        Hint,
        Flash,
        Freeze,
        Dizzy,
        invalid
    };

signals:
};

#endif // PROP_H
