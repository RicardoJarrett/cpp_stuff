#ifndef _BASE_H_
#define _BASE_H_

#include <queue>
#include <vector>
#include <iostream>
#include "SDL.h"
#include "helper.h"
#include "Turret.h"

class Game;

struct Base{
    enum baseUpdState{
        usNULL=0,
        usHALF_SWAP=1,
        usSWAP_FINISH=2,
        usFINISH_UPGRADE=4,
        usUPGRADE=8
    };
    Game* owner;
    static std::queue<int> FreeIDs;
    SDL_Rect pos;
    int baseTexID;
    int turretBaseTexID;
    int coins;
    bool isSwapping;
    bool halfSwapPassed;
    int upgradingBays;
    int upgStat;
    Turret* swapT1;
    Turret* swapT2;
    int swapT1ID, swapT2ID;
    double swapTime;
    double swapTicker;
    double upgradeTime;
    double upgradeTicker;
    double currentUpgradeTime;

    //SDL_Rect turret_Rects[21];
    //Turret* turretBays[21];
    TurretBay* turretBays_2[21];

    Base(Game& _owner);
    Base();
    ~Base();
    void initTurretBases();
    int getFreeBase();
    int update(double _dTime, std::vector<SDL_Rect*>* _upgDrawRects);
    void setOwner(Game& _owner);
    void startSwap(int _bayID_1, int _bayID_2);
};

#endif // _BASE_H_
