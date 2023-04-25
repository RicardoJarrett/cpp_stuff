#ifndef _STAT_H_
#define _STAT_H_

#include "SDL.h"

struct Stat{
    enum statUpdState{
        sNULL=0,
        sUPGRADING=1,
        sFINISH_UPGRADE=2
    };
    enum StatType{
        stNULL=0,
        stDMG,
        stAUTOFIRE,
        stRANGE,
        stCOINDROP,
        stMAXHP,
        stHPREGEN,
        stSPAWNSPEED,
        stENEMYSPEED,
        stMAXENEMIES
    };
    int stat;
    int upgradeCost;
    double upgradeCostScale;
    int level;
    int type;

    int texID;
    SDL_Rect texRect;
    SDL_Texture* textTex;
    int btnID;
    SDL_Rect btnTexRect;
    int iconID;
    SDL_Rect iconRect;
    bool isUpgrading;
    double upgradeTimeScale;
    double upgradeTime;
    double upgradeTicker;
    double upgradeProgress;

    Stat();
    Stat(SDL_Rect _rect);
    Stat(int x, int y, int w, int h);
    void moveStat(SDL_Rect _pos);
    int update(double _dTime);
    bool upgrade(int _coins);
};

#endif // _STAT_H_
