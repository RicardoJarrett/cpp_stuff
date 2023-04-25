#ifndef _TURRET_H_
#define _TURRET_H_

#include <iostream>

#include "SDL.h"
#include "Enemy.h"
#include "Anim.h"
#include "Stat.h"
#include "Button.h"

struct Turret{
    enum turretType{
        tAutoGun=0,
        tRocket=1,
        tLaser=2
    };
    enum tStat{
        tsDMG,
        tsAUTOFIRE,
        tsRANGE
    };
    turretType type;
    double angle;
    double range;
    double autoFireDelay;
    double autoFireTicker;
    bool isSwapping;
    double swapScale;

    Enemy* target;
    int targetID;
    Anim* muzzleAnim;

    Stat* stats[3];
    Button* statButtons[3];
    bool buttonsActive;

    SDL_Rect baseRect;
    SDL_Rect cannonRect;
    SDL_Point pivotOffset;

    Turret(double _x, double _y, turretType _type, double _fireDelay=500.0);
    Turret();
    ~Turret();
    void setAngle(double _angle);
    void setTarget(Enemy* _target);
    void moveTurret(int _x, int _y);
    void setStatButton(int _buttonIndex, Button* _b);
    void activateButtons();
    void deactivateButtons();
};


struct TurretBay{
    enum tbUpgState{
        tbusNULL,
        tbusUPGRADE,
        tbusUPGRADE_FINISHED
    };
    Turret* turret;
    int ID;
    SDL_Rect pos;
    SDL_Rect turret_Rect;

    int baseTexID;
    int turretBaseTexID;

    bool isSwapping;
    bool isUpgrading;
    int upgStat;

    double upgradeProgress;

    TurretBay(int _ID);
    bool addTurret(Turret* _turret);
    void fSwapTurret(Turret* _turret);
    int update(double _dTime, std::vector<SDL_Rect*>* _upgDrawRects);
};

#endif // _TURRET_H_
