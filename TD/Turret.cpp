#include "Turret.h"

void Turret::activateButtons(){
    for(int i = 0; i < 3; i++){
        if(statButtons[i] != nullptr){
            statButtons[i]->active = true;
            buttonsActive = true;
        }
    }
}


void Turret::deactivateButtons(){
    for(int i = 0; i < 3; i++){
        if(statButtons[i] != nullptr){
            statButtons[i]->active = false;
            buttonsActive = false;
        }
    }
}

void Turret::setStatButton(int _buttonIndex, Button* _b){
    if((_buttonIndex >= 0) && (_buttonIndex < 3)){
        statButtons[_buttonIndex] = _b;
    }
}

void Turret::moveTurret(int _x, int _y){
    baseRect.x = _x;
    baseRect.y = _y;
    baseRect.w = 64;
    baseRect.h = 64;

    cannonRect = {baseRect.x + 20, baseRect.y - 20, 24, 64};
}

void Turret::setTarget(Enemy* _target){
    if((_target == nullptr) || (Enemy::checkID(_target->id) == false)){
        target = nullptr;
        targetID = -1;
    }else{
        target = _target;
        targetID = _target->id;
    }
}

void Turret::setAngle(double _angle){
    angle = _angle;
}

Turret::~Turret(){
    target = nullptr;
    muzzleAnim = nullptr;
    isSwapping = false;
    delete stats[0];
    delete stats[1];
    delete stats[2];
    stats[0] = nullptr;
    stats[1] = nullptr;
    stats[2] = nullptr;
    statButtons[0] = nullptr;
    statButtons[1] = nullptr;
    statButtons[2] = nullptr;
    buttonsActive = false;
}

Turret::Turret(double _x=0.0, double _y=0.0, turretType _type=Turret::turretType::tAutoGun, double _fireDelay) : type(_type), autoFireDelay(_fireDelay){
    baseRect.x = _x;
    baseRect.y = _y;
    baseRect.w = 64;
    baseRect.h = 64;

    cannonRect = {baseRect.x + 20, baseRect.y - 20, 24, 64};
    pivotOffset = {12, 52};
    autoFireTicker = 0.0;
    range = 220.0;
    angle = 0.0;
    target = nullptr;
    targetID = -1;
    muzzleAnim = nullptr;
    isSwapping = false;
    swapScale = 1.0;
    stats[0] = nullptr;
    stats[1] = nullptr;
    stats[2] = nullptr;
    statButtons[0] = nullptr;
    statButtons[1] = nullptr;
    statButtons[2] = nullptr;
}

TurretBay::TurretBay(int _ID) : ID(_ID){
    turret = nullptr;
    upgradeProgress = 1.0;
    isSwapping = false;
    isUpgrading = false;
}

bool TurretBay::addTurret(Turret* _turret){
    if(_turret != nullptr){
        if(turret == nullptr){
            turret = &(*_turret);
            return true;
        }else{
            std::cout << "ERROR: TurretBay Occupied: " << ID << "\n";
        }
    }else{
        std::cout << "ERROR: Passing Null Turret\n";
    }
    return false;
}

void TurretBay::fSwapTurret(Turret* _turret){
    if(_turret == nullptr){
        turret = nullptr;
    }else{
        turret = &(*_turret);
    }
}

int TurretBay::update(double _dTime, std::vector<SDL_Rect*>* _upgDrawRects){
    int retState = tbUpgState::tbusNULL;
    if(turret != nullptr){
        for(int i = 0; i < 3; i++){
            int statState = turret->stats[i]->update(_dTime);
            if(statState == Stat::statUpdState::sUPGRADING){
                isUpgrading = true;
                upgradeProgress = turret->stats[i]->upgradeProgress;

                SDL_Rect* progressRect = new SDL_Rect;
                *progressRect = pos;
                progressRect->y += (progressRect->h - 8);
                progressRect->w = int(double(progressRect->w) * upgradeProgress);
                progressRect->h = 16;
                _upgDrawRects->push_back(progressRect);

                retState |= tbUpgState::tbusUPGRADE;
            }else if(statState == Stat::statUpdState::sFINISH_UPGRADE){
                isUpgrading = false;
                upgradeProgress = 1.0;
                retState |= tbUpgState::tbusUPGRADE_FINISHED;
            }
        }
    }
    return retState;
}
