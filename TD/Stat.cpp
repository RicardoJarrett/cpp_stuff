#include "Stat.h"

bool Stat::upgrade(int _coins){
    if(_coins >= upgradeCost){
        if(!isUpgrading){
            upgradeTicker = upgradeTime;
            isUpgrading = true;
            return true;
        }
    }
    return false;
}

int Stat::update(double _dTime){
    int _retState = Stat::statUpdState::sNULL;
    if(isUpgrading){
        upgradeTicker -= _dTime;
        if(upgradeTicker <= 0){
            upgradeTicker = 0;
            upgradeTime = upgradeTime * upgradeTimeScale;
            upgradeCost = upgradeCost * upgradeCostScale;
            level++;
            stat++;
            isUpgrading = false;
            _retState |= statUpdState::sFINISH_UPGRADE;
        }else{
            upgradeProgress = (upgradeTime - upgradeTicker) / upgradeTime;
            _retState |= Stat::statUpdState::sUPGRADING;
        }
    }
    return _retState;
}

Stat::Stat(){
    btnID = -1;
    upgradeCost = 10;
    upgradeCostScale = 1.3;
    upgradeTimeScale = 1.3;
    textTex = nullptr;
    iconID = -1;
    level = 1;
    stat = 1;
    upgradeProgress = 1.0;
    upgradeTicker = 0.0;
    upgradeTime = 500.0;
    upgradeTimeScale = 1.5;
    isUpgrading = false;

};
Stat::Stat(SDL_Rect _rect) : Stat(){
    moveStat(_rect);
}
Stat::Stat(int x, int y, int w, int h) : Stat((SDL_Rect){x, y, w, h}){
}

void Stat::moveStat(SDL_Rect _pos){
    btnTexRect = _pos;
    texRect = {btnTexRect.x + 16, btnTexRect.y, 60, 12};
    iconRect = {btnTexRect.x + 2, btnTexRect.y + 2, 12, 12};
}
