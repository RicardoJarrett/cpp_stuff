#define SDL_MAIN_HANDLED
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include <iostream>
#include <fstream>
#include <Windows.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <ctime>
#include <cmath>
#include <queue>
#include <algorithm>

#define _countof(array) (sizeof(array) / sizeof(array[0]))

int SCR_W = 1280;//480;
int SCR_H = 720;//360;
int LEVEL_W = 4096;
int LEVEL_H = 4096;
bool FULLSCREEN = true;
bool PLAY_TEST = false;
bool HOLD_CONSOLE_ON_EXIT = false;

enum AspectRatio{
    r16x9,
    r3x2
};

std::string getPath(){
    char buf[256];
    GetCurrentDirectoryA(256, buf);
    std::string retStr = std::string(buf) + "\\";
    return retStr;
}

SDL_Texture* loadTexture(std::string path, SDL_Renderer* renderer){
    SDL_Texture* retTex = NULL;

    SDL_Surface* tmpSurface = IMG_Load(path.c_str());
    if(tmpSurface == NULL){
        std::cout << "Error loading texture: Null Surface: " << path.c_str() << "\n";
        return nullptr;
    }else{
        retTex = SDL_CreateTextureFromSurface(renderer, tmpSurface);
        if(retTex == NULL){
            std::cout << "Error loading texture: Null Texture from Surface: " << path.c_str() << "\n";
            return nullptr;
        }
        SDL_FreeSurface(tmpSurface);
    }
    return retTex;
}

SDL_Texture* renderText(std::string _text, SDL_Color _col, SDL_Renderer* renderer, TTF_Font* _font){
    SDL_Texture* retTx = nullptr;
    SDL_Surface* textSurf = TTF_RenderText_Blended(_font, _text.c_str(), _col);
    if(textSurf== NULL){
        std::cout << "Could not render text: " << _text << "\n" << TTF_GetError() << "\n";
    }else{
        retTx = SDL_CreateTextureFromSurface(renderer, textSurf);
        if(retTx == NULL){
            std::cout << "Could not create texture from surface\n" << _text << "\n" << TTF_GetError() << "\n";
        }
        SDL_FreeSurface(textSurf);
    }
    return retTx;
}

std::string rectToString(SDL_Rect _r){
    std::string retStr = std::to_string(_r.x) + "," + std::to_string(_r.y) + " " + std::to_string(_r.w) + "," + std::to_string(_r.h);
    return retStr;
}

double _180_over_pi = (180.0 / M_PI);
double _pi_over_180 = (M_PI / 180.0);
enum Dir{
    dU=0,
    dUR=1,
    dR=2,
    dDR=3,
    dD=4,
    dDL=5,
    dL=6,
    dUL=7
};
Dir RadToDir(double _angle){
    double _angleD = _angle * _180_over_pi;
    if(_angleD > 180){
        _angleD -= 180.0;
    }else{
        _angleD += 180.0;
    }
    if(_angleD < 0){
        _angleD += 360;
    }
    double _dirSplit = 22.5;
    Dir _dir = Dir::dU;
    if(_angleD < _dirSplit){
        _dir = Dir::dU;
    }else if(_angleD < (3*_dirSplit)){
        _dir = Dir::dUR;
    }else if(_angleD < (5*_dirSplit)){
        _dir = Dir::dR;
    }else if(_angleD < (7*_dirSplit)){
        _dir = Dir::dDR;
    }else if(_angleD < (9*_dirSplit)){
        _dir = Dir::dD;
    }else if(_angleD < (11*_dirSplit)){
        _dir = Dir::dDL;
    }else if(_angleD < (13*_dirSplit)){
        _dir = Dir::dL;
    }else if(_angleD < (15*_dirSplit)){
        _dir = Dir::dUL;
    }else{
        _dir = Dir::dU;
    }
    return _dir;
}

class Game;
struct Stat;

class Button{
    enum type{
        func,
        statfunc,
        memberfunc,
        baseStatFunc
    };
    enum tStat{
        tsDMG,
        tsAUTOFIRE,
        tsRANGE
    };
    type T;
    double bounce;
    SDL_Rect area;
    Game* gamePtr;
    SDL_Texture* toolTipTex;
    Stat* upgStat;
    int tStatID;

    void (*clickFuncPtr)();
    void (Game::*clickMFuncPtr)();
    void (Game::*clickStatFuncPtr)(Stat& _stat, bool _texOnly);
    void (Game::*StatBaseUpgFuncPtr)(Stat& _stat, int _tStatID, bool _texOnly);
public:
    bool active;

    Button(SDL_Rect _area, void (*F)(), SDL_Texture* _toolTipTex);
    Button(SDL_Rect _area, void (Game::*F)(), Game* _game, SDL_Texture* _toolTipTex);
    Button(SDL_Rect _area, void (Game::*F)(Stat& _stat, bool _texOnly), Game* _game, Stat* _stat, SDL_Texture* _toolTipTex);
    Button(SDL_Rect _area, void (Game::*F)(Stat& _stat, int _tStatID, bool _texOnly), Game* _game, Stat* _stat, SDL_Texture* _toolTipTex, int _tStatID);
    ~Button();
    bool checkClick();
    bool checkOver(int x, int y);
    void update(double dTime);
    SDL_Rect& getArea();
    void moveButton(SDL_Rect _pos);
    Stat* getStat();
    SDL_Texture* getTexture();
};

SDL_Rect& Button::getArea(){
    return area;
}

Button::~Button(){
    gamePtr = nullptr;
    clickFuncPtr = nullptr;
    clickMFuncPtr = nullptr;
    clickStatFuncPtr = nullptr;
    StatBaseUpgFuncPtr = nullptr;
    toolTipTex = nullptr;
    upgStat = nullptr;
}

void Button::moveButton(SDL_Rect _pos){
    area = _pos;
}

Stat* Button::getStat(){
    return upgStat;
}

SDL_Texture* Button::getTexture(){
    return toolTipTex;
}

Button::Button(SDL_Rect _area, void (*F)(), SDL_Texture* _toolTipTex) : area(_area), toolTipTex(_toolTipTex), clickFuncPtr(F){
    bounce = 0.0;
    T = func;
    gamePtr = nullptr;
    upgStat = nullptr;
    active = true;
}

Button::Button(SDL_Rect _area, void (Game::*F)(), Game* _game, SDL_Texture* _toolTipTex) : gamePtr(_game), toolTipTex(_toolTipTex), clickMFuncPtr(F){
    bounce = 0.0;
    T = memberfunc;
    upgStat = nullptr;
    active = true;
}

Button::Button(SDL_Rect _area, void (Game::*F)(Stat& _stat, bool _texOnly), Game* _game, Stat* _stat, SDL_Texture* _toolTipTex) : area(_area), gamePtr(_game), toolTipTex(_toolTipTex), upgStat(_stat), clickStatFuncPtr(F){
    bounce = 0.0;
    T = statfunc;
    active = true;
}

Button::Button(SDL_Rect _area, void (Game::*F)(Stat& _stat, int _tStatID, bool _texOnly), Game* _game, Stat* _stat, SDL_Texture* _toolTipTex, int _tStatID) : area(_area), gamePtr(_game), toolTipTex(_toolTipTex), upgStat(_stat), tStatID(_tStatID), StatBaseUpgFuncPtr(F){
    bounce = 0.0;
    T = baseStatFunc;
    active = true;
}

bool Button::checkClick(){
    if(bounce == 0.0){
        switch(T){
            case func:
                clickFuncPtr();
                break;
            case statfunc:
                (gamePtr->*clickStatFuncPtr)(*upgStat, false);
                break;
            case memberfunc:
                (gamePtr->*clickMFuncPtr)();
                break;
            case baseStatFunc:
                (gamePtr->*StatBaseUpgFuncPtr)(*upgStat, tStatID, false);
                break;
        }
        bounce = 200.0;
        return true;
    }
    return false;
}

bool Button::checkOver(int x, int y){
    if(active){
        if((x > area.x) && (x < (area.x + area.w)) && (y > area.y) && (y < (area.y + area.h))){
            return true;
        }
    }
    return false;
}

void Button::update(double dTime){
    if(bounce > 0.0){
        bounce -= dTime;
    }else if(bounce < 0.0){
        bounce = 0.0;
    }
}

struct Anim{
    std::vector<SDL_Texture*> frames;
    double frameRate;

    Anim(Anim& _anim);
    Anim(double _frameRate);
    void addFrameTx(SDL_Texture* _newFrame);
};

Anim::Anim(Anim& _anim){
    frameRate = _anim.frameRate;
    for(std::vector<SDL_Texture*>::iterator it = frames.begin(); it != frames.end(); it++){
        frames.push_back((*it));
    }
}

Anim::Anim(double _frameRate) : frameRate(_frameRate){
}

void Anim::addFrameTx(SDL_Texture* _newFrame){
    frames.push_back(_newFrame);
}

struct AnimInst{
    Anim* anim;
    int currentFrame;

    SDL_Rect pos;
    double angle;
    SDL_Point offset;
    bool isLooping;
    double frameTicker, frameDelay;

    AnimInst(Anim* _anim, SDL_Rect _pos, double _angle, SDL_Point _offset, bool _loop, double _speed);
    virtual ~AnimInst();
    virtual bool update(double dTime);
};

AnimInst::AnimInst(Anim* _anim, SDL_Rect _pos, double _angle, SDL_Point _offset, bool _loop, double _speed) : anim(_anim), pos(_pos), angle(_angle), offset(_offset), isLooping(_loop){
    currentFrame = 0;
    frameTicker = 0;
    frameDelay = 1000.0 / (_anim->frameRate * _speed);
}

AnimInst::~AnimInst(){
    anim = nullptr;
}

