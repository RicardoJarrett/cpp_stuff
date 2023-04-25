#include "Base.h"

Base::Base(Game& _owner){
    owner = &_owner;
    swapTime = 2000.0;
    swapTicker = swapTime;
    halfSwapPassed = false;
    isSwapping = false;
    swapT1ID = -1;
    swapT2ID = -1;
    upgradeTime = 2000.0;
    upgradeTicker = 0;
    currentUpgradeTime = 0.0;
    upgradingBays = 0;
};

Base::Base(){
    owner = nullptr;
    swapTime = 2000.0;
    swapTicker = swapTime;
    halfSwapPassed = false;
    isSwapping = false;
    swapT1ID = -1;
    swapT2ID = -1;
    upgradeTime = 2000.0;
    upgradeTicker = 0;
    currentUpgradeTime = 0.0;
    upgradingBays = 0;
}

Base::~Base(){
    owner = nullptr;
}
void Base::setOwner(Game& _owner){
    owner = &_owner;
}

void Base::initTurretBases(){
    Base::FreeIDs = std::queue<int>();
    for(int i = 0; i < 21; i++){
        Base::FreeIDs.push(i);
        turretBays_2[i] = new TurretBay(i);
    }

    int turret_base_w = 64;
    int turret_base_h = 64;
    double _x = ((LEVEL_W - turret_base_w) / 2.0);
    double _y = ((LEVEL_H - turret_base_w) / 2.0);
    SDL_Rect turretRect = {int(_x), int(_y), turret_base_w, turret_base_h};
    turretBays_2[0]->pos = turretRect;
    turretBays_2[0]->turret_Rect = turretRect;

    int turret_base_radius_outer = (pos.w / 2.0) + 104;
    int turret_base_radius_inner = (pos.w / 2.0) + 48;
    double turret_angle = 11.25 * helper::_pi_over_180;
    int newTurretIndex = 1;
    for(int i = 0; i < 32; i++){
        bool newTurret = false;
        _x = ((LEVEL_W - turret_base_w) / 2.0);
        _y = ((LEVEL_H - turret_base_h) / 2.0);
        if(((i - 1) % 2) != 0){
            if((((i / 2) + 2 ) % 4) != 0){
                _x += turret_base_radius_outer * std::cos(i * turret_angle);
                _y += turret_base_radius_outer * std::sin(i * turret_angle);
                newTurret = true;
            }
        }else{
            if((((i / 2) % 4) == 0) || ((((i / 2) + 1) % 4) == 0)){
                _x += turret_base_radius_inner * std::cos(i * turret_angle);
                _y += turret_base_radius_inner * std::sin(i * turret_angle);
                newTurret = true;
            }
        }
        if(newTurret){
            turretRect = {int(_x), int(_y), turret_base_w, turret_base_h};
            int index = newTurretIndex++;
            turretBays_2[index]->turret_Rect = turretRect;
        }
    }
}

void Base::startSwap(int _bayID_1, int _bayID_2){
    if(!isSwapping){
        if(_bayID_1 != _bayID_2){
            if((_bayID_1 >= 0) && (_bayID_1 < 21)){
                if((_bayID_2 >= 0) && (_bayID_2 < 21)){
                    //start swap
                    isSwapping = true;
                    swapT1ID = _bayID_1;
                    swapT2ID = _bayID_2;
                    if(turretBays_2[_bayID_1]->turret != nullptr){
                        swapT1 = turretBays_2[_bayID_1]->turret;
                        swapT1->isSwapping = true;
                    }else{
                        swapT1 = nullptr;
                    }
                    if(turretBays_2[_bayID_2]->turret != nullptr){
                        swapT2 = turretBays_2[_bayID_2]->turret;
                        swapT2->isSwapping = true;
                    }else{
                        swapT2 = nullptr;
                    }
                }else{
                    std::cout << "Invalid bay ID: _bayID_2 " << _bayID_2 << "\n";
                }
            }else{
                std::cout << "Invalid bay ID: _bayID_1 " << _bayID_1 << "\n";
            }
        }else{
            std::cout << "Invalid: bay ID 1 = 2: " << _bayID_1 << " " << _bayID_2 << "\n";
        }
    }else{
        std::cout << "Still Swapping\n";
    }
}

int Base::update(double _dTime, std::vector<SDL_Rect*>* _upgProgressRects){
    int _retState = Base::baseUpdState::usNULL;
    if(isSwapping){
        if((swapT1 == nullptr) && (swapT2 == nullptr)){
            std::cout << "ERROR: 2 null pointers to swap\n";
            return -1;
        }
        swapTicker -= _dTime;
        if(swapTicker <= 0){
            isSwapping = false;
            if(swapT1 != nullptr){
                swapT1->isSwapping = false;
                swapT1->swapScale = 1.0;
            }
            if(swapT2 != nullptr){
                swapT2->isSwapping = false;
                swapT2->swapScale = 1.0;
            }
            swapT1 = nullptr;
            swapT2 = nullptr;
            swapT1ID = -1;
            swapT2ID = -1;
            halfSwapPassed = false;
            swapTicker = swapTime;
            _retState |= Base::baseUpdState::usSWAP_FINISH;
        }else{
            if(swapTicker > (swapTime / 2.0)){
                if(swapT1 != nullptr){
                    swapT1->swapScale = ((swapTicker - (swapTime / 2.0)) / (swapTime / 2.0));
                }
                if(swapT2 != nullptr){
                    swapT2->swapScale = ((swapTicker - (swapTime / 2.0)) / (swapTime / 2.0));
                }
            }else{
                if(!halfSwapPassed){
                    halfSwapPassed = true;
                    _retState |= baseUpdState::usHALF_SWAP;
                    if(turretBays_2[swapT1ID]->turret == nullptr){
                        turretBays_2[swapT1ID]->turret = turretBays_2[swapT2ID]->turret;
                        turretBays_2[swapT2ID]->turret = nullptr;
                    }else if(turretBays_2[swapT2ID]->turret == nullptr){
                        turretBays_2[swapT2ID]->turret = turretBays_2[swapT1ID]->turret;
                        turretBays_2[swapT1ID]->turret = nullptr;
                    }else{
                        Turret* _T1 = &(*(turretBays_2[swapT1ID]->turret));
                        turretBays_2[swapT1ID]->turret = turretBays_2[swapT2ID]->turret;
                        turretBays_2[swapT2ID]->turret = _T1;
                    }
                    int tSwapID = swapT1ID;
                    swapT1ID = swapT2ID;
                    swapT2ID = tSwapID;
                }
                if(swapT1 != nullptr){
                    swapT1->swapScale = (((swapTime / 2.0) - swapTicker) / (swapTime / 2.0));
                }
                if(swapT2 != nullptr){
                    swapT2->swapScale = (((swapTime / 2.0) - swapTicker) / (swapTime / 2.0));
                }
            }
        }
    }
    for(int i = 0; i < 21; i++){
        int bayState = turretBays_2[i]->update(_dTime, _upgProgressRects);
        if(bayState == TurretBay::tbUpgState::tbusUPGRADE){
            _retState |= Base::baseUpdState::usUPGRADE;
            //draw upgrade status
        }else if(bayState == TurretBay::tbUpgState::tbusUPGRADE_FINISHED){
            //
        }
    }
    return _retState;
}

std::queue<int> Base::FreeIDs;

int Base::getFreeBase(){
    int retID = -1;
    if(!Base::FreeIDs.empty()){
        retID = Base::FreeIDs.front();
        Base::FreeIDs.pop();
    }
    return retID;
}
