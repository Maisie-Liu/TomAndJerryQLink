#ifndef OBJECTS_H
#define OBJECTS_H

#include <QWidget>
#include <QPoint>
#include <QSize>
#include <QJsonObject>
//#include <QLabel>

#include "block.h"
#include "prop.h"

class Objects
{
    Q_GADGET
public:
    Objects();
    Objects(const Objects &ob);
    Objects& operator=(const Objects & ob);
    static Objects fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

    enum State {
        null,
        player,
        block,
        prop
    };

    Block::Type type = Block::invalid;
    Prop::Kind kind = Prop::invalid;

    QPoint pos = QPoint(0,0);
    State state = null;
    const QSize size = QSize(40,40);

    bool isHighlighted = false , isSelected = false ,isSelected_1 = false ,isSelected_2 = false  ;


signals:
};

#endif // OBJECTS_H