bool AnimInst::update(double dTime){
    if((frameTicker - dTime) <= 0){
        frameTicker = frameDelay + (frameTicker - dTime);
        if(currentFrame >= (int(anim->frames.size()) - 1)){
            currentFrame = 0;
            if(!isLooping){
                return true;
            }
        }else{
            currentFrame++;
        }
    }else{
        frameTicker -= dTime;
    }
    return false;
}

struct SpriteAnimInst : public AnimInst{
    int w, h;
    SpriteAnimInst(Anim* _anim, SDL_Rect _pos, int _w, int _h, double _angle, SDL_Point _offset, bool _loop, double _speed);
    bool update(double dTime) override;
    ~SpriteAnimInst() override;
};

SpriteAnimInst::SpriteAnimInst(Anim* _anim, SDL_Rect _pos, int _w, int _h, double _angle, SDL_Point _offset, bool _loop, double _speed) : AnimInst(_anim, _pos, _angle, _offset, _loop, _speed), w(_w), h(_h){
}

SpriteAnimInst::~SpriteAnimInst(){
    anim = nullptr;
}

bool SpriteAnimInst::update(double dTime){
    if((frameTicker - dTime) <= 0){
        frameTicker = frameDelay + (frameTicker - dTime);
        int _width;
        SDL_QueryTexture(anim->frames[0], NULL, NULL, &_width, NULL);
        if(currentFrame >= ((_width / w) - 1)){
            currentFrame = 0;
            if(!isLooping){
                return true;
            }
        }else{
            currentFrame++;
        }
    }else{
        frameTicker -= dTime;
    }
    return false;
}

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
    Dir facingDir;
    SpriteAnimInst* animInst;

    static int getID();
    static void clearID(int _id);
    static bool checkID(int _id);
    static void initEnemies(int _maxEnemies);
    void update(double dTime, int _spdMult);
    bool takeDamage(int _dmg);
    bool dying;
};

std::queue<int> Enemy::ids;

int Enemy::getID(){
    int retID = -1;
    if(!ids.empty()){
        retID = ids.front();
        ids.pop();
    }
    return retID;
}

bool Enemy::checkID(int _id){
    std::queue<int> tQ = ids;
    while(!tQ.empty()){
        if(tQ.front() == _id){
            return false;
        }else{
            tQ.pop();
        }
    }
    return true;
}

void Enemy::initEnemies(int _maxEnemies){
    ids = std::queue<int>();
    for(int i = 0; i < _maxEnemies; i++){
        ids.push(i);
    }
}

void Enemy::clearID(int _id){
    if(checkID(_id) == true){
        ids.push(_id);
    }
}

bool Enemy::takeDamage(int _dmg){
    if((hp - _dmg) > 0){
        hp -= _dmg;
    }else{
        dying = true;
        return true;
    }
    return false;
}

void Enemy::update(double dTime, int _spdMult){
    isAttacking = false;
    animInst->update(dTime);
    if(!attackState){
        int dx = (LEVEL_W / 2.0) - (x + (w / 2.0));
        int dy = (LEVEL_H / 2.0) - (y + (h / 2.0));
        double dist = std::sqrt((dx*dx)+(dy*dy));
        double dxN = dx / dist;
        double dyN = dy / dist;
        if(dist > (128 + w)){
            y += ((spd * _spdMult * (dTime / 1000.0)) * dyN);
            x += ((spd * _spdMult * (dTime / 1000.0)) * dxN);
        }else{
            attackState = true;
        }
    }else{
        if((atkTicker - dTime) <= 0){
            atkTicker = atkDelay + (atkTicker - dTime);
            isAttacking = true;
        }else{
            atkTicker -= dTime;
        }
    }
}

struct Stat{
    int stat;
    int upgradeCost;
    int level;

    int texID;
    SDL_Rect texRect;
    SDL_Texture* textTex;
    int btnID;
    SDL_Rect btnTexRect;
    int iconID;
    SDL_Rect iconRect;

    Stat();
    Stat(SDL_Rect _rect);
    Stat(int x, int y, int w, int h);
    void moveStat(SDL_Rect _pos);
};
Stat::Stat(){
    btnID = -1;
    upgradeCost = 10;
    textTex = nullptr;
    iconID = -1;
    level = 1;
    stat = 1;
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

enum GState{
    gsInit,
    gsMainMenu,
    gsGameStart,
    gsGame,
    gsClose
};


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

    Turret(double _x, double _y, turretType _type, double _fireDelay);
    Turret();
    ~Turret();
    void setAngle(double _angle);
    void setTarget(Enemy* _target);
    void moveTurret(int _x, int _y);
    void setStatButton(int _buttonIndex, Button* _b);
    void activateButtons();
    void deactivateButtons();
};

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

