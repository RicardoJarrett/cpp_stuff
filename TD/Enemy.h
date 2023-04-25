#ifndef _ENEMY_H_
#define _ENEMY_H_

#include <queue>
#include "helper.h"
#include "Anim.h"
#include <cmath>

extern int LEVEL_W;
extern int LEVEL_H;

class Enemy{
public:
    enum Types{
        tRegular,
        tFast,
        tHeavy,
        tBoss
    };
    double x, y, w, h, spd;
    int hp, maxHp, id, coinDrop, dmg;
    static std::queue<int> ids;
    Types type;
    bool attackState;
    bool isAttacking;
    double atkTicker;
    double atkDelay;
    double aspd;
    double walkingAngleR;
    helper::OctDir facingDir;
    SpriteAnimInst* animInst;

    static int getID();
    static void clearID(int _id);
    static bool checkID(int _id);
    static void initEnemies(int _maxEnemies);
    void update(double dTime, int _spdMult);
    bool takeDamage(int _dmg);
    bool dying;
};

#endif // _ENEMY_H_
