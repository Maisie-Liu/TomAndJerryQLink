#ifndef PLAYER_H
#define PLAYER_H

#include <QWidget>
#include <QJsonObject>

enum Direction{W , S , A , D , m};

class Player
{
    Q_GADGET
public:
    Player();
    Player(const Player & player);
    Player& operator=(const Player & player);
    ~Player();
    static Player fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

    int loc = -1 , turnings = 0 , selected = -1 , freeze_clk = 0 , dizzy_clk = 0;
    bool freeze_flag = false , dizzy_flag = false ,drawPath_flag = false , warn_flag = false;
    int score = 0;
    Player * opponent = nullptr;
    int * path = new int[4];
    Direction last_dirct = m;


signals:

};

#endif // PLAYER_H
