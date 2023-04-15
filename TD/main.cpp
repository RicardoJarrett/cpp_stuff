#define SDL_MAIN_HANDLED
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include <iostream>
#include <fstream>
#include <Windows.h>
#include <stdlib.h>
#include <string>
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
bool FULLSCREEN = false;

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
        //error
        return nullptr;
    }else{
        retTex = SDL_CreateTextureFromSurface(renderer, tmpSurface);
        if(retTex == NULL){
            //error
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
            std::cout << "Could not create texture from surface\n";
        }
        SDL_FreeSurface(textSurf);
    }
    return retTx;
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
    };
    SDL_Rect area;
    void (*clickFuncPtr)();
    void (Game::*clickMFuncPtr)();
    void (Game::*clickStatFuncPtr)(Stat& _stat);
    double bounce;
    type T;
    Game* gamePtr;
    Stat* upgStat;
public:
    Button(SDL_Rect _area, void (*F)());
    Button(SDL_Rect _area, void (Game::*F)(), Game* _game);
    Button(SDL_Rect _area, void (Game::*F)(Stat& _stat), Game* _game, Stat* _stat);
    bool checkClick(int x, int y);
    bool checkOver(int x, int y);
    void update(double dTime);
};

Button::Button(SDL_Rect _area, void (*F)()) : area(_area), clickFuncPtr(F){
    bounce = 0.0;
    T = func;
    gamePtr = nullptr;
    upgStat = nullptr;
}

Button::Button(SDL_Rect _area, void (Game::*F)(), Game* _game) : clickMFuncPtr(F), gamePtr(_game){
    bounce = 0.0;
    T = memberfunc;
    upgStat = nullptr;
}

Button::Button(SDL_Rect _area, void (Game::*F)(Stat& _stat), Game* _game, Stat* _stat) : area(_area), clickStatFuncPtr(F), gamePtr(_game), upgStat(_stat){
    bounce = 0.0;
    T = statfunc;
}

bool Button::checkClick(int x, int y){
    if((bounce == 0.0) && (checkOver(x, y))){
        switch(T){
            case func:
                clickFuncPtr();
                break;
            case statfunc:
                (gamePtr->*clickStatFuncPtr)(*upgStat);
                break;
            case memberfunc:
                (gamePtr->*clickMFuncPtr)();
                break;
        }
        bounce = 200.0;
        return true;
    }
    return false;
}

bool Button::checkOver(int x, int y){
    if((x > area.x) && (x < (area.x + area.w)) && (y > area.y) && (y < (area.y + area.h))){
        return true;
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
    bool isLooping;

    SDL_Rect pos;
    SDL_Point offset;
    double angle;
    double frameTicker, frameDelay;

    AnimInst(Anim* _anim, SDL_Rect _pos, double _angle, SDL_Point _offset, bool _loop, double _speed);
    virtual bool update(double dTime);
};

AnimInst::AnimInst(Anim* _anim, SDL_Rect _pos, double _angle, SDL_Point _offset, bool _loop, double _speed) : pos(_pos), angle(_angle), offset(_offset), isLooping(_loop), anim(_anim){
    currentFrame = 0;
    frameTicker = 0;
    frameDelay = 1000.0 / (_anim->frameRate * _speed);
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
};

SpriteAnimInst::SpriteAnimInst(Anim* _anim, SDL_Rect _pos, int _w, int _h, double _angle, SDL_Point _offset, bool _loop, double _speed) : w(_w), h(_h), AnimInst(_anim, _pos, _angle, _offset, _loop, _speed){
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
    ids.push(_id);
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
        int dx = (LEVEL_W / 2.0) - (x + (w / 4.0));
        int dy = (LEVEL_H / 2.0) - (y + (h / 4.0));
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
    btnTexRect = _rect;
    texRect = {btnTexRect.x + 16, btnTexRect.y + 2, 60, 12};
    iconRect = {btnTexRect.x + 2, btnTexRect.y + 2, 12, 12};
}
Stat::Stat(int x, int y, int w, int h) : Stat((SDL_Rect){x, y, w, h}){
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
    turretType type;
    double angle;
    double range;
    double autoFireDelay;
    double autoFireTicker;

    Enemy* target;
    int targetID;
    Anim* muzzleAnim;

    SDL_Rect baseRect;
    SDL_Rect cannonRect;
    SDL_Point pivotOffset;

    Turret(double _x, double _y, turretType _type, double _fireDelay);
    ~Turret();
    void setAngle(double _angle);
    void setTarget(Enemy* _target);
    void moveTurret(int _x, int _y);
};

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
}

