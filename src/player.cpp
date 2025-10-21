#include "player.h"
#include <QJsonArray>

Player::Player()
{}

Player::Player(const Player &player)
{
    loc = player.loc;
    turnings = player.turnings;
    selected = player.selected;
    freeze_clk = player.freeze_clk;
    dizzy_clk = player.dizzy_clk;
    freeze_flag = player.freeze_flag;
    dizzy_flag = player.dizzy_flag;
    drawPath_flag = player.drawPath_flag;
    score = player.score;
    last_dirct = player.last_dirct;
    opponent = player.opponent;
    for(int i = 0 ; i < 4 ; ++i){
        path[i] = player.path[i];
    }
}

Player& Player::operator=(const Player &player)
{
    loc = player.loc;
    turnings = player.turnings;
    selected = player.selected;
    freeze_clk = player.freeze_clk;
    dizzy_clk = player.dizzy_clk;
    freeze_flag = player.freeze_flag;
    dizzy_flag = player.dizzy_flag;
    drawPath_flag = player.drawPath_flag;
    score = player.score;
    last_dirct = player.last_dirct;
    opponent = player.opponent;
    for(int i = 0 ; i < 4 ; ++i){
        path[i] = player.path[i];
    }
    return *this;
}

Player::~Player()
{
    if(path != nullptr)
        delete [] path;
}

/* Load from a QjsonObject.
 * Parameter: json - the QJsonObject that will be loaded.
 * Return a Object after loading.
 */
Player Player::fromJson(const QJsonObject &json)
{
    Player result;
    if(const QJsonValue v = json["loc"]; v.isDouble())
        result.loc = v.toInt();
    if(const QJsonValue v = json["turings"]; v.isDouble())
        result.turnings = v.toInt();
    if(const QJsonValue v = json["selected"]; v.isDouble())
        result.selected = v.toInt();
    if(const QJsonValue v = json["freeze_clk"]; v.isDouble())
        result.freeze_clk = v.toInt();
    if(const QJsonValue v = json["dizzy_clk"]; v.isDouble())
        result.dizzy_clk = v.toInt();
    if(const QJsonValue v = json["freeze_flag"]; v.isBool())
        result.freeze_flag = v.toBool();
    if(const QJsonValue v = json["dizzy_flag"]; v.isBool())
        result.dizzy_flag = v.toBool();
    if(const QJsonValue v = json["drawPath_flag"]; v.isBool())
        result.drawPath_flag = v.toBool();
    if(const QJsonValue v = json["score"]; v.isDouble())
        result.score = v.toInt();
    if(const QJsonValue v = json["last_direction"]; v.isDouble())
        result.last_dirct = Direction(v.toInt());

    return result;
}


/* Write to a QjsonObject.
 * No parameter.
 * Return a QjsonObject wrote to.
 */
QJsonObject Player::toJson() const
{
    QJsonObject json;
    json["loc"] = loc;
    json["turings"] = turnings;
    json["selected"] = selected;
    json["freeze_clk"] = freeze_clk;
    json["dizzy_clk"] = dizzy_clk;
    json["freeze_flag"] = freeze_flag;
    json["dizzy_flag"] = dizzy_flag;
    json["drawPath_flag"] = drawPath_flag;
    json["score"] = score;
    json["last_direction"] = last_dirct;
    return json;
}

