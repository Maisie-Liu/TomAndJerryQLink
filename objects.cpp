#include "objects.h"

// Copy constructor.
Objects::Objects(const Objects &ob)
{
    type = ob.type;
    kind = ob.kind;
    pos = ob.pos;
    state = ob.state;
    isHighlighted = ob.isHighlighted;
    isSelected = ob.isSelected;
    isSelected_1 = ob.isSelected_1;
    isSelected_2 = ob.isSelected_2;
}

// Default constructor
Objects::Objects() = default;

// Overload the = operator
Objects &Objects::operator=(const Objects &ob)
{
    type = ob.type;
    kind = ob.kind;
    pos = ob.pos;
    state = ob.state;
    isHighlighted = ob.isHighlighted;
    isSelected = ob.isSelected;
    isSelected_1 = ob.isSelected_1;
    isSelected_2 = ob.isSelected_2;
    return *this;
}

/* Load from a QjsonObject.
 * Parameter: json - the QJsonObject that will be loaded.
 * Return a Object after loading.
 */
Objects Objects::fromJson(const QJsonObject &json)
{
    Objects result;
    if(const QJsonValue v = json["type"]; v.isDouble())
        result.type = Block::Type(v.toInt());

    if(const QJsonValue v = json["kind"]; v.isDouble())
        result.kind = Prop::Kind(v.toInt());

    if(const QJsonValue v = json["pos_x"]; v.isDouble())
        result.pos.setX(v.toInt());

    if(const QJsonValue v = json["pos_y"]; v.isDouble())
        result.pos.setY(v.toInt());

    if(const QJsonValue v = json["state"]; v.isDouble())
        result.state = State(v.toInt());

    if(const QJsonValue v = json["isHighlighted"]; v.isBool())
        result.isHighlighted = v.toBool();

    if(const QJsonValue v = json["isSelected"]; v.isBool())
        result.isSelected = v.toBool();

    if(const QJsonValue v = json["isSelected_1"]; v.isBool())
        result.isSelected_1 = v.toBool();

    if(const QJsonValue v = json["isSelected_2"]; v.isBool())
        result.isSelected_2 = v.toBool();

    return result;
}

/* Write to a QjsonObject.
 * No parameter.
 * Return a QjsonObject wrote to.
 */
QJsonObject Objects::toJson() const
{
    QJsonObject json;
    json["type"] = type;
    json["kind"] = kind;
    json["pos_x"] = pos.x();
    json["pos_y"] = pos.y();
    json["state"] = state;
    json["isHighlighted"] = isHighlighted;
    json["isSelected"] = isSelected;
    json["isSelected_1"] = isSelected_1;
    json["isSelected_2"] = isSelected_2;
    return json;
}