Turret::Turret(double _x, double _y, turretType _type, double _fireDelay=500.0) : type(_type), autoFireDelay(_fireDelay){
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
}

class Game{
    SDL_Renderer* renderer;
    SDL_Texture* ScrSurface;
    SDL_Texture* LevelSurface;
    uint32_t bgCol;
    SDL_Rect ScrnRect;
    SDL_Rect LevelRect;
    double ViewAngle;
    SDL_Rect ViewRect;
    double viewScale;

    TTF_Font* font;
    std::vector<std::pair<int, std::string>> hiScores;
    SDL_Texture* hiscoreTexs[4];

    GState state;
    bool btnsActive;
    bool gameOver;

    int fontSize;
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

    int redEnemyID;
    int blueEnemyID;
    int greenEnemyID;

    int hpBarID;
    int hpBarBgID;
    int turretBaseID;
    int turretCannonIDs[3];
    //double turretAngle;
    //Enemy* targettedEnemy;
    //double baseRange;

    Anim* muzzleAnim;
    int muzzleFlashID;
    Anim* impactAnim;
    int impactAnimIDs[4];
    Anim* walkAnims[8];
    int walkAnimIDs[8];

    int circleFenceID;

    int statBtnID;

    int coins;
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

    //SDL_Rect turretBaseRect;
    //SDL_Rect turretCannonRect;
    //SDL_Point turretPivotOffset;

    SDL_Rect circleFenceRect;

    std::vector<Button*> buttons;
    std::vector<SDL_Texture*> textures;
    std::map<std::string, int> texIDs;
    std::vector<Enemy*> enemies;
    std::vector<Anim*> anims;
    std::vector<AnimInst*> animInstances;
    std::vector<SpriteAnimInst*> spriteAnimInstances;

    std::map<std::string, Stat*> statMap;
    std::vector<Stat*> stats;
    std::vector<Turret*> turrets;

    int MAX_ENEMIES;
    double spawnDelay;
    double spawnTicker;
    double fireDelay;
    double fireTicker;
    //double autoFireDelay;
    //double autoFireTicker;

    void initStats();

    void targetEnemy(int _tid);
    void fireTurret(int _turretID);

    void statUpgF(Stat& _stat);
    void drawUpgBtns();