Turret::Turret(double _x=0.0, double _y=0.0, turretType _type=Turret::turretType::tAutoGun, double _fireDelay=500.0) : type(_type), autoFireDelay(_fireDelay){
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

struct Base{
    enum UpdateState{
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
    bool isUpgrading;
    int upgStat;
    Turret* swapT1;
    Turret* swapT2;
    int swapT1ID, swapT2ID;
    double swapTime;
    double swapTicker;
    double upgradeTime;
    double upgradeTicker;
    double currentUpgradeTime;

    SDL_Rect turret_Rects[21];
    Turret* turretBays[21];

    Base(Game& _owner);
    Base();
    ~Base();
    void initTurretBases();
    int getFreeBase();
    int update(double _dTime);
    void setOwner(Game& _owner);
    void startSwap(int _bayID_1, int _bayID_2);
    void startUpgrade(int _stat);
};

Base::Base(Game& _owner):owner(&_owner){
    swapTime = 2000.0;
    swapTicker = swapTime;
    halfSwapPassed = false;
    swapT1ID = -1;
    swapT2ID = -1;
    upgradeTime = 2000.0;
    upgradeTicker = 0;
    currentUpgradeTime = 0.0;
};
Base::Base(){
    owner = nullptr;
    swapTime = 2000.0;
    swapTicker = swapTime;
    halfSwapPassed = false;
    swapT1ID = -1;
    swapT2ID = -1;
    upgradeTime = 2000.0;
    upgradeTicker = 0;
    currentUpgradeTime = 0.0;
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
    }

    int turret_base_w = 64;
    int turret_base_h = 64;
    double _x = ((LEVEL_W - turret_base_w) / 2.0);
    double _y = ((LEVEL_H - turret_base_w) / 2.0);
    SDL_Rect turretRect = {int(_x), int(_y), turret_base_w, turret_base_h};
    turret_Rects[0] = turretRect;
    turretBays[0] = nullptr;

    int turret_base_radius_outer = (pos.w / 2.0) + 104;
    int turret_base_radius_inner = (pos.w / 2.0) + 48;
    double turret_angle = 11.25 * _pi_over_180;
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
            turret_Rects[index] = turretRect;
            turretBays[index] = nullptr;
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
                    if(turretBays[_bayID_1] != nullptr){
                        swapT1 = turretBays[_bayID_1];
                        swapT1->isSwapping = true;
                    }else{
                        swapT1 = nullptr;
                    }
                    if(turretBays[_bayID_2] != nullptr){
                        swapT2 = turretBays[_bayID_2];
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

void Base::startUpgrade(int _stat){
    upgStat = _stat;
    if((turretBays[0] != nullptr) && (!isUpgrading)){
        currentUpgradeTime = upgradeTime * turretBays[0]->stats[_stat]->stat;
        upgradeTicker = currentUpgradeTime;
        isUpgrading = true;
    }
}

int Base::update(double _dTime){
    int _retState = Base::UpdateState::usNULL;
    if(isSwapping){
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
            _retState |= Base::UpdateState::usSWAP_FINISH;
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
                    _retState |= UpdateState::usHALF_SWAP;
                    if(turretBays[swapT1ID] == nullptr){
                        turretBays[swapT1ID] = turretBays[swapT2ID];
                        turretBays[swapT2ID] = nullptr;
                    }else if(turretBays[swapT2ID] == nullptr){
                        turretBays[swapT2ID] = turretBays[swapT1ID];
                        turretBays[swapT1ID] = nullptr;
                    }else{
                        Turret* _T1 = &(*(turretBays[swapT1ID]));
                        turretBays[swapT1ID] = turretBays[swapT2ID];
                        turretBays[swapT2ID] = _T1;
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
    if(isUpgrading){
        upgradeTicker -= _dTime;
        if(upgradeTicker <= 0){
            upgradeTicker = 0;
            _retState |= Base::UpdateState::usFINISH_UPGRADE;
            isUpgrading = false;
        }else{
            _retState |= Base::UpdateState::usUPGRADE;
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


class Game{
    friend class Base;
    SDL_Renderer* renderer;
    SDL_Texture* ScrSurface;
    SDL_Texture* LevelSurface;
    TTF_Font* fonts[3];
    Base base;
    int dmgIconID, autofireIconID, fireRangeIconID;

    uint32_t bgCol;
    SDL_Rect ScrnRect;
    SDL_Rect LevelRect;
    double ViewAngle;
    SDL_Rect ViewRect;
    double viewScale;


    std::vector<std::pair<int, std::string>> hiScores;
    SDL_Texture* hiscoreTexs[4];

    GState state;
    bool btnsActive;
    bool gameOver;
    bool paused;

    double fps;
    double fpsTicker;
    bool tick;
    int mouseX, mouseY;
    bool lClick;
    double clickBounce;

    int logoID;
    int ngbID;
    int nextWaveBtnID;
    int diedSplashID;
    int pausedSplashID;
    int grassBGID;

    int redEnemyID;
    int blueEnemyID;
    int greenEnemyID;

    int hpBarID;
    int hpBarBgID;
    int turretBaseID;
    int turretCannonIDs[3];
    int selectedBayID;
    //double turretAngle;
    //Enemy* targettedEnemy;
    //double baseRange;

    Anim* muzzleAnim;
    int muzzleFlashID;
    Anim* impactAnim;
    int impactAnimIDs[4];
    Anim* walkAnims[8];
    int walkAnimIDs[8];

    int statBtnID;

    int CoinTxID;
    SDL_Rect CoinIconRect;
    SDL_Texture* CoinTextTx;
    SDL_Rect CoinTextRect;
    double towerHp;
    double towerMaxHp;

    int currentWave;
    int nextWaveCounter;
    int nextWaveReq;

    SDL_Texture* WaveTextTx;
    SDL_Rect WaveTextRect;

    SDL_Rect logoRect;
    SDL_Rect ngbRect;
    SDL_Rect nextWaveRect;
    int bgXtiles;
    int bgYtiles;
    SDL_Rect grassBGRect;

    //SDL_Rect turretBaseRect;
    //SDL_Rect turretCannonRect;
    //SDL_Point turretPivotOffset;

    std::vector<Button*> buttons;
    std::vector<SDL_Texture*> textures;
    std::map<std::string, int> texIDs;
    std::vector<Enemy*> enemies;
    std::vector<Anim*> anims;
    std::vector<AnimInst*> animInstances;
    std::vector<SpriteAnimInst*> spriteAnimInstances;

    std::map<std::string, Stat*> statMap;
    std::vector<Stat*> stats;
    std::vector<Turret*> StoredTurrets;
    Turret* activeTurrets[21];

    int MAX_ENEMIES;
    static int TURRET_CYCLE;
    double spawnDelay;
    double spawnTicker;
    double fireDelay;
    double fireTicker;
    //double autoFireDelay;
    //double autoFireTicker;

    void initStats();

    void targetEnemy(int _tid);
    void targetEnemy(Turret* _t);
    void fireTurret(int _turretID);
    void fireTurret(Turret* _t);
    void drawBase();

    void statUpgF(Stat& _stat, bool _texOnly);
    void statBaseUpgF(Stat& _stat, int _tStatType, bool _textOnly);
    void drawUpgBtns();

    void newStatBtn(Stat* _stat, int _texID, std::string _toolTip);
    void newStatBtn(Stat* _stat, std::string _iconPath, std::string _toolTip);
    Button* newStatForTurret(Stat** _stat, int _texID, std::string _toolTip, int _tStatID);

    void updateCoins(int _coins);
    void updateWaves();

    bool updateAnims();
    void nextWaveBtnF();

    void loadHiscores();
public:
    Game();
    ~Game();
    bool update(double dTime);
    void setRenderer(SDL_Renderer* _renderer, SDL_Texture* _ScrSurface, SDL_Texture* _LevelSurface);
    void destroyButtons();
    void destroyTextures();
    void destroyEnemies();
    void destroyStats();
    void destroyTurrets();
    void init();
    void newTurret(Turret::turretType _turretType);
    void updateState(GState newState);
    int newTexture(std::string _path);
    void drawTexture(int _id, SDL_Rect _dest);
    void drawTexture(int _id, SDL_Rect _dest, double _angle);
    void drawTurret(int _id, SDL_Rect _dest, double _angle, SDL_Point _offset, double _range, double _swapScale=1.0);
    int spawnNewEnemy();
    void setTurret(int _bayID, Turret* _turret);
    bool cycleTurrets(Turret** _t, Turret** _tVec);
    AnimInst* spawnAnim(Anim* _anim, SDL_Rect _pos, double _angle, SDL_Point _offset, bool _loop, double _speed);
    SpriteAnimInst* spawnSpriteAnim(Anim* _anim, SDL_Rect _pos, int _w, int _h, double _angle, SDL_Point _offset, bool _loop, double _speed);
    void screenToLevel(double& _x, double& _y);
    void levelToScreen(double& _x, double& _y);
};

void Game::levelToScreen(double& _x, double& _y){
    _x = ((double(_x) / LEVEL_W) * (ViewRect.w * viewScale));
    _y = ((double(_y) / LEVEL_H) * (ViewRect.h * viewScale));
}

void Game::screenToLevel(double& _x, double& _y){
    SDL_Point _p = {int(LEVEL_W / 2.0), int(LEVEL_H / 2.0)};
    SDL_Rect scaledViewRect = ViewRect;
    scaledViewRect.w *= viewScale;
    scaledViewRect.h *= viewScale;
    scaledViewRect.x = _p.x - (scaledViewRect.w / 2.0);
    scaledViewRect.y = _p.y - (scaledViewRect.h / 2.0);
    _x = ((double(_x) / SCR_W) * scaledViewRect.w) + scaledViewRect.x;
    _y = ((double(_y) / SCR_H) * scaledViewRect.h) + scaledViewRect.y;
}

void Game::setTurret(int _bayID, Turret* _turret){
    if((_bayID >= 0) && (_bayID < 21)){
        activeTurrets[_bayID] = _turret;
    }
}

int Game::TURRET_CYCLE = 0;
bool Game::cycleTurrets(Turret** _t, Turret** _tVec=nullptr){
    if(_tVec == nullptr){
        _tVec = activeTurrets;
    }
    if(Game::TURRET_CYCLE >= 21){
        Game::TURRET_CYCLE = 0;
        *_t = nullptr;
        return false;
    }
    *_t = *(_tVec + Game::TURRET_CYCLE);
    Game::TURRET_CYCLE++;
    return true;
}

Game::Game(){
    state = gsInit;
    ViewAngle = 0.0;
    ScrnRect = {0, 0, SCR_W, SCR_H};

    LevelRect = {0, 0, LEVEL_W, LEVEL_H};
    ViewRect = {0, 0, SCR_W, SCR_H};
    viewScale = 1.0;

    hiscoreTexs[0] = nullptr;
    hiscoreTexs[1] = nullptr;
    hiscoreTexs[3] = nullptr;
    hiscoreTexs[4] = nullptr;

    for(int i = 0; i < 8; i++){
        walkAnims[i] = nullptr;
    }

    fps = 30.0;
    fpsTicker = 1000.0 / fps;

    base.setOwner(*(this));
    base.pos = {int(LEVEL_W / 2.0) - 128, int(LEVEL_H / 2.0) - 128, 256, 256};
    logoRect = {int((SCR_W / 2.0) - (SCR_W / 4.0)), int(SCR_H / 8.0), int(SCR_W / 2.0), int(SCR_H / 4.0)};
    ngbRect = {int(SCR_W / 2.0) - 128, int((SCR_H / 4.0) * 3.0), 256, 64};
    nextWaveRect = {int(SCR_W / 2.0) - 64, int(SCR_H / 0.75), 128, 32};

    bgXtiles = (LEVEL_W / 1280) + 1;
    bgYtiles = (LEVEL_H / 720) + 1;
    grassBGRect = {0, 0, 1280, 720};

    //turretBaseRect = {int(LEVEL_W / 2.0) - 32, int(LEVEL_H / 2.0) - 32, 64, 64};
    //turretCannonRect = {turretBaseRect.x + 20, turretBaseRect.y - 20, 24, 64};
    //turretPivotOffset = {12, 52};

    SDL_GetMouseState(&mouseX, &mouseY);

    MAX_ENEMIES = 250;

    //baseRange = 220.0;

    renderer = nullptr;
    selectedBayID = -1;

    CoinIconRect = {int(SCR_W / 8.0), 10, 32, 32};
    CoinTextRect = {int(SCR_W / 8.0) + 32, 12, 64, 32};

    WaveTextRect = {int((SCR_W / 2.0)), 10, 64, 32};
    btnsActive = true;
    tick = false;
    fonts[0] = nullptr;
    fonts[1] = nullptr;
    fonts[2] = nullptr;
    logoID = -1;
    ngbID = -1;
    grassBGID = -1;
    dmgIconID = -1;
    autofireIconID = -1;
    fireRangeIconID = -1;
}

void Game::loadHiscores(){
    std::ifstream hiscoresF;
    hiscoresF.open(getPath() + "hiscores.sav");
    if(hiscoresF.is_open()){
        std::string line;
        while(std::getline(hiscoresF, line)){
            std::string tStrPlayer = line.substr(0, line.find(":"));
            std::string tStrScore = line.substr(line.find(":")+1);
            hiScores.push_back(make_pair(std::stoi(tStrScore), tStrPlayer));
        }
    }
    hiscoresF.close();
    sort(hiScores.begin(), hiScores.end());
    reverse(hiScores.begin(), hiScores.end());
    hiscoreTexs[0] = renderText("HiScores", {0,0,0}, renderer, fonts[2]);
    for(int i = 0; i < 3; i++){
        std::string tStr = hiScores[i].second + " : "  + std::to_string(hiScores[i].first);
        hiscoreTexs[i+1] = renderText(tStr, {0,0,0}, renderer, fonts[2]);
    }
}

void Game::init(){
    btnsActive = true;
    tick = false;
    fonts[0] = nullptr;
    fonts[1] = nullptr;
    fonts[2] = nullptr;
    gameOver = false;
    paused = false;

    //turretAngle = 0.0;
    muzzleAnim = nullptr;
    muzzleFlashID = -1;
    impactAnim = nullptr;
    impactAnimIDs[0] = -1;
    impactAnimIDs[1] = -1;
    impactAnimIDs[2] = -1;
    impactAnimIDs[3] = -1;

    //targettedEnemy = nullptr;
    lClick = false;
    clickBounce = 0.0;
    logoID = -1;
    ngbID = -1;
    nextWaveBtnID = -1;
    diedSplashID = -1;
    pausedSplashID = -1;

    redEnemyID = -1;
    blueEnemyID = -1;
    greenEnemyID = -1;

    spawnDelay = 1000.0;
    //autoFireDelay = 500.0;
    spawnTicker = 0.0;
    //autoFireTicker = 0.0;

    towerHp = 250.0;
    towerMaxHp = 250.0;
    hpBarID = -1;
    hpBarBgID = -1;

    base.baseTexID = -1;
    base.turretBaseTexID = -1;
    statBtnID = -1;

    base.coins = 0;
    CoinTxID = -1;
    CoinTextTx = nullptr;

    currentWave = 1;
    nextWaveCounter = 0;
    nextWaveReq = 5;
    WaveTextTx = nullptr;

    initStats();
    fonts[0] = TTF_OpenFont("slkscr.ttf", 8);
    fonts[1] = TTF_OpenFont("slkscr.ttf", 16);
    fonts[2] = TTF_OpenFont("slkscr.ttf", 24);
    loadHiscores();
    selectedBayID = -1;
}

void Game::initStats(){
    std::list<std::string> statNames = {"CoinDrop", "regenHP", "maxHP", "SpawnRate", "EnemySpeed", "maxEnemies"};
    int statOffset = int(SCR_H / 4.0) * 3.0;
    int rowOffset = 80 + 5;
    int max_row = 4;
    int iCount = 0;
    for(std::list<std::string>::iterator it = statNames.begin(); it != statNames.end(); it++, iCount++){
        statMap[(*it)] = new Stat(10 + (rowOffset * (iCount / max_row)), statOffset + ((iCount % max_row) * 20), 80, 16);
    }
}

void Game::destroyStats(){
    for(std::map<std::string, Stat*>::iterator it = statMap.begin(); it != statMap.end(); it++){
        (*it).second = nullptr;
    }
    statMap.clear();
}

void Game::destroyButtons(){
    for(std::vector<Button*>::iterator it = buttons.begin(); it != buttons.end(); it++){
        delete (*it);
    }
    buttons.clear();
}

void Game::destroyTurrets(){
    Turret* t = nullptr;
    while(cycleTurrets(&t, activeTurrets)){
        delete t;
    }
    std::vector<Turret*>::iterator it;
    for(it = StoredTurrets.begin(); it != StoredTurrets.end(); it++){
        delete (*it);
    }
    StoredTurrets.clear();
    for(int i = 0; i < 21; i++){
        base.turretBays[i] = nullptr;
    }
}

void Game::destroyTextures(){
    for(std::vector<SDL_Texture*>::iterator it = textures.begin(); it != textures.end(); it++){
        SDL_Texture* T = (*it);
        SDL_DestroyTexture(T);
    }
    textures.clear();
    texIDs.clear();
    logoID = -1;
    ngbID = -1;
    hpBarID = -1;
}

void Game::destroyEnemies(){
    for(std::vector<Enemy*>::iterator it = enemies.begin(); it != enemies.end(); it++){
        delete (*it);
    }
    enemies.clear();
}

void Game::targetEnemy(int _tIndex){
    if((_tIndex < 21) && (_tIndex >= 0)){
        targetEnemy(activeTurrets[_tIndex]);
    }else{
        std::cout << "Invalid turret index: " << _tIndex << "\n";
    }
}

void Game::targetEnemy(Turret* _t){
    if(_t != nullptr){
        Turret& t = *(_t);
        double lowestDist = t.range + (10 * t.stats[Turret::tStat::tsRANGE]->stat);
        for(std::vector<Enemy*>::iterator it = enemies.begin(); it != enemies.end(); it++){
            Enemy& E = (*(*it));
            double xd = (E.x + (E.w / 2.0)) - (t.baseRect.x + (t.baseRect.w / 2.0));
            double yd = (t.baseRect.y + (t.baseRect.h / 2.0)) - (E.y + (E.h / 2.0));
            double dist = std::abs(sqrt((xd*xd)+(yd*yd)));
            if(dist < lowestDist){
                lowestDist = dist;
                double newAngle = (std::atan2(-yd, xd) * (180.0 / M_PI)) + 90.0;
                t.angle = newAngle;
                t.target = &E;
                t.targetID = E.id;
            }
        }
    }else{
        //null turret
    }
}

void printDir(Dir _dir){
    std::string oStr = "";
    switch(_dir){
        case Dir::dU:
            oStr = "Up\n";
            break;
        case Dir::dUR:
            oStr = "Up - RIGHT\n";
            break;
        case Dir::dR:
            oStr = "RIGHT\n";
            break;
        case Dir::dDR:
            oStr = "DOWN - RIGHT\n";
            break;
        case Dir::dD:
            oStr = "DOWN\n";
            break;
        case Dir::dDL:
            oStr = "DOWN - LEFT\n";
            break;
        case Dir::dL:
            oStr = "LEFT\n";
            break;
        case Dir::dUL:
            oStr = "Up - LEFT\n";
            break;
    }
    std::cout << oStr;
}

double nAngle = -90.0;
bool ENEMY_SPAWN_TEST_ANGLE = false;
int Game::spawnNewEnemy(){
    if(int(enemies.size()) < (10 + (statMap["maxEnemies"]->stat * 5)) && (int(enemies.size()) < MAX_ENEMIES)){
        Enemy& E = *(new Enemy());
        E.id = Enemy::getID();
        E.maxHp = 5 * (1.0 + (currentWave / 2.0));
        E.hp = 5 * (1.0 + (currentWave / 2.0));
        double newAngle = ((rand() % 360) * _pi_over_180);
        if(ENEMY_SPAWN_TEST_ANGLE){
            newAngle = nAngle * _pi_over_180;
            nAngle += 18.0;
        }
        E.walkingAngleR = newAngle + (M_PI / 2.0);
        E.facingDir = RadToDir(E.walkingAngleR);
        double spawnRadius = LEVEL_W / 4.0;
        E.x = (int(LEVEL_W / 2.0) - 8) + (spawnRadius * std::cos(newAngle));
        E.y = (int(LEVEL_H / 2.0) - 8) + (spawnRadius * std::sin(newAngle));
        E.w = 32.0;
        E.h = 32.0;
        E.animInst = spawnSpriteAnim(walkAnims[int(E.facingDir)], SDL_Rect{int(E.x), int(E.y), int(E.w), int(E.h)}, int(E.w), int(E.h), 0.0, SDL_Point{0,0}, true, 1.0);
        E.spd = 32.0;
        E.coinDrop = 1 + (currentWave / 2.0);
        E.atkDelay = 1000.0;
        E.dmg = 1 + (currentWave / 2.5);
        E.aspd = 0.5;

        if((currentWave % 10) == 0){
            E.type = Enemy::Types::tBoss;
            E.w *= 4;
            E.h *= 4;
            E.maxHp *= 20;
            E.hp *= 20;
            E.spd *= 0.5;
            E.dmg *= 5;
            E.coinDrop *= 10;
        }else{
            if(((currentWave - 3) % 10) == 0){
                E.type = Enemy::Types::tFast;
                E.spd *= 2;
            }else if(((currentWave + 3) % 10) == 0){
                E.type = Enemy::Types::tHeavy;
                E.w *= 2;
                E.h *= 2;
                E.maxHp *= 2;
                E.hp *= 2;
                E.spd *= 0.75;
            }else{
                E.type = Enemy::Types::tRegular;
            }
        }

        enemies.push_back(&E);
        return E.id;
    }
    return -1;
}

Game::~Game(){
    destroyButtons();
    destroyTextures();
    destroyEnemies();
    destroyStats();
    destroyTurrets();
    renderer = nullptr;
    ScrSurface = nullptr;
    LevelSurface = nullptr;
    fonts[0] = nullptr;
    fonts[1] = nullptr;
    fonts[2] = nullptr;
    muzzleAnim = nullptr;
    impactAnim = nullptr;
    CoinTextTx = nullptr;
    WaveTextTx = nullptr;
}

Game game;

void Game::newStatBtn(Stat* _stat, int _texID, std::string _toolTip){
    _stat->btnID = statBtnID;
    SDL_Texture* _tooltipTex = renderText(_toolTip, {0,0,0}, renderer, fonts[1]);
    buttons.push_back(new Button(_stat->btnTexRect, &statUpgF, this, _stat, _tooltipTex));
    SDL_Color _col;
    if(_stat->upgradeCost <= base.coins){
        _col = {21, 111, 48};
    }else{
        _col = {208, 0, 0};
    }
    _stat->textTex = renderText(" " + std::to_string(_stat->level) + " : " + std::to_string(_stat->upgradeCost), _col, renderer, fonts[1]);
    _stat->iconID = _texID;
}

void Game::newStatBtn(Stat* _stat, std::string _iconPath, std::string _toolTip){
    _stat->iconID = newTexture(_iconPath);
    newStatBtn(_stat, _stat->iconID, _toolTip);
}

Button* Game::newStatForTurret(Stat** _stat, int _texID, std::string _toolTip, int _tStatID){
    (*_stat) = new Stat(0, 0, 0, 0);
    (*_stat)->btnID = statBtnID;
    SDL_Texture* _tooltipTex = renderText(_toolTip, {0,0,0}, renderer, fonts[1]);
    Button* _B = new Button((*_stat)->btnTexRect, &statBaseUpgF, this, (*_stat), _tooltipTex, _tStatID);
    buttons.push_back(_B);
    SDL_Color _col;
    if((*_stat)->upgradeCost <= base.coins){
        _col = {21, 111, 48};
    }else{
        _col = {208, 0, 0};
    }
    (*_stat)->textTex = renderText(" " + std::to_string((*_stat)->level) + " : " + std::to_string((*_stat)->upgradeCost), _col, renderer, fonts[1]);
    (*_stat)->iconID = _texID;
    return _B;
}

void Game::updateCoins(int _coins){
    base.coins += _coins;
    CoinTextTx = renderText(std::to_string(base.coins), {0,0,0}, renderer, fonts[2]);
}

void Game::updateWaves(){
    nextWaveCounter++;
    if(nextWaveCounter >= nextWaveReq){
        currentWave++;
        if((currentWave % 10) == 0){
            nextWaveReq = 1;
        }
        if(((currentWave - 1) % 5) == 0){
            nextWaveReq = (1 + (currentWave / 5)) * 5;
        }
        nextWaveCounter = 0;
    }
    std::string tStr = "Wave " + std::to_string(currentWave) + " (" + std::to_string(nextWaveCounter) + " / " + std::to_string(nextWaveReq) + ")";
    WaveTextTx = renderText(tStr, {0,0,0}, renderer, fonts[2]);
}

void Game::nextWaveBtnF(){
    nextWaveCounter = 0;
    std::string tStr = "Wave " + std::to_string(currentWave) + " (" + std::to_string(nextWaveCounter) + " / " + std::to_string(nextWaveReq) + ")";
    WaveTextTx = renderText(tStr, {0,0,0}, renderer, fonts[2]);
}

void newGameBtn(){
    game.updateState(gsGameStart);
    game.destroyTextures();
    game.destroyButtons();
}

int Game::newTexture(std::string _path){
    int retID = -1;
    std::string path = getPath();
    std::map<std::string, int>::iterator texIDtmp = texIDs.find(path + _path);

    if(texIDtmp != texIDs.end()){
        retID = texIDtmp->second;
    }else{
        textures.push_back(loadTexture((path + _path), renderer));
        retID = int(textures.size() - 1);
    }
    if(retID == -1){
        std::cout << "Error - Loading " << _path << "\n";
    }
    return retID;
}

void Game::drawTexture(int _id, SDL_Rect _dest){
    if(_id < int(textures.size())){
        if(textures[_id] != nullptr){
            SDL_RenderCopy(renderer, textures[_id], NULL, &_dest);
        }
    }
}

void Game::drawTexture(int _id, SDL_Rect _dest, double _angle){
    if(_id < int(textures.size())){
        if(textures[_id] != nullptr){
            SDL_RenderCopyEx(renderer, textures[_id], NULL, &_dest, _angle, NULL, SDL_FLIP_NONE);
        }
    }
}

void DrawCircle(SDL_Renderer* renderer, int32_t centreX, int32_t centreY, int32_t radius){
    const int32_t diameter = (radius * 2);

    int32_t x = (radius - 1);
    int32_t y = 0;
    int32_t tx = 1;
    int32_t ty = 1;
    int32_t error = (tx - diameter);

    while (x >= y){
        // Each of the following renders an octant of the circle
        SDL_RenderDrawPoint(renderer, centreX + x, centreY - y);
        SDL_RenderDrawPoint(renderer, centreX + x, centreY + y);
        SDL_RenderDrawPoint(renderer, centreX - x, centreY - y);
        SDL_RenderDrawPoint(renderer, centreX - x, centreY + y);
        SDL_RenderDrawPoint(renderer, centreX + y, centreY - x);
        SDL_RenderDrawPoint(renderer, centreX + y, centreY + x);
        SDL_RenderDrawPoint(renderer, centreX - y, centreY - x);
        SDL_RenderDrawPoint(renderer, centreX - y, centreY + x);

        if(error <= 0){
            ++y;
            error += ty;
            ty += 2;
        }
        if(error > 0){
            --x;
            tx += 2;
            error += (tx - diameter);
        }
    }
}

void Game::drawTurret(int _id, SDL_Rect _dest, double _angle, SDL_Point _offset, double _range, double _swapScale){
    if(_id < int(textures.size())){
        SDL_Rect _scaledRect = _dest;
        _scaledRect.w = double(_scaledRect.w) * _swapScale;
        _scaledRect.h = double(_scaledRect.h) * _swapScale;
        _scaledRect.x = _scaledRect.x + ((_dest.w - _scaledRect.w) / 2.0);
        _scaledRect.y = _scaledRect.y + ((_dest.h - _scaledRect.h) / 2.0);
        SDL_RenderCopyEx(renderer, textures[_id], NULL, &_scaledRect, _angle, &_offset, SDL_FLIP_NONE);
    }
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    DrawCircle(renderer, _dest.x + (_dest.w / 2.0), _dest.y + 20 + (_dest.h / 2.0), _range);
}

void Game::statUpgF(Stat& _stat, bool _texOnly = false){
    if(!_texOnly){
        if(base.coins >= _stat.upgradeCost){
            updateCoins(-_stat.upgradeCost);
            _stat.upgradeCost *= 1.3;
            _stat.level++;
            _stat.stat++;
        }
    }
    SDL_DestroyTexture(_stat.textTex);
    std::string _tLvl;
    std::string _tUpgC;
    if((_stat.level / 1000.0) > 1){
        if((_stat.level / 1000000.0) > 1){
            _tLvl = std::to_string(_stat.level / 1000000) = "M";
        }else{
            _tLvl = std::to_string(_stat.level / 1000) + "K";
        }
    }else{
        _tLvl = std::to_string(_stat.level);
    }
    if((_stat.upgradeCost / 1000.0) > 1){
        if((_stat.upgradeCost / 1000000.0) > 1){
            _tUpgC = std::to_string(_stat.upgradeCost / 1000000) + "M";
        }else{
            _tUpgC = std::to_string(_stat.upgradeCost / 1000) + "K";
        }
    }else{
        _tUpgC = std::to_string(_stat.upgradeCost);
    }
    SDL_Color _col;
    if(_stat.upgradeCost <= base.coins){
        _col = {21, 111, 48};
    }else{
        _col = {208, 0, 0};
    }
    _stat.textTex = renderText(_tLvl + ": " + _tUpgC, _col, renderer, fonts[1]);
}

void Game::statBaseUpgF(Stat& _stat, int _tStatID, bool _texOnly){
    base.startUpgrade(_tStatID);
}

void Game::drawUpgBtns(){
    for(std::map<std::string, Stat*>::iterator it = statMap.begin(); it != statMap.end(); it++){
        Stat& s = *((*it).second);
        statUpgF(s, true);
        drawTexture(s.btnID, s.btnTexRect);
        drawTexture(base.turretBaseTexID, s.iconRect);
        drawTexture(s.iconID, s.iconRect);
        int txw, txh;
        SDL_QueryTexture(s.textTex, NULL, NULL, &txw, &txh);
        s.texRect.w = txw * 0.8;
        s.texRect.h = txh;
        SDL_RenderCopy(renderer, s.textTex, NULL, &(s.texRect));
    }
}

AnimInst* Game::spawnAnim(Anim* _anim, SDL_Rect _pos, double _angle, SDL_Point _offset, bool _loop, double _speed){
    AnimInst* _A = new AnimInst(_anim, _pos, _angle, _offset, _loop, _speed);
    return _A;
}
SpriteAnimInst* Game::spawnSpriteAnim(Anim* _anim, SDL_Rect _pos, int _w, int _h, double _angle, SDL_Point _offset, bool _loop, double _speed){
    SpriteAnimInst* _A = new SpriteAnimInst(_anim, _pos, _w, _h, _angle, _offset, _loop, _speed);
    return _A;
}

void Game::fireTurret(int _turretID){
    if((_turretID >= 0) && (_turretID < 21)){
        fireTurret(activeTurrets[_turretID]);
    }else{
        std::cout << "FireTurret: Invalid turret ID: " << _turretID << "\n";
    }
}

void Game::fireTurret(Turret* _t){
    if(_t != nullptr){
        Turret& t = *(_t);
        if((t.target != nullptr) && (Enemy::checkID(t.targetID) == true)){
            if(t.target->takeDamage(t.stats[Turret::tStat::tsDMG]->stat)){
                t.target = nullptr;
            }
            SDL_Rect pos = {t.cannonRect.x - 4, t.cannonRect.y - 32, 32, 32};
            SDL_Point offset = t.pivotOffset;
            offset.x += 4;
            offset.y += 32;
            animInstances.push_back(spawnAnim(muzzleAnim, pos, t.angle, offset, false, 1.0));
        }
    }else{
        std::cout << "FireTurret: Null pointer\n";
    }
}

void Game::drawBase(){
    drawTexture(base.baseTexID, base.pos);
    for(int i = 0; i < 21; i++){
        if(i == selectedBayID){
            SDL_Rect _selectedRect = base.turret_Rects[i];
            _selectedRect.w = double(_selectedRect.w) * 1.5;
            _selectedRect.h = double(_selectedRect.h) * 1.5;
            _selectedRect.x -= double(_selectedRect.w) / 6;
            _selectedRect.y -= double(_selectedRect.h) / 6;
            drawTexture(base.turretBaseTexID, _selectedRect);
        }else{
            drawTexture(base.turretBaseTexID, base.turret_Rects[i]);
        }
    }
}

void Game::newTurret(Turret::turretType _turretType){
    int _newTurretID = base.getFreeBase();
    if((_newTurretID != -1) && (_newTurretID < 21)){
        double _x = base.turret_Rects[_newTurretID].x;
        double _y = base.turret_Rects[_newTurretID].y;
        Turret* t = new Turret(_x, _y, _turretType);
        activeTurrets[_newTurretID] = t;
        base.turretBays[_newTurretID] = t;
        t->setStatButton(Turret::tsDMG, newStatForTurret(&(t->stats[Turret::tsDMG]), dmgIconID, "+ Dmg ", 0));
        t->setStatButton(Turret::tsAUTOFIRE, newStatForTurret(&(t->stats[Turret::tsAUTOFIRE]), autofireIconID, "+ FireRate ", 1));
        t->setStatButton(Turret::tsRANGE, newStatForTurret(&(t->stats[Turret::tsRANGE]), fireRangeIconID, "+ Range ", 2));
        t->statButtons[Turret::tsDMG]->active = false;
        t->statButtons[Turret::tsAUTOFIRE]->active = false;
        t->statButtons[Turret::tsRANGE]->active = false;
    }
}

int healDelay = 10000.0;
int healTicker = healDelay;
bool Game::update(double dTime){
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
    SDL_SetRenderTarget(renderer, ScrSurface);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 240, 240, 0, 255);
    SDL_SetRenderTarget(renderer, LevelSurface);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, 0);
    SDL_RenderClear(renderer);

    SDL_Event e;

    tick = false;
    if(!paused){
        fpsTicker -= dTime;
        if(clickBounce > 0.0){
            clickBounce -= dTime;
        }
        if(fpsTicker < 0){
            fpsTicker = 1000.0 / fps;
            tick = true;
        }
        healTicker -= dTime;
        if(healTicker < 0){
            healTicker = healDelay;
            double healAmt = (statMap["regenHP"]->stat * 0.25);
            if((towerHp + healAmt) >= towerMaxHp){
                towerHp = towerMaxHp;
            }else{
                towerHp += healAmt;
            }
        }
    }
    lClick = false;

    while(SDL_PollEvent(&e) != 0){
        switch(e.type){
            case SDL_QUIT:
                return true;
                break;
            case SDL_KEYDOWN:
                switch(e.key.keysym.sym){
                    case SDLK_q:
                        return true;
                        break;
                    case SDLK_LEFT:
                        if(ViewAngle > (-360.0)){
                            ViewAngle -= 90.0;
                        }else{
                            ViewAngle = 0.0;
                        }
                        break;
                    case SDLK_RIGHT:
                        if(ViewAngle < (360.0)){
                            ViewAngle += 90.0;
                        }else{
                            ViewAngle = 0.0;
                        }
                        break;
                    case SDLK_UP:
                        if(viewScale > 0.5){
                            viewScale -= 0.1;
                        }else{
                            viewScale = 0.5;
                        }
                        break;
                    case SDLK_DOWN:
                        if(viewScale < 2.0){
                            viewScale += 0.1;
                        }else{
                            viewScale = 2.0;
                        }
                        break;
                    case SDLK_RETURN:
                        if(gameOver){
                            updateState(gsInit);
                        }
                        break;
                    case SDLK_p:
                        paused = !paused;
                        break;
                }
                break;
            case SDL_MOUSEMOTION:
                SDL_GetMouseState(&mouseX, &mouseY);
                break;
            case SDL_MOUSEBUTTONDOWN:
                if(e.button.button == SDL_BUTTON_LEFT){
                    if(clickBounce <= 0.0){
                        if(gameOver){
                            updateState(gsInit);
                        }
                        lClick = true;
                        clickBounce = 10.0;
                    }else{
                        lClick = false;
                    }
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if(e.button.button == SDL_BUTTON_LEFT){
                    lClick = false;
                }
                break;
            case SDL_MOUSEWHEEL:
                if(e.wheel.y > 0){
                    if(viewScale < 2.0){
                        viewScale += e.wheel.y * 0.1;
                    }
                    if(viewScale > 2.0){
                        viewScale = 2.0;
                    }
                }else if(e.wheel.y < 0){
                    if(viewScale > 0.5){
                        viewScale += e.wheel.y * 0.1;
                    }
                    if(viewScale < 0.5){
                        viewScale = 0.5;
                    }
                }
                break;
        }
    }// poll &e

    if(btnsActive){
        for(std::vector<Button*>::iterator it = buttons.begin(); it != buttons.end(); it++){
            Button& B = *(*it);
            B.update(dTime);
            if(B.checkOver(mouseX, mouseY)){
                if(lClick){
                    if(B.checkClick()){
                        break;
                    }
                }
                Stat* _s = B.getStat();
                SDL_Rect tRect;
                if(_s == nullptr){
                    tRect = ngbRect;
                }else{
                    tRect = _s->btnTexRect;
                }
                tRect.x += 16;
                tRect.y -= 16;
                int txw, txh;
                SDL_Texture* _tex = B.getTexture();
                SDL_QueryTexture(_tex, NULL, NULL, &txw, &txh);
                tRect.w = txw;
                tRect.h = txh;
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderFillRect(renderer, &tRect);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderDrawRect(renderer, &tRect);
                SDL_RenderCopy(renderer, _tex, NULL, &tRect);
            }
        }
    }

    if(state == gsInit){
        destroyButtons();
        destroyEnemies();
        destroyTextures();
        destroyStats();
        destroyTurrets();

        init();
        logoID = newTexture("TowerDefence.png");
        ngbID = newTexture("NewGame.png");
        SDL_Texture* _ttTex = renderText("New Game Button", {0,0,0}, renderer, fonts[2]);
        buttons.push_back(new Button(ngbRect, newGameBtn, _ttTex));
        towerHp = 250;
        towerMaxHp = 250;
        updateState(gsMainMenu);
    }else if(state == gsMainMenu){
        SDL_SetRenderTarget(renderer, ScrSurface);
        drawTexture(logoID, logoRect);
        drawTexture(ngbID, ngbRect);

        SDL_Rect scoreRect = {int((SCR_W / 2.0) - (SCR_W / 8.0)), int((SCR_H / 16.0) * 7.0), int(SCR_W / 4.0), int(SCR_H / 32.0)};
        SDL_RenderCopy(renderer, hiscoreTexs[0], NULL, &scoreRect);
        scoreRect.y = int((SCR_H / 16.0) * 8.0);
        SDL_RenderCopy(renderer, hiscoreTexs[1], NULL, &scoreRect);
        scoreRect.y = int((SCR_H / 16.0) * 9.0);
        scoreRect.x += int((scoreRect.w * 0.1));
        scoreRect.w = int(scoreRect.w * 0.8);
        scoreRect.h = int(scoreRect.h * 0.9);
        SDL_RenderCopy(renderer, hiscoreTexs[2], NULL, &scoreRect);
        scoreRect.y = int((SCR_H / 16.0) * 10.0);
        scoreRect.x += int(scoreRect.w * 0.1);
        scoreRect.w = int(scoreRect.w * 0.8);
        scoreRect.h = int(scoreRect.h * 0.9);
        SDL_RenderCopy(renderer, hiscoreTexs[3], NULL, &scoreRect);
        SDL_SetRenderTarget(renderer, 0);
    }else if(state == gsGameStart){
        std::string path = getPath();
        nextWaveBtnID = newTexture("nextWave.png");
        diedSplashID = newTexture("diedSplash.png");
        pausedSplashID = newTexture("pausedSplash.png");

        redEnemyID = newTexture("redEnemy.png");
        blueEnemyID = newTexture("blueEnemy.png");
        greenEnemyID = newTexture("greenEnemy.png");
        grassBGID = newTexture("grass_bg.jpg");

        hpBarID = newTexture("hpBar.png");
        hpBarBgID = newTexture("hpBar_bg.png");
        Enemy::initEnemies(MAX_ENEMIES);

        turretBaseID = newTexture("td_basic_towers\\Tower.png");
        turretCannonIDs[0] = newTexture("td_basic_towers\\MG3.png");
        turretCannonIDs[1] = newTexture("td_basic_towers\\Missile_Launcher.png");
        turretCannonIDs[2] = newTexture("td_basic_towers\\Cannon.png");
        base.baseTexID = newTexture("circle_fence.png");
        base.turretBaseTexID = newTexture("icon_Base.png");
        muzzleFlashID = newTexture("MuzzleFlash.png");

        impactAnimIDs[0] = newTexture("impact_0.png");
        impactAnimIDs[1] = newTexture("impact_1.png");
        impactAnimIDs[2] = newTexture("impact_2.png");
        impactAnimIDs[3] = newTexture("impact_3.png");

        walkAnimIDs[0] = newTexture("Walk_Sprites\\Z1WalkUp.png");
        walkAnimIDs[1] = newTexture("Walk_Sprites\\Z1WalkUpRight.png");
        walkAnimIDs[2] = newTexture("Walk_Sprites\\Z1WalkRight.png");
        walkAnimIDs[3] = newTexture("Walk_Sprites\\Z1WalkDownRight.png");
        walkAnimIDs[4] = newTexture("Walk_Sprites\\Z1WalkDown.png");
        walkAnimIDs[5] = newTexture("Walk_Sprites\\Z1WalkDownLeft.png");
        walkAnimIDs[6] = newTexture("Walk_Sprites\\Z1WalkLeft.png");
        walkAnimIDs[7] = newTexture("Walk_Sprites\\Z1WalkUpLeft.png");

        CoinTxID = newTexture("coin.png");
        updateCoins(100);
        WaveTextTx = renderText("Wave " + std::to_string(currentWave) + " (" + std::to_string(nextWaveCounter) + " / " + std::to_string(nextWaveReq) + ")", {0,0,0}, renderer, fonts[2]);

        statBtnID = newTexture("button_turq.png");

        dmgIconID = newTexture("bullet.png");
        newStatBtn(statMap["CoinDrop"], CoinTxID, "+ Coin Drop");
        newStatBtn(statMap["SpawnRate"], "Hourglass.png", "- Spawn Delay");
        newStatBtn(statMap["EnemySpeed"], "speedIcon.png", "+ Enemy Speed");
        autofireIconID = newTexture("autofireIcon.png");
        fireRangeIconID = newTexture("rangeIcon.png");
        newStatBtn(statMap["maxEnemies"], "maxEnemyIcon.png", "+ Max Enemies");
        newStatBtn(statMap["regenHP"], "regenHPIcon.png", "+ HP Regen");
        newStatBtn(statMap["maxHP"], "maxHPIcon.png", "+ Max HP");

        muzzleAnim = new Anim{30.0};
        muzzleAnim->addFrameTx(textures[muzzleFlashID]);
        muzzleAnim->addFrameTx(textures[muzzleFlashID]);

        impactAnim = new Anim{30.0};
        impactAnim->addFrameTx(textures[impactAnimIDs[0]]);
        impactAnim->addFrameTx(textures[impactAnimIDs[1]]);
        impactAnim->addFrameTx(textures[impactAnimIDs[2]]);
        impactAnim->addFrameTx(textures[impactAnimIDs[3]]);

        for(int i = 0; i < 8; i++){
            walkAnims[i] = new Anim(10.0);
            walkAnims[i]->addFrameTx(textures[walkAnimIDs[i]]);
            walkAnims[i]->addFrameTx(textures[walkAnimIDs[i]]);
        }

        base.initTurretBases();
        int _turretCount = 3;
        for(int i = 0; i < _turretCount; i++){
            newTurret(static_cast<Turret::turretType>(i%3));
        }
        paused = false;
        updateState(gsGame);
    }else if(state == gsGame){
        SDL_SetRenderTarget(renderer, LevelSurface);

        SDL_Rect tRect = grassBGRect;
        for(int i = 0; i < bgXtiles; i++){
            for(int j = 0; j < bgYtiles; j++){
                tRect.x = i * 1280;
                tRect.y = j * 720;
                drawTexture(grassBGID, tRect);
            }
        }
        towerMaxHp = 240 + (statMap["maxHP"]->stat * 10);

        if((lClick) && (!base.isSwapping) && (!base.isUpgrading)){
            bool clickedBay = false;
            for(int i = 0; i < 21; i++){
                double scaledMX = double(mouseX);
                double scaledMY = double(mouseY);
                screenToLevel(scaledMX, scaledMY);
                if((scaledMX > base.turret_Rects[i].x) && (scaledMX < (base.turret_Rects[i].x + base.turret_Rects[i].w))
                && (scaledMY > base.turret_Rects[i].y) && (scaledMY < (base.turret_Rects[i].y + base.turret_Rects[i].h))){
                    //change to selected bay
                    if(selectedBayID == i){
                        if(activeTurrets[selectedBayID] != nullptr){
                            activeTurrets[selectedBayID]->deactivateButtons();
                        }
                        selectedBayID = -1;
                    }else{
                        if(selectedBayID != -1){
                            if(activeTurrets[selectedBayID] != nullptr){
                                activeTurrets[selectedBayID]->deactivateButtons();
                            }
                            base.startSwap(selectedBayID, i);
                        }
                        selectedBayID = i;
                    }
                    clickedBay = true;
                    break;
                }
            }
            /*if(!clickedBay){
                if(selectedBayID != -1){
                    activeTurrets[selectedBayID]->deactivateButtons();
                    selectedBayID = -1;
                }
            }*/
        }

        int baseState = base.update(dTime);
        if((baseState & Base::UpdateState::usHALF_SWAP) == Base::UpdateState::usHALF_SWAP){
            int _t1 = base.swapT1ID;
            int _t2 = base.swapT2ID;
            if(activeTurrets[_t1] == nullptr){
                activeTurrets[_t1] = activeTurrets[_t2];
                activeTurrets[_t2] = nullptr;
            }else if(activeTurrets[_t2] == nullptr){
                activeTurrets[_t2] = activeTurrets[_t1];
                activeTurrets[_t1] = nullptr;
            }else{
                Turret* _T1 = activeTurrets[_t1];
                activeTurrets[_t1] = activeTurrets[_t2];
                activeTurrets[_t2] = _T1;
            }

            if(activeTurrets[_t1] != nullptr){
                activeTurrets[_t1]->moveTurret(base.turret_Rects[_t1].x, base.turret_Rects[_t1].y);
            }
            if(activeTurrets[_t2] != nullptr){
                activeTurrets[_t2]->moveTurret(base.turret_Rects[_t2].x, base.turret_Rects[_t2].y);
            }
        }
        if((baseState & Base::UpdateState::usSWAP_FINISH) == Base::UpdateState::usSWAP_FINISH){
            selectedBayID = -1;
        }
        if((baseState & Base::UpdateState::usFINISH_UPGRADE) == Base::UpdateState::usFINISH_UPGRADE){
            statUpgF(*(base.turretBays[0]->stats[base.upgStat]), false);
        }

        drawBase();

        for(int i = 0; i < 21; i++){
            if(activeTurrets[i] != nullptr){
                Turret& t = *(activeTurrets[i]);
                drawTexture(turretBaseID, t.baseRect);
                int tcID = int(t.type);
                drawTurret(turretCannonIDs[tcID], t.cannonRect, t.angle, t.pivotOffset, t.range + (t.stats[Turret::tsRANGE]->stat * 10), t.swapScale);

                if(!paused){
                    t.autoFireTicker -= dTime;
                    if((t.autoFireTicker <= 0) && !(gameOver)){
                        if((t.targetID != -1) && (Enemy::checkID(t.targetID) == true)){
                            fireTurret(i);
                            t.autoFireTicker = t.autoFireDelay * (1.0 / t.stats[Turret::tsAUTOFIRE]->stat);
                        }else{
                            targetEnemy(i);
                        }
                    }
                }
            }
        }

        if((baseState & Base::UpdateState::usUPGRADE) == Base::UpdateState::usUPGRADE){
            double _progress = base.upgradeTicker / base.currentUpgradeTime;
            SDL_Rect progressRect = base.turret_Rects[0];
            progressRect.h = int(double(progressRect.h) * _progress);
            drawTexture(hpBarID, progressRect);
        }

        if((selectedBayID != -1) && !(base.isSwapping) && !(base.isUpgrading)){
            if(base.turretBays[selectedBayID] != nullptr){
                if(!(activeTurrets[selectedBayID]->buttonsActive)){
                    activeTurrets[selectedBayID]->activateButtons();
                    for(int i = 0; i < 3; i++){
                        double _angle = ((120.0 * i) + 90) * _pi_over_180;
                        double _x = base.turret_Rects[selectedBayID].x + (base.turret_Rects[selectedBayID].w / 2) - 40 + ((base.turret_Rects[selectedBayID].w) * std::cos(_angle));
                        double _y = base.turret_Rects[selectedBayID].y + (base.turret_Rects[selectedBayID].h / 2) - 16 + ((base.turret_Rects[selectedBayID].h) * std::sin(_angle));
                        SDL_Rect _dest = {int(_x), int(_y), 80, 16};
                        Stat* s = activeTurrets[selectedBayID]->stats[i];
                        s->moveStat(_dest);
                        _x = base.turret_Rects[selectedBayID].x - (base.turret_Rects[selectedBayID].w / 2);
                        _y = base.turret_Rects[selectedBayID].y - (base.turret_Rects[selectedBayID].h / 2);
                        levelToScreen(_x, _y);
                        _dest.x = _x + ((base.turret_Rects[selectedBayID].w * viewScale) * std::cos(_angle)) - 20;
                        _dest.y = _y + ((base.turret_Rects[selectedBayID].h * viewScale) * std::sin(_angle)) - 8;
                        activeTurrets[selectedBayID]->statButtons[i]->moveButton(_dest);
                    }
                }
                SDL_SetRenderTarget(renderer, LevelSurface);
                for(int i = 0; i < 3; i++){
                    Stat* s = activeTurrets[selectedBayID]->stats[i];
                    statUpgF(*s, true);
                    drawTexture(s->btnID, s->btnTexRect);
                    drawTexture(base.turretBaseTexID, s->iconRect);
                    drawTexture(s->iconID, s->iconRect);
                    int txw, txh;
                    SDL_QueryTexture(s->textTex, NULL, NULL, &txw, &txh);
                    s->texRect.w = txw * 0.8;
                    s->texRect.h = txh;
                    SDL_RenderCopy(renderer, s->textTex, NULL, &(s->texRect));
                }
                SDL_SetRenderTarget(renderer, LevelSurface);
            }else{
                //
            }
        }


        if(!paused){
            if(spawnTicker > 0.0){
                spawnTicker -= dTime;
            }else{
                if(int(enemies.size()) < (nextWaveReq - nextWaveCounter)){
                    int _randEnemyAmt = (rand() % ((currentWave / 5) + 1)) - 1;
                    if(_randEnemyAmt < 0){
                        _randEnemyAmt = 0;
                    }
                    for(int i = 0; i <= _randEnemyAmt; i++){
                        spawnNewEnemy();
                    }
                    spawnTicker = spawnDelay / statMap["SpawnRate"]->stat;
                }
            }

            if((lClick) && !(gameOver)){
                Turret* t = nullptr;
                while(cycleTurrets(&t, activeTurrets)){
                    if(t != nullptr){
                        if((t->targetID == -1) || (Enemy::checkID(t->targetID) == false)){
                            targetEnemy(t);
                        }else{
                            fireTurret(t);
                        }
                    }else{
                        //std::cout << "Null turret\n";
                    }
                }
            }
        }


        for(std::vector<Enemy*>::iterator it = enemies.begin(); it != enemies.end();){
            if((*it) == nullptr){
                it++;
                break;
            }
            Enemy& E = *(*it);
            if(E.dying){
                updateCoins(E.coinDrop * (1.0 + (statMap["CoinDrop"]->stat - 1)));
                for(int i = 0; i < 21; i++){
                    if(activeTurrets[i] != nullptr){
                        Turret& t = *(activeTurrets[i]);
                        if(&E == t.target){
                            t.target = nullptr;
                        }
                    }
                }
                if(E.type == Enemy::Types::tBoss){
                    newTurret(Turret::turretType::tAutoGun);
                }
                Enemy::clearID(E.id);
                delete (*it);
                it = enemies.erase(it);
                updateWaves();
            }else{
                if((!gameOver) && (!paused)){
                    E.update(dTime, statMap["EnemySpeed"]->stat);
                    if(E.isAttacking){
                        double _angle = E.walkingAngleR + (M_PI / 2.0);
                        double _x = E.x - 32 + (E.h * std::cos(_angle));
                        double _y = E.y - 32 + (E.h * std::sin(_angle));
                        SDL_Rect pos = {int(_x), int(_y), 64, 64};
                        SDL_Point offset = {0,0};
                        animInstances.push_back(spawnAnim(impactAnim, pos, 0.0, offset, false, 0.5));
                        towerHp -= E.dmg;
                        if(towerHp <= 0){
                            gameOver = true;
                        }
                    }
                }

                Turret* t = nullptr;
                while(cycleTurrets(&t)){
                    if(t != nullptr){
                        if(t->target != nullptr){
                            if(&E == t->target){
                                DrawCircle(renderer, E.x + (E.w / 2.0), E.y + (E.h / 2.0), E.w);
                                break;
                            }
                        }
                    }
                }

                SDL_Rect enemyRect = {int(E.x), int(E.y), int(E.w), int(E.h)};
                //double _angle = 360.0 - ViewAngle;
                int _texID = -1;
                _texID = walkAnimIDs[int(E.facingDir)];
                if(_texID != -1){
                    SDL_Rect tR = {E.animInst->currentFrame * E.animInst->w, 0, 32, 32};
                    if(textures[_texID] != nullptr){
                        SDL_RenderCopy(renderer, textures[_texID], &tR, &enemyRect);
                    }else{
                        std::cout << "Null Texture\n";
                    }
                    SDL_Rect hpRect = enemyRect;
                    hpRect.y -= 8;
                    hpRect.h = 4;
                    hpRect.w = int(hpRect.w * double(double(E.hp) / double(E.maxHp)));
                    drawTexture(hpBarID, hpRect);
                }else{
                    std::cout << "Invalid texID\n";
                }
                it++;
            }
        }

        for(std::vector<SpriteAnimInst*>::iterator it = spriteAnimInstances.begin(); it != spriteAnimInstances.end();){
            SpriteAnimInst& s = *(*it);
            SDL_Rect _sXY = {s.w * s.currentFrame, 0, s.w, s.h};
            SDL_RenderCopyEx(renderer, s.anim->frames[0], &_sXY, &s.pos, s.angle, &s.offset, SDL_FLIP_NONE);

            if(!paused){
                if(s.update(dTime)){
                    delete (*it);
                    it = spriteAnimInstances.erase(it);
                }else{
                    it++;
                }
            }
        }

        for(std::vector<AnimInst*>::iterator it = animInstances.begin(); it != animInstances.end();){
            AnimInst& A = *(*it);
            SDL_RenderCopyEx(renderer, A.anim->frames[A.currentFrame], NULL, &A.pos, A.angle, &A.offset, SDL_FLIP_NONE);

            if(!paused){
                if(A.update(dTime)){
                    delete (*it);
                    it = animInstances.erase(it);
                }else{
                    it++;
                }
            }
        }

        SDL_SetRenderTarget(renderer, ScrSurface);

        drawUpgBtns();

        SDL_Rect towerhpRect = {int(SCR_W / 2.0) - 129, 39, 258, 18};
        drawTexture(hpBarBgID, towerhpRect);
        towerhpRect = {int(SCR_W / 2.0) - 128, 40, int(256 * double(double(towerHp) / double(towerMaxHp))), 16};
        drawTexture(hpBarID, towerhpRect);

        int txw, txh;
        drawTexture(CoinTxID, CoinIconRect);
        SDL_QueryTexture(CoinTextTx, NULL, NULL, &txw, &txh);
        CoinTextRect.w = txw;
        CoinTextRect.h = txh;
        SDL_RenderCopy(renderer, CoinTextTx, NULL, &CoinTextRect);

        SDL_QueryTexture(WaveTextTx, NULL, NULL, &txw, &txh);
        WaveTextRect.x = (int((SCR_W - txw) / 2.0));
        WaveTextRect.w = txw;
        WaveTextRect.h = txh;
        SDL_RenderCopy(renderer, WaveTextTx, NULL, &WaveTextRect);

        if(gameOver){
            btnsActive = false;
            SDL_Rect splashRect = {0, 0, SCR_W, SCR_H};
            drawTexture(diedSplashID, splashRect);
        }
    }

    if(paused){
        SDL_Rect _scrRect = {0, 0, SCR_W, SCR_H};
        SDL_RenderCopy(renderer, textures[pausedSplashID], NULL, &_scrRect);
    }

    SDL_SetRenderTarget(renderer, 0);
    SDL_Point _p = {int(LEVEL_W / 2.0), int(LEVEL_H / 2.0)};
    SDL_Rect scaledViewRect = ViewRect;
    scaledViewRect.w *= viewScale;
    scaledViewRect.h *= viewScale;
    scaledViewRect.x = _p.x - (scaledViewRect.w / 2.0);
    scaledViewRect.y = _p.y - (scaledViewRect.h / 2.0);
    SDL_RenderCopyEx(renderer, LevelSurface, &scaledViewRect, &ScrnRect, ViewAngle, NULL, SDL_FLIP_NONE);
    SDL_RenderCopyEx(renderer, ScrSurface, NULL, &ScrnRect, 0.0, NULL, SDL_FLIP_NONE);
    SDL_RenderPresent(renderer);

    return false;
}

void Game::setRenderer(SDL_Renderer* _renderer, SDL_Texture* _ScrSurface, SDL_Texture* _LevelSurface){
    renderer = _renderer;
    ScrSurface = _ScrSurface;
    LevelSurface = _LevelSurface;
}

void Game::updateState(GState newState){
    state = newState;
}

int main(int argc, char* argv[]){
    std::srand(time(nullptr));
    SDL_Window* window = NULL;
    SDL_Texture* ScrSurface;
    SDL_Texture* LevelSurface;
    SDL_Renderer* renderer;

    bool initSuccess = true;
    if(PLAY_TEST){
        FULLSCREEN = true;
    }
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        std::cout << "Error - Init Video\n";
        initSuccess = false;
    }else if(TTF_Init() == -1){
        std::cout << "Error - Init TTF\n";
        initSuccess = false;
    }else{
        window = SDL_CreateWindow("TD", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCR_W, SCR_H, SDL_WINDOW_SHOWN);
        if(window == NULL){
            std::cout << "Error - Create window\n";
            initSuccess = false;
        }else{
            if(FULLSCREEN){
                SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
            }
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
            ScrSurface = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, SCR_W, SCR_H);
            SDL_SetTextureBlendMode(ScrSurface, SDL_BLENDMODE_BLEND);
            LevelSurface = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, LEVEL_W, LEVEL_H);
            SDL_SetTextureBlendMode(LevelSurface, SDL_BLENDMODE_BLEND);
        }
    }

    if(initSuccess){
        AspectRatio aspect;
        SDL_DisplayMode dm;
        if(SDL_GetCurrentDisplayMode(0, &dm) >= 0){
            double div = double(dm.w) / double(dm.h);
            if(div > (1.7) && div < (1.8)){
                aspect = r16x9;
            }else if(div == (1.5)){
                aspect = r3x2;
            }
        }
        game.setRenderer(renderer, ScrSurface, LevelSurface);
        bool quit = false;

        Uint64 dTimeNow = SDL_GetPerformanceCounter();
        Uint64 dTimePrev = 0;
        double deltaTime = 0;
        time_t startTime = time(NULL);
        double perfFrequency =(double)SDL_GetPerformanceFrequency();
        while(!quit){
            dTimePrev = dTimeNow;
            dTimeNow = SDL_GetPerformanceCounter();
            deltaTime = (double)((dTimeNow - dTimePrev) * 1000 / perfFrequency);

            quit = game.update(deltaTime);
        }// !quit
        time_t timeNow = time(NULL);

        if(PLAY_TEST){
            std::fstream playTestTimeFile;
            playTestTimeFile.open(getPath() + "playTestTime.sav");
            std::stringstream ss;
            std::vector<std::string> lines;
            if(playTestTimeFile.is_open()){
                std::string currentLine;
                while(std::getline(playTestTimeFile, currentLine)){
                    if(currentLine != ""){
                        int totalRunTime = std::stoi(currentLine);
                        ss.str("");
                        ss << totalRunTime << "\n";
                        lines.push_back(ss.str());
                    }
                }
            }
            playTestTimeFile.close();
            ss.str("");
            ss << (timeNow - startTime);
            lines.push_back(ss.str());
            int timeDif = std::stoi(ss.str());
            ss.str("");
            int totalTimeDif = std::stoi(lines[0]);
            if(int(lines.size()) != 1){
                totalTimeDif += timeDif;
            }
            ss << totalTimeDif << "\n";
            lines[0] = ss.str();
            playTestTimeFile.open(getPath() + "playTestTime.sav", std::ios::out);
            for(std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); it++){
                playTestTimeFile << (*it);
            }
            std::cout << "Total Test time: " << totalTimeDif << " seconds.\nLast Test time: " << timeDif << " seconds\n";
            playTestTimeFile.close();
        }
    }//initsuccess
    if(HOLD_CONSOLE_ON_EXIT){
        std::cout << "Enter to close.\n";
        std::cin.get();
    }
    SDL_DestroyWindow(window);
    SDL_DestroyTexture(ScrSurface);
    SDL_DestroyTexture(LevelSurface);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
    return 0;
}
