#define SDL_MAIN_HANDLED
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include <iostream>
#include <Windows.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <ctime>
#include <cmath>
#include <queue>

#define _countof(array) (sizeof(array) / sizeof(array[0]))

int SCR_W = 480;
int SCR_H = 360;
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

SDL_Texture* renderText(std::string _text, SDL_Color _col, SDL_Renderer* renderer, TTF_Font* _font)
{
    SDL_Texture* retTx = nullptr;
    SDL_Surface* textSurf = TTF_RenderText_Blended(_font, _text.c_str(), _col);
    if(textSurf== NULL){
        std::cout << "Could not render text: " << _text << "\n";
    }else{
        retTx = SDL_CreateTextureFromSurface(renderer, textSurf);
        if(retTx == NULL){
            std::cout << "Could not create texture from surface\n";
        }
        SDL_FreeSurface(textSurf);
    }
    return retTx;
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
    bool update(double dTime);
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

class Enemy{
public:
    enum Types{
        tRegular,
        tFast,
        tHeavy
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

    static int getID();
    static void clearID(int _id);
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
    if(!attackState){
        int dx = (SCR_W / 2.0) - x;
        int dy = (SCR_H - 48) - y;
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

class Game{
    SDL_Renderer* renderer;
    TTF_Font* font;

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
    int turretCannonID;
    double turretAngle;
    Enemy* targettedEnemy;
    double baseRange;

    Anim* muzzleAnim;
    int muzzleFlashID;
    Anim* impactAnim;
    int impactAnimIDs[4];

    int circleFenceID;

    int statBtnID;

    int coins;
    int CoinTxID;
    SDL_Rect CoinIconRect;
    SDL_Texture* CoinTextTx;
    SDL_Rect CoinTextRect;
    int towerHp;
    int towerMaxHp;

    int currentWave;
    int nextWaveCounter;
    int nextWaveReq;
    SDL_Texture* WaveTextTx;
    SDL_Rect WaveTextRect;

    SDL_Rect logoRect;
    SDL_Rect ngbRect;
    SDL_Rect nextWaveRect;

    SDL_Rect turretBaseRect;
    SDL_Rect turretCannonRect;
    SDL_Point turretPivotOffset;

    SDL_Rect circleFenceRect;

    std::vector<Button*> buttons;
    std::vector<SDL_Texture*> textures;
    std::map<std::string, int> texIDs;
    std::vector<Enemy*> enemies;
    std::vector<Anim*> anims;
    std::vector<AnimInst*> animInstances;

    std::map<std::string, Stat*> statMap;
    std::vector<Stat*> stats;

    int MAX_ENEMIES;
    double spawnDelay;
    double spawnTicker;
    double fireDelay;
    double fireTicker;
    double autoFireDelay;
    double autoFireTicker;

    void initStats();

    void targetEnemy();
    bool fireTurret();

    void statUpgF(Stat& _stat);
    void drawUpgBtns();

    void newStatBtn(Stat* _stat, int _texID);
    void newStatBtn(Stat* _stat, std::string _iconPath);

    void updateCoins(int _coins);
    void updateWaves();

    bool updateAnims();
    void nextWaveBtnF();
public:
    Game();
    ~Game();
    bool update(double dTime);
    void setRenderer(SDL_Renderer* _renderer);
    void destroyButtons();
    void destroyTextures();
    void destroyEnemies();
    void destroyStats();
    void init();
    void updateState(GState newState);
    int newTexture(std::string _path);
    void drawTexture(int _id, SDL_Rect _dest);
    void drawTurret(int _id, SDL_Rect _dest, double angle);
    int spawnNewEnemy();
    AnimInst* spawnAnim(Anim* _anim, SDL_Rect _pos, double _angle, SDL_Point _offset, bool _loop, double _speed);
};

Game::Game(){
    state = gsInit;
    fps = 30.0;
    fpsTicker = 1000.0 / fps;

    logoRect = {(SCR_W / 2.0) - 114, 30, 228, 122};
    ngbRect = {(SCR_W / 2.0) - 128, 240, 256, 64};
    nextWaveRect = {(SCR_W / 2.0) - 64, SCR_H / 0.75, 128, 32};

    turretBaseRect = {int(SCR_W / 2.0) - 32, SCR_H - 80, 64, 64};
    turretCannonRect = {turretBaseRect.x + 20, turretBaseRect.y - 20, 24, 64};

    turretPivotOffset = {12, 52};

    SDL_GetMouseState(&mouseX, &mouseY);

    MAX_ENEMIES = 100;

    baseRange = 220.0;
    circleFenceRect = {int(SCR_W / 2.0) - 128, (SCR_H - 48) - 128, 256, 256};

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

void Game::init(){
    btnsActive = true;
    tick = false;
    font = nullptr;
    gameOver = false;

    turretAngle = 0.0;
    muzzleAnim = nullptr;
    muzzleFlashID = -1;
    impactAnim = nullptr;
    impactAnimIDs[0] = -1;
    impactAnimIDs[1] = -1;
    impactAnimIDs[2] = -1;
    impactAnimIDs[3] = -1;

    targettedEnemy = nullptr;
    lClick = false;
    clickBounce = 0.0;
    logoID = -1;
    ngbID = -1;
    nextWaveBtnID = -1;
    diedSplashID = -1;

    redEnemyID = -1;
    blueEnemyID = -1;
    greenEnemyID = -1;

    spawnDelay = 2000.0;
    autoFireDelay = 500.0;
    spawnTicker = 0.0;
    autoFireTicker = 0.0;

    towerHp = 100;
    towerMaxHp = 100;
    hpBarID = -1;
    hpBarBgID = -1;

    circleFenceID = -1;
    statBtnID = -1;

    coins = 0;
    CoinTxID = -1;
    CoinTextTx = nullptr;

    currentWave = 1;
    nextWaveCounter = 0;
    nextWaveReq = 25;
    WaveTextTx = nullptr;

    initStats();
}

void Game::initStats(){
    std::list<std::string> statNames = {"Dmg", "CoinDrop", "SpawnRate", "EnemySpeed", "AutofireRate", "fireRange", "maxEnemies"};
    int statOffset = int(SCR_H / 2.0) + 64;
    int rowOffset = 64 + 4;
    int max_row = 4;
    int iCount = 0;
    for(std::list<std::string>::iterator it = statNames.begin(); it != statNames.end(); it++, iCount++){
        statMap[(*it)] = new Stat(10 + (rowOffset * (iCount / max_row)), statOffset + ((iCount % max_row) * 20), 64, 16);
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

void Game::targetEnemy(){
    double lowestDist = baseRange + (10 * statMap["fireRange"]->stat);
    for(std::vector<Enemy*>::iterator it = enemies.begin(); it != enemies.end(); it++){
        Enemy& E = (*(*it));
        double xd = (E.x + (E.w / 2.0)) - (turretBaseRect.x + (turretBaseRect.w / 2.0));
        double yd = (turretBaseRect.y + (turretBaseRect.h / 2.0)) - (E.y + (E.h / 2.0));
        double dist = sqrt((xd*xd)+(yd*yd));
        if(dist < lowestDist){
            lowestDist = dist;
            turretAngle = sin(xd/yd) * 45.0;
            targettedEnemy = (*it);
        }
    }
}

int Game::spawnNewEnemy(){
    if(int(enemies.size()) < (10 + (statMap["maxEnemies"]->stat * 5)) && (int(enemies.size()) < MAX_ENEMIES)){
        Enemy* E = new Enemy();
        E->id = Enemy::getID();
        E->maxHp = 5 * (1.0 + ((currentWave - 1) / 2.0));
        E->hp = 5 * (1.0 + ((currentWave - 1) / 2.0));
        E->x = (std::rand() % int(SCR_W * 2.0)) - (SCR_W);
        E->y = -32.0;
        E->w = 16.0;
        E->h = 16.0;
        E->spd = 16.0;
        E->coinDrop = 1 + (currentWave / 2.0);
        E->atkDelay = 1000.0;
        E->dmg = 1;
        E->aspd = 0.5;

        if(((currentWave - 3) % 10) == 0){
            E->type = Enemy::Types::tFast;
            E->spd *= 2;
        }else if(((currentWave + 3) % 10) == 0){
            E->type = Enemy::Types::tHeavy;
            E->w *= 2;
            E->h *= 2;
            E->maxHp *= 2;
            E->hp *= 2;
        }else{
            E->type = Enemy::Types::tRegular;
        }

        enemies.push_back(E);
        return E->id;
    }
    return -1;
}

Game::~Game(){
    destroyButtons();
    destroyTextures();
    destroyEnemies();
    destroyStats();
}

Game game;

void Game::newStatBtn(Stat* _stat, int _texID){
    _stat->btnID = statBtnID;
    buttons.push_back(new Button(_stat->btnTexRect, &statUpgF, this, _stat));
    _stat->textTex = renderText(std::to_string(_stat->level) + ": " + std::to_string(_stat->upgradeCost), {0,0,0}, renderer, font);
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
        //buttons.push_back(new Button(nextWaveRect, nextWaveBtnF, this));
        currentWave++;
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

void Game::drawTurret(int _id, SDL_Rect _dest, double angle){
    if(_id < int(textures.size())){
        SDL_RenderCopyEx(renderer, textures[_id], NULL, &_dest, angle, &turretPivotOffset, SDL_FLIP_NONE);
    }
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    DrawCircle(renderer, turretBaseRect.x  + (turretBaseRect.w / 2.0), turretBaseRect.y + (turretBaseRect.h / 2.0), baseRange + (statMap["fireRange"]->stat * 10));
}

void Game::statUpgF(Stat& _stat){
    if(coins >= _stat.upgradeCost){
        updateCoins(-_stat.upgradeCost);
        _stat.upgradeCost *= 1.3;
        SDL_DestroyTexture(_stat.textTex);
        _stat.level++;
        _stat.stat++;
        _stat.textTex = renderText(std::to_string(_stat.level) + ": " + std::to_string(_stat.upgradeCost), {0,0,0}, renderer, font);
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

bool Game::fireTurret(){
    if(targettedEnemy != nullptr){
        targettedEnemy->takeDamage(statMap["Dmg"]->stat);
        SDL_Rect pos = {turretCannonRect.x - 4, turretCannonRect.y - 32, 32, 32};
        SDL_Point offset = turretPivotOffset;
        offset.x += 4;
        offset.y += 32;
        animInstances.push_back(spawnAnim(muzzleAnim, pos, turretAngle, offset, false, 1.0));
        return true;
    }else{
        targetEnemy();
        if(targettedEnemy != nullptr){
            targettedEnemy->takeDamage(statMap["Dmg"]->stat);
            SDL_Rect pos = {turretCannonRect.x - 4, turretCannonRect.y - 32, 32, 32};
            SDL_Point offset = turretPivotOffset;
            offset.x += 4;
            offset.y += 32;
            animInstances.push_back(spawnAnim(muzzleAnim, pos, turretAngle, offset, false, 1.0));
            return true;
        }
    }
    return false;
}

bool Game::update(double dTime){
    SDL_SetRenderDrawColor(renderer, 240, 240, 0, 255);
    SDL_RenderClear(renderer);

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
                        if(turretAngle > (-45.0)){
                            turretAngle -= 5.0;
                        }
                        break;
                    case SDLK_RIGHT:
                        if(turretAngle < (45.0)){
                            turretAngle += 5.0;
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
        }
    }// poll &e

    if(state == gsInit){
        destroyButtons();
        destroyEnemies();
        destroyTextures();
        destroyStats();
        init();
        font = TTF_OpenFont("slkscr.ttf", fontSize);
        logoID = newTexture("TowerDefence.png");
        ngbID = newTexture("NewGame.png");
        buttons.push_back(new Button(ngbRect, newGameBtn));
        towerHp = 100;
        towerMaxHp = 100;
        updateState(gsMainMenu);
    }else if(state == gsMainMenu){
        drawTexture(logoID, logoRect);
        drawTexture(ngbID, ngbRect);
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
        turretCannonID = newTexture("td_basic_towers\\Cannon.png");
        circleFenceID = newTexture("circle_fence.png");
        muzzleFlashID = newTexture("MuzzleFlash.png");

        impactAnimIDs[0] = newTexture("impact_0.png");
        impactAnimIDs[1] = newTexture("impact_1.png");
        impactAnimIDs[2] = newTexture("impact_2.png");
        impactAnimIDs[3] = newTexture("impact_3.png");

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

        muzzleAnim = new Anim{30.0};
        muzzleAnim->addFrameTx(textures[muzzleFlashID]);
        muzzleAnim->addFrameTx(textures[muzzleFlashID]);

        impactAnim = new Anim{30.0};
        impactAnim->addFrameTx(textures[impactAnimIDs[0]]);
        impactAnim->addFrameTx(textures[impactAnimIDs[1]]);
        impactAnim->addFrameTx(textures[impactAnimIDs[2]]);
        impactAnim->addFrameTx(textures[impactAnimIDs[3]]);

        updateState(gsGame);
    }else if(state == gsGame){
        drawTexture(turretBaseID, turretBaseRect);
        drawTurret(turretCannonID, turretCannonRect, turretAngle);

        if(spawnTicker > 0.0){
            spawnTicker -= dTime;
        }else{
            if(int(enemies.size()) < (nextWaveReq - nextWaveCounter)){
                int newID = spawnNewEnemy();
                spawnTicker = spawnDelay / statMap["SpawnRate"]->stat;
            }
        }

        autoFireTicker -= dTime;
        if((autoFireTicker <= 0) && !(gameOver)){
            if(fireTurret()){
                autoFireTicker = autoFireDelay * (1.0 / statMap["AutofireRate"]->stat);
            }
        }
        SDL_Rect hpRect = {int(SCR_W / 2.0) - 32, SCR_H - 10, int(64 * double(autoFireTicker / autoFireDelay)), 4};
        drawTexture(hpBarID, hpRect);

        if((lClick) && !(gameOver)){
            fireTurret();
        }

        drawTexture(circleFenceID, circleFenceRect);

        for(std::vector<Enemy*>::iterator it = enemies.begin(); it != enemies.end();){
            if((*it)->dying){
                updateCoins((*it)->coinDrop * (1.0 + (statMap["CoinDrop"]->stat - 1)));
                if((*it) == targettedEnemy){
                    targettedEnemy = nullptr;
                }
                Enemy::clearID((*it)->id);
                delete (*it);
                it = enemies.erase(it);
                updateWaves();
            }else{
                if(!gameOver){
                    (*it)->update(dTime, statMap["EnemySpeed"]->stat);
                    if((*it)->isAttacking){
                        SDL_Rect pos = {(*it)->x + ((*it)->w / 2.0)- 32, (*it)->y + (*it)->w - 32, 64, 64};
                        SDL_Point offset = {0,0};
                        animInstances.push_back(spawnAnim(impactAnim, pos, 0.0, offset, false, 0.5));
                        towerHp -= (*it)->dmg;
                        if(towerHp <= 0){
                            gameOver = true;
                        }
                    }
                }
                if((*it) == targettedEnemy){
                    DrawCircle(renderer, (*it)->x + ((*it)->w / 2.0), (*it)->y + ((*it)->h / 2.0), (*it)->w);
                }
                SDL_Rect enemyRect = {int((*it)->x), int((*it)->y), int((*it)->w), int((*it)->h)};
                switch((*it)->type){
                    case Enemy::Types::tFast:
                        drawTexture(blueEnemyID, enemyRect);
                        break;
                    case Enemy::Types::tHeavy:
                        drawTexture(greenEnemyID, enemyRect);
                        break;
                    case Enemy::Types::tRegular:
                    default:
                        drawTexture(redEnemyID, enemyRect);
                        break;
                }
                SDL_Rect hpRect = enemyRect;
                hpRect.y -= 8;
                hpRect.h = 4;
                hpRect.w = int(hpRect.w * double(double((*it)->hp) / double((*it)->maxHp)));
                drawTexture(hpBarID, hpRect);
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

    SDL_RenderPresent(renderer);

    return false;
}

void Game::setRenderer(SDL_Renderer* _renderer){
    renderer = _renderer;
}

void Game::updateState(GState newState){
    state = newState;
}

int main(int argc, char* argv[]){
    std::srand(time(nullptr));
    SDL_Window* window = NULL;
    SDL_Surface* ScrSurface = NULL;
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
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
            SDL_RenderClear(renderer);
            SDL_RenderPresent(renderer);
        }
    }

    if(initSuccess){
        AspectRatio aspect;
        SDL_DisplayMode dm;
        if(SDL_GetCurrentDisplayMode(0, &dm) >= 0){
            double div = double(dm.w) / double(dm.h);
            if(div > (1.7) && div < (1.8)){
                aspect = r16x9;
            }else if(div = (1.5)){
                aspect = r3x2;
            }
            if(FULLSCREEN == true){

            }
        }
        game.setRenderer(renderer);
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
    SDL_Quit();
    return 0;
}