    void newStatBtn(Stat* _stat, int _texID);
    void newStatBtn(Stat* _stat, std::string _iconPath);

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
    void updateState(GState newState);
    int newTexture(std::string _path);
    void drawTexture(int _id, SDL_Rect _dest);
    void drawTexture(int _id, SDL_Rect _dest, double _angle);
    void drawTurret(int _id, SDL_Rect _dest, double _angle, SDL_Point _offset, double _range);
    int spawnNewEnemy();
    AnimInst* spawnAnim(Anim* _anim, SDL_Rect _pos, double _angle, SDL_Point _offset, bool _loop, double _speed);
    SpriteAnimInst* spawnSpriteAnim(Anim* _anim, SDL_Rect _pos, int _w, int _h, double _angle, SDL_Point _offset, bool _loop, double _speed);
};

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

    logoRect = {(SCR_W / 2.0) - (SCR_W / 4.0), (SCR_H / 8.0), (SCR_W / 2.0), (SCR_H / 4.0)};
    ngbRect = {(SCR_W / 2.0) - 128, (SCR_H / 4.0) * 3.0, 256, 64};
    nextWaveRect = {(SCR_W / 2.0) - 64, SCR_H / 0.75, 128, 32};

    //turretBaseRect = {int(LEVEL_W / 2.0) - 32, int(LEVEL_H / 2.0) - 32, 64, 64};
    //turretCannonRect = {turretBaseRect.x + 20, turretBaseRect.y - 20, 24, 64};
    //turretPivotOffset = {12, 52};

    SDL_GetMouseState(&mouseX, &mouseY);

    MAX_ENEMIES = 100;

    //baseRange = 220.0;
    circleFenceRect = {int(LEVEL_W / 2.0) - 128, int(LEVEL_H / 2.0) - 128, 256, 256};

    fontSize = 8;
    renderer = nullptr;

    CoinIconRect = {int(SCR_W / 8.0), 10, 32, 32};
    CoinTextRect = {int(SCR_W / 8.0) + 32, 12, 64, 32};

    WaveTextRect = {int((SCR_W / 2.0)), 10, 64, 32};
    btnsActive = true;
    tick = false;
    font = nullptr;
    logoID = -1;
    ngbID = -1;
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
    hiscoreTexs[0] = renderText("HiScores", {0,0,0}, renderer, font);
    for(int i = 0; i < 3; i++){
        std::string tStr = hiScores[i].second + " : "  + std::to_string(hiScores[i].first);
        hiscoreTexs[i+1] = renderText(tStr, {0,0,0}, renderer, font);
    }
}

void Game::init(){
    btnsActive = true;
    tick = false;
    font = nullptr;
    gameOver = false;

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

    circleFenceID = -1;
    statBtnID = -1;

    coins = 0;
    CoinTxID = -1;
    CoinTextTx = nullptr;

    currentWave = 1;
    nextWaveCounter = 0;
    nextWaveReq = 5;
    WaveTextTx = nullptr;

    initStats();
    font = TTF_OpenFont("slkscr.ttf", fontSize);
    loadHiscores();
}

void Game::initStats(){
    std::list<std::string> statNames = {"Dmg", "CoinDrop", "SpawnRate", "EnemySpeed", "AutofireRate", "fireRange", "maxEnemies", "regenHP"};
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
    for(std::vector<Turret*>::iterator it = turrets.begin(); it != turrets.end(); it++){
        delete (*it);
    }
    turrets.clear();
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

void Game::targetEnemy(int _tid){
    double lowestDist = turrets[_tid]->range + (10 * statMap["fireRange"]->stat);
    for(std::vector<Enemy*>::iterator it = enemies.begin(); it != enemies.end(); it++){
        Enemy& E = (*(*it));
        double xd = (E.x + (E.w / 2.0)) - (turrets[_tid]->baseRect.x + (turrets[_tid]->baseRect.w / 2.0));
        double yd = (turrets[_tid]->baseRect.y + (turrets[_tid]->baseRect.h / 2.0)) - (E.y + (E.h / 2.0));
        double dist = std::abs(sqrt((xd*xd)+(yd*yd)));
        if(dist < lowestDist){
            lowestDist = dist;
            double newAngle = (std::atan2(-yd, xd) * (180.0 / M_PI)) + 90.0;
            turrets[_tid]->angle = newAngle;
            turrets[_tid]->target = (*it);
            turrets[_tid]->targetID = (*it)->id;
        }
    }
}

void printDir(Dir _dir){
    switch(_dir){
        case Dir::dU:
            std::cout << "Up\n";
            break;
        case Dir::dUR:
            std::cout << "Up - RIGHT\n";
            break;
        case Dir::dR:
            std::cout << "RIGHT\n";
            break;
        case Dir::dDR:
            std::cout << "DOWN - RIGHT\n";
            break;
        case Dir::dD:
            std::cout << "DOWN\n";
            break;
        case Dir::dDL:
            std::cout << "DOWN - LEFT\n";
            break;
        case Dir::dL:
            std::cout << "LEFT\n";
            break;
        case Dir::dUL:
            std::cout << "Up - LEFT\n";
            break;
    }
}

double nAngle = -90.0;
bool ENEMY_SPAWN_TEST_ANGLE = false;
int Game::spawnNewEnemy(){
    if(int(enemies.size()) < (10 + (statMap["maxEnemies"]->stat * 5)) && (int(enemies.size()) < MAX_ENEMIES)){
        Enemy& E = *(new Enemy());
        E.id = Enemy::getID();
        E.maxHp = 5 * (1.0 + ((currentWave - 1) / 2.0));
        E.hp = 5 * (1.0 + ((currentWave - 1) / 2.0));
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
        E.animInst = spawnSpriteAnim(walkAnims[int(E.facingDir)], SDL_Rect{E.x, E.y, E.w, E.h}, E.w, E.h, NULL, SDL_Point{0,0}, true, 1.0);
        E.spd = 32.0;
        E.coinDrop = 1 + (currentWave / 2.0);
        E.atkDelay = 1000.0;
        E.dmg = 1 + (currentWave / 2.5);
        E.aspd = 0.5;

        if((currentWave % 10) == 0){
            E.type = Enemy::Types::tBoss;
            E.w *= 8;
            E.h *= 8;
            E.maxHp *= 25;
            E.hp *= 25;
            E.spd *= 0.5;
            E.dmg *= 10;
            E.coinDrop *= 50;
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
    font = nullptr;
    muzzleAnim = nullptr;
    impactAnim = nullptr;
    CoinTextTx = nullptr;
    WaveTextTx = nullptr;
}

Game game;

void Game::newStatBtn(Stat* _stat, int _texID){
    _stat->btnID = statBtnID;
    buttons.push_back(new Button(_stat->btnTexRect, &statUpgF, this, _stat));
    _stat->textTex = renderText(" " + std::to_string(_stat->level) + " : " + std::to_string(_stat->upgradeCost), {0,0,0}, renderer, font);
    _stat->iconID = _texID;
}

void Game::newStatBtn(Stat* _stat, std::string _iconPath){
    _stat->iconID = newTexture(_iconPath);
    newStatBtn(_stat, _stat->iconID);
}

void Game::updateCoins(int _coins){
    coins += _coins;
    CoinTextTx = renderText(std::to_string(coins), {0,0,0}, renderer, font);
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
    WaveTextTx = renderText(tStr, {0,0,0}, renderer, font);
}

void Game::nextWaveBtnF(){
    nextWaveCounter = 0;
    std::string tStr = "Wave " + std::to_string(currentWave) + " (" + std::to_string(nextWaveCounter) + " / " + std::to_string(nextWaveReq) + ")";
    WaveTextTx = renderText(tStr, {0,0,0}, renderer, font);
}

void newGameBtn(){
    std::cout << "New Game\n";
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
        SDL_RenderCopy(renderer, textures[_id], NULL, &_dest);
    }
}

void Game::drawTexture(int _id, SDL_Rect _dest, double _angle){
    if(_id < int(textures.size())){
        SDL_RenderCopyEx(renderer, textures[_id], NULL, &_dest, _angle, NULL, SDL_FLIP_NONE);
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

void Game::drawTurret(int _id, SDL_Rect _dest, double _angle, SDL_Point _offset, double _range){
    if(_id < int(textures.size())){
        SDL_RenderCopyEx(renderer, textures[_id], NULL, &_dest, _angle, &_offset, SDL_FLIP_NONE);
    }
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    DrawCircle(renderer, _dest.x + (_dest.w / 2.0), _dest.y + 20 + (_dest.h / 2.0), _range + (statMap["fireRange"]->stat * 10));
}

void Game::statUpgF(Stat& _stat){
    if(coins >= _stat.upgradeCost){
        updateCoins(-_stat.upgradeCost);
        _stat.upgradeCost *= 1.3;
        SDL_DestroyTexture(_stat.textTex);
        _stat.level++;
        _stat.stat++;
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
        _stat.textTex = renderText(_tLvl + ": " + _tUpgC, {0,0,0}, renderer, font);
    }
}

void Game::drawUpgBtns(){
    for(std::map<std::string, Stat*>::iterator it = statMap.begin(); it != statMap.end(); it++){
        drawTexture((*it).second->btnID, (*it).second->btnTexRect);
        drawTexture((*it).second->iconID, (*it).second->iconRect);
        int txw, txh;
        SDL_QueryTexture((*it).second->textTex, NULL, NULL, &txw, &txh);
        (*it).second->texRect.w = txw * 1.5;
        (*it).second->texRect.h = txh * 1.5;
        SDL_RenderCopy(renderer, (*it).second->textTex, NULL, &((*it).second->texRect));
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
    if((turrets[_turretID]->target != nullptr) && (Enemy::checkID(turrets[_turretID]->targetID) == true)){
        if(turrets[_turretID]->target->takeDamage(statMap["Dmg"]->stat)){
            turrets[_turretID]->target = nullptr;
        }
        SDL_Rect pos = {turrets[_turretID]->cannonRect.x - 4, turrets[_turretID]->cannonRect.y - 32, 32, 32};
        SDL_Point offset = turrets[_turretID]->pivotOffset;
        offset.x += 4;
        offset.y += 32;
        animInstances.push_back(spawnAnim(muzzleAnim, pos, turrets[_turretID]->angle, offset, false, 1.0));
    }
}

int healDelay = 10000.0;
int healTicker = healDelay;
bool Game::update(double dTime){
    SDL_SetRenderDrawColor(renderer, 240, 240, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, LevelSurface);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
    SDL_SetRenderTarget(renderer, ScrSurface);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 240, 240, 0, 255);

    SDL_Event e;

    fpsTicker -= dTime;
    if(clickBounce > 0.0){
        clickBounce -= dTime;
    }
    tick = false;
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

    if(state == gsInit){
        destroyButtons();
        destroyEnemies();
        destroyTextures();
        destroyStats();
        destroyTurrets();
        init();
        logoID = newTexture("TowerDefence.png");
        ngbID = newTexture("NewGame.png");
        buttons.push_back(new Button(ngbRect, newGameBtn));
        towerHp = 250;
        towerMaxHp = 250;
        updateState(gsMainMenu);
    }else if(state == gsMainMenu){
        SDL_SetRenderTarget(renderer, ScrSurface);
        drawTexture(logoID, logoRect);
        drawTexture(ngbID, ngbRect);

        SDL_Rect scoreRect = {(SCR_W / 2.0) - (SCR_W / 8.0), (SCR_H / 16.0) * 7.0, SCR_W / 4.0, (SCR_H / 32.0)};
        SDL_RenderCopy(renderer, hiscoreTexs[0], NULL, &scoreRect);
        scoreRect.y = (SCR_H / 16.0) * 8.0;
        SDL_RenderCopy(renderer, hiscoreTexs[1], NULL, &scoreRect);
        scoreRect.y = (SCR_H / 16.0) * 9.0;
        scoreRect.x += (scoreRect.w * 0.1);
        scoreRect.w *= 0.8;
        scoreRect.h *= 0.9;
        SDL_RenderCopy(renderer, hiscoreTexs[2], NULL, &scoreRect);
        scoreRect.y = (SCR_H / 16.0) * 10.0;
        scoreRect.x += (scoreRect.w * 0.1);
        scoreRect.w *= 0.8;
        scoreRect.h *= 0.9;
        SDL_RenderCopy(renderer, hiscoreTexs[3], NULL, &scoreRect);
        SDL_SetRenderTarget(renderer, 0);
    }else if(state == gsGameStart){
        std::string path = getPath();
        nextWaveBtnID = newTexture("nextWave.png");
        diedSplashID = newTexture("diedSplash.png");

        redEnemyID = newTexture("redEnemy.png");
        blueEnemyID = newTexture("blueEnemy.png");
        greenEnemyID = newTexture("greenEnemy.png");

        hpBarID = newTexture("hpBar.png");
        hpBarBgID = newTexture("hpBar_bg.png");
        Enemy::initEnemies(MAX_ENEMIES);
        turretBaseID = newTexture("td_basic_towers\\Tower.png");
        turretCannonIDs[0] = newTexture("td_basic_towers\\MG3.png");
        turretCannonIDs[1] = newTexture("td_basic_towers\\Missile_Launcher.png");
        turretCannonIDs[2] = newTexture("td_basic_towers\\Cannon.png");
        circleFenceID = newTexture("circle_fence.png");
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
        WaveTextTx = renderText("Wave " + std::to_string(currentWave) + " (" + std::to_string(nextWaveCounter) + " / " + std::to_string(nextWaveReq) + ")", {0,0,0}, renderer, font);

        statBtnID = newTexture("button_turq.png");

        newStatBtn(statMap["Dmg"], "bullet.png");
        newStatBtn(statMap["CoinDrop"], CoinTxID);
        newStatBtn(statMap["SpawnRate"], "Hourglass.png");
        newStatBtn(statMap["EnemySpeed"], "speedIcon.png");
        newStatBtn(statMap["AutofireRate"], "autofireIcon.png");
        newStatBtn(statMap["fireRange"], "rangeIcon.png");
        newStatBtn(statMap["maxEnemies"], "maxEnemyIcon.png");
        newStatBtn(statMap["regenHP"], "regenHPIcon.png");

        muzzleAnim = new Anim{30.0};
        muzzleAnim->addFrameTx(textures[muzzleFlashID]);
        muzzleAnim->addFrameTx(textures[muzzleFlashID]);

        impactAnim = new Anim{30.0};
        impactAnim->addFrameTx(textures[impactAnimIDs[0]]);
        impactAnim->addFrameTx(textures[impactAnimIDs[1]]);
        impactAnim->addFrameTx(textures[impactAnimIDs[2]]);
        impactAnim->addFrameTx(textures[impactAnimIDs[3]]);

        for(int i = 0; i < 8; i++){
            walkAnims[i] = new Anim(30.0);
            walkAnims[i]->addFrameTx(textures[walkAnimIDs[i]]);
            walkAnims[i]->addFrameTx(textures[walkAnimIDs[i]]);
        }

        int _turretCount = 1;
        Turret* t;
        if(_turretCount < 2){
            t = new Turret(int(LEVEL_W / 2.0) - 32, int(LEVEL_H / 2.0) - 32, Turret::turretType::tAutoGun);
            turrets.push_back(t);
        }else{
            int _radius = 64;
            for(int i = 0; i < _turretCount; i++){
                double _angle;
                if(_turretCount % 2){
                    _angle = ((360.0 / _turretCount) * (i + 1)) * _pi_over_180;
                }else{
                    _angle = (((360.0 / _turretCount) * (i + 1)) - 90) * _pi_over_180;
                }
                double _x = ((LEVEL_W / 2.0) + (_radius * std::cos(_angle))) - 32;
                double _y = ((LEVEL_H / 2.0) + (_radius * std::sin(_angle))) - 32;
                t = new Turret(_x, _y, static_cast<Turret::turretType>(i%3));
                turrets.push_back(t);
            }
        }
        t = nullptr;

        updateState(gsGame);
    }else if(state == gsGame){
        SDL_SetRenderTarget(renderer, LevelSurface);
        int _tid = 0;

        for(std::vector<Turret*>::iterator it = turrets.begin(); it != turrets.end(); it++, _tid++){
            drawTexture(turretBaseID, (*it)->baseRect);
            int tcID = 0;
            switch((*it)->type){
                case Turret::turretType::tRocket:
                    tcID = 1;
                    break;
                case Turret::turretType::tLaser:
                    tcID = 2;
                    break;
                case Turret::turretType::tAutoGun:
                default:
                    tcID = 0;
                    break;
            }

            drawTurret(turretCannonIDs[tcID], (*it)->cannonRect, (*it)->angle, (*it)->pivotOffset, (*it)->range);
            (*it)->autoFireTicker -= dTime;
            if(((*it)->autoFireTicker <= 0) && !(gameOver)){
                if(((*it)->targetID != -1) && (Enemy::checkID((*it)->targetID) == true)){
                    fireTurret(_tid);
                    (*it)->autoFireTicker = (*it)->autoFireDelay * (1.0 / statMap["AutofireRate"]->stat);
                }else{
                    targetEnemy(_tid);
                }
            }
        }

        if(spawnTicker > 0.0){
            spawnTicker -= dTime;
        }else{
            if(int(enemies.size()) < (nextWaveReq - nextWaveCounter)){
                int newID = spawnNewEnemy();
                spawnTicker = spawnDelay / statMap["SpawnRate"]->stat;
            }
        }

        if((lClick) && !(gameOver)){
            if((turrets[0]->targetID == -1) || (Enemy::checkID(turrets[0]->targetID) == false)){
                targetEnemy(0);
            }else{
                fireTurret(0);
            }
        }

        drawTexture(circleFenceID, circleFenceRect);

        for(std::vector<Enemy*>::iterator it = enemies.begin(); it != enemies.end();){
            if((*it)->dying){
                updateCoins((*it)->coinDrop * (1.0 + (statMap["CoinDrop"]->stat - 1)));
                for(std::vector<Turret*>::iterator turretIt = turrets.begin(); turretIt != turrets.end(); turretIt++){
                    if((*it) == (*turretIt)->target){
                        (*turretIt)->target = nullptr;
                    }
                }
                if((*it)->type == Enemy::Types::tBoss){
                    Turret* t;
                    int _turretCount = int(turrets.size()) + 1;
                    int _radius = 64;
                    double _angle, _x, _y;
                    for(int i = 0; i < (_turretCount-1); i++){
                        _angle = (((360.0 / _turretCount) * (i + 1)) - 90) * _pi_over_180;
                        _x = ((LEVEL_W / 2.0) + (_radius * std::cos(_angle))) - 32;
                        _y = ((LEVEL_H / 2.0) + (_radius * std::sin(_angle))) - 32;
                        turrets[i]->moveTurret(_x, _y);
                    }
                    _angle = 270.0 * _pi_over_180;
                    _x = ((LEVEL_W / 2.0) + (_radius * std::cos(_angle))) - 32;
                    _y = ((LEVEL_H / 2.0) + (_radius * std::sin(_angle))) - 32;
                    t = new Turret(_x, _y, static_cast<Turret::turretType>(_turretCount%3));
                    turrets.push_back(t);
                    t = nullptr;
                }
                Enemy::clearID((*it)->id);
                delete (*it);
                it = enemies.erase(it);
                updateWaves();
            }else{
                if(!gameOver){
                    (*it)->update(dTime, statMap["EnemySpeed"]->stat);
                    if((*it)->isAttacking){
                        double _angle = (*it)->walkingAngleR + (M_PI / 2.0);
                        double _x = ((*it)->x - 32) + ((*it)->h * 2.0 * std::cos(_angle));
                        double _y = ((*it)->y - 32) + ((*it)->h * 2.0 * std::sin(_angle));
                        SDL_Rect pos = {_x, _y, 64, 64};
                        SDL_Point offset = {0,0};
                        animInstances.push_back(spawnAnim(impactAnim, pos, 0.0, offset, false, 0.5));
                        towerHp -= (*it)->dmg;
                        if(towerHp <= 0){
                            gameOver = true;
                        }
                    }
                }
                for(std::vector<Turret*>::iterator turretIt = turrets.begin(); turretIt != turrets.end(); turretIt++){
                    if((*it) == (*turretIt)->target){
                        DrawCircle(renderer, (*it)->x + ((*it)->w / 2.0), (*it)->y + ((*it)->h / 2.0), (*it)->w);
                    }
                }

                SDL_Rect enemyRect = {int((*it)->x), int((*it)->y), int((*it)->w), int((*it)->h)};
                double _angle = 360.0 - ViewAngle;
                int _texID = -1;
                _texID = walkAnimIDs[int((*it)->facingDir)];
                /*switch((*it)->type){
                    case Enemy::Types::tFast:
                        _texID = blueEnemyID;
                        break;
                    case Enemy::Types::tHeavy:
                        _texID = greenEnemyID;
                        break;
                    case Enemy::Types::tRegular:
                    default:
                        _texID = redEnemyID;
                        break;
                }*/
                SDL_Rect tR = {(*it)->animInst->currentFrame * (*it)->animInst->w, 0, 32, 32};
                SDL_RenderCopy(renderer, textures[_texID], &tR, &enemyRect);
                SDL_Rect hpRect = enemyRect;
                hpRect.y -= 8;
                hpRect.h = 4;
                hpRect.w = int(hpRect.w * double(double((*it)->hp) / double((*it)->maxHp)));
                drawTexture(hpBarID, hpRect, _angle);
                it++;
            }
        }

        for(std::vector<SpriteAnimInst*>::iterator it = spriteAnimInstances.begin(); it != spriteAnimInstances.end();){
            SDL_Rect _sXY = {(*it)->w * (*it)->currentFrame, 0, (*it)->w, (*it)->h};
            SDL_RenderCopyEx(renderer, (*it)->anim->frames[0], &_sXY, &(*it)->pos, (*it)->angle, &(*it)->offset, SDL_FLIP_NONE);

            if((*it)->update(dTime)){
                delete (*it);
                it = spriteAnimInstances.erase(it);
            }else{
                it++;
            }
        }

        for(std::vector<AnimInst*>::iterator it = animInstances.begin(); it != animInstances.end();){
            SDL_RenderCopyEx(renderer, (*it)->anim->frames[(*it)->currentFrame], NULL, &(*it)->pos, (*it)->angle, &(*it)->offset, SDL_FLIP_NONE);

            if((*it)->update(dTime)){
                delete (*it);
                it = animInstances.erase(it);
            }else{
                it++;
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
        CoinTextRect.w = txw * 3;
        CoinTextRect.h = txh * 3;
        SDL_RenderCopy(renderer, CoinTextTx, NULL, &CoinTextRect);

        SDL_QueryTexture(WaveTextTx, NULL, NULL, &txw, &txh);
        WaveTextRect.x = (int(SCR_W / 2.0) - txw);
        WaveTextRect.w = txw * 2;
        WaveTextRect.h = txh * 2;
        SDL_RenderCopy(renderer, WaveTextTx, NULL, &WaveTextRect);

        if(gameOver){
            btnsActive = false;
            SDL_Rect splashRect = {0, 0, SCR_W, SCR_H};
            drawTexture(diedSplashID, splashRect);
        }
    }

    if(btnsActive){
        for(std::vector<Button*>::iterator it = buttons.begin(); it != buttons.end(); it++){
            Button& B = *(*it);
            B.update(dTime);
            if(lClick){
                if(B.checkClick(mouseX, mouseY)){
                    break;
                }
            }
        }
    }

    SDL_SetRenderTarget(renderer, 0);
    SDL_Point _p = {LEVEL_W / 2.0, LEVEL_H / 2.0};
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
            if(FULLSCREEN == true){

            }
        }
        game.setRenderer(renderer, ScrSurface, LevelSurface);
        bool quit = false;

        Uint64 dTimeNow = SDL_GetPerformanceCounter();
        Uint64 dTimePrev = 0;
        double deltaTime = 0;

        while(!quit){
            dTimePrev = dTimeNow;
            dTimeNow = SDL_GetPerformanceCounter();
            deltaTime = (double)((dTimeNow - dTimePrev) * 1000 / (double)SDL_GetPerformanceFrequency());

            quit = game.update(deltaTime);
        }// !quit
    }//initsuccess

    SDL_DestroyWindow(window);
    SDL_DestroyTexture(ScrSurface);
    SDL_DestroyTexture(LevelSurface);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
    return 0;
}
