#define SDL_MAIN_HANDLED
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include <iostream>
#include <Windows.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <cmath>
#include <queue>

#define _countof(array) (sizeof(array) / sizeof(array[0]))

int SCR_W = 320;
int SCR_H = 320;

std::string getPath(){
    char buf[256];
    GetCurrentDirectoryA(256, buf);
    return std::string(buf) + '\\';
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
        memberfunc,
    };
    SDL_Rect area;
    void (*clickFuncPtr)();
    void (Game::*clickMFuncPtr)(Stat& _stat);
    double bounce;
    type T;
    Game* gamePtr;
    Stat* upgStat;
public:
    Button(SDL_Rect _area, void (*F)());
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

Button::Button(SDL_Rect _area, void (Game::*F)(Stat& _stat), Game* _game, Stat* _stat) : area(_area), clickMFuncPtr(F), gamePtr(_game), upgStat(_stat){
    bounce = 0.0;
    T = memberfunc;
}

bool Button::checkClick(int x, int y){
    if((bounce == 0.0) && (checkOver(x, y))){
        switch(T){
            case func:
                clickFuncPtr();
                break;
            case memberfunc:
                (gamePtr->*clickMFuncPtr)(*upgStat);
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



class Enemy{
public:
    double x, y, w, h, spd;
    int hp, maxHp, id;
    static std::queue<int> ids;
    static int getID();
    static void clearID(int _id);
    static void initEnemies(int _maxEnemies);
    void update(double dTime);
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

void Enemy::update(double dTime){
    if(y < ((SCR_H / 2.0) - 16)){
        y += (spd * (dTime / 1000.0));
    }}

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
};

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
    int fontSize;
    double fps;
    double fpsTicker;
    bool tick;
    int mouseX, mouseY;
    bool lClick;
    double clickBounce;

    int logoID;
    int ngbID;
    int fenceID;
    int redEnemyID;
    int hpBarID;
    int turretBaseID;
    int turretCannonID;
    double turretAngle;
    Enemy* targettedEnemy;
    double turretRange;


    Stat Dmg;
    Stat CoinDrop;
    Stat SpawnRate;

    int CoinID;
    int coins;
    SDL_Texture* CoinsTx;

    SDL_Rect logoRect;
    SDL_Rect ngbRect;
    SDL_Rect fenceRect;
    SDL_Rect turretBaseRect;
    SDL_Rect turretCannonRect;
    SDL_Point turretPivotOffset;
    SDL_Rect CoinRect;
    SDL_Rect CoinsTxRect;

    std::vector<Button*> buttons;
    std::vector<SDL_Texture*> textures;
    std::map<std::string, int> texIDs;
    std::vector<Enemy*> enemies;

    int MAX_ENEMIES;
    double spawnDelay;
    double spawnTicker;
    double fireDelay;
    double fireTicker;

    void targetEnemy();

    void statUpgF(Stat& _stat);
    void drawUpgBtn(Stat& _stat);


    void updateCoins(int _coins);
public:
    Game();
    ~Game();
    bool update(double dTime);
    void setRenderer(SDL_Renderer* _renderer);
    void destroyButtons();
    void destroyTextures();
    void destroyEnemies();
    void updateState(GState newState);
    int newTexture(std::string _path);
    void drawTexture(int _id, SDL_Rect _dest);
    void drawTurret(int _id, SDL_Rect _dest, double angle);
    int spawnNewEnemy();
};

Game::Game(){
    state = gsInit;
    fps = 30.0;
    fpsTicker = 1000.0 / fps;
    tick = false;;
    logoRect = {50, 30, 228, 122};
    ngbRect = {32, 240, 256, 64};
    fenceRect = {0, int(SCR_H / 2.0) - 16, SCR_W, 32};
    turretBaseRect = {int(SCR_W / 2.0) - 32, SCR_H - 80, 64, 64};
    turretCannonRect = {turretBaseRect.x + 20, turretBaseRect.y - 20, 24, 64};
    turretAngle = 0.0;
    turretPivotOffset = {12, 52};
    targettedEnemy = nullptr;
    SDL_GetMouseState(&mouseX, &mouseY);
    lClick = false;
    clickBounce = 0.0;
    logoID = -1;
    ngbID = -1;
    fenceID = -1;
    redEnemyID = -1;
    MAX_ENEMIES = 20;
    spawnDelay = 1000.0;
    spawnTicker = 0.0;
    turretRange = 220.0;

    int statOffset = int(SCR_H / 2.0) + 64;

    Dmg.btnID = -1;
    Dmg.btnTexRect = {10, statOffset, 64, 16};
    Dmg.texRect = {Dmg.btnTexRect.x + 16, Dmg.btnTexRect.y + 2, 60, 12};
    Dmg.upgradeCost = 10;
    Dmg.textTex = nullptr;
    Dmg.iconID = -1;
    Dmg.iconRect = {Dmg.btnTexRect.x + 2, Dmg.btnTexRect.y + 2, 12, 12};
    Dmg.level = 1;
    Dmg.stat = 1;

    CoinDrop.btnID = -1;
    CoinDrop.btnTexRect = {10, statOffset + 20, 64, 16};
    CoinDrop.texRect = {CoinDrop.btnTexRect.x + 16, CoinDrop.btnTexRect.y + 2, 60, 12};
    CoinDrop.upgradeCost = 10;
    CoinDrop.textTex = nullptr;
    CoinDrop.iconID = -1;
    CoinDrop.iconRect = {CoinDrop.btnTexRect.x + 2, CoinDrop.btnTexRect.y + 2, 12, 12};
    CoinDrop.level = 1;
    CoinDrop.stat = 1;

    SpawnRate.btnID = -1;
    SpawnRate.btnTexRect = {10, statOffset + 40, 64, 16};
    SpawnRate.texRect = {SpawnRate.btnTexRect.x + 16, SpawnRate.btnTexRect.y + 2, 60, 12};
    SpawnRate.upgradeCost = 10;
    SpawnRate.textTex = nullptr;
    SpawnRate.iconID = -1;
    SpawnRate.iconRect = {SpawnRate.btnTexRect.x + 2, SpawnRate.btnTexRect.y + 2, 12, 12};
    SpawnRate.level = 1;
    SpawnRate.stat = 1;

    fontSize = 8;
    renderer = nullptr;
    font = nullptr;
    CoinsTx = nullptr;
    CoinRect = {int(SCR_W / 8.0), 10, 32, 32};
    CoinsTxRect = {int(SCR_W / 8.0) + 32, 12, 64, 32};
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
    fenceID = -1;
    hpBarID = -1;
}

void Game::destroyEnemies(){
    for(std::vector<Enemy*>::iterator it = enemies.begin(); it != enemies.end(); it++){
        delete (*it);
    }
    enemies.clear();
}

void Game::targetEnemy(){
    double lowestDist = turretRange;
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
    if(int(enemies.size()) < MAX_ENEMIES){
        Enemy* E = new Enemy();
        E->id = Enemy::getID();
        E->maxHp = 5;
        E->hp = 5;
        E->x = std::rand() % (SCR_W - 16);
        E->y = -32.0;
        E->w = 16.0;
        E->h = 16.0;
        E->spd = 32.0;
        enemies.push_back(E);
        return E->id;
    }
    return -1;
}

Game::~Game(){
    destroyButtons();
    destroyTextures();
    destroyEnemies();
}

Game game;

void Game::updateCoins(int _coins){
    coins += _coins;
    CoinsTx = renderText(std::to_string(coins), {0,0,0}, renderer, font);
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

void DrawCircle(SDL_Renderer* renderer, int32_t centreX, int32_t centreY, int32_t radius)
{
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
    DrawCircle(renderer, turretBaseRect.x  + (turretBaseRect.w / 2.0), turretBaseRect.y + (turretBaseRect.h / 2.0), turretRange);
}

void Game::statUpgF(Stat& _stat){
    if(coins >= _stat.upgradeCost){
        updateCoins(-_stat.upgradeCost);
        _stat.upgradeCost *= 1.1;
        SDL_DestroyTexture(_stat.textTex);
        _stat.level++;
        _stat.stat++;
        _stat.textTex = renderText(std::to_string(_stat.level) + ": " + std::to_string(_stat.upgradeCost), {0,0,0}, renderer, font);
    }
}

void Game::drawUpgBtn(Stat& _stat){
    drawTexture(_stat.btnID, _stat.btnTexRect);
    drawTexture(_stat.iconID, _stat.iconRect);
    int txw, txh;
    SDL_QueryTexture(_stat.textTex, NULL, NULL, &txw, &txh);
    _stat.texRect.w = txw * 1.5;
    _stat.texRect.h = txh * 1.5;
    SDL_RenderCopy(renderer, _stat.textTex, NULL, &_stat.texRect);
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
        font = TTF_OpenFont("slkscr.ttf", fontSize);
        logoID = newTexture("TowerDefence.png");
        ngbID = newTexture("NewGame.png");
        buttons.push_back(new Button(ngbRect, newGameBtn));
        updateState(gsMainMenu);
    }else if(state == gsMainMenu){
        drawTexture(logoID, logoRect);
        drawTexture(ngbID, ngbRect);
    }else if(state == gsGameStart){
        std::string path = getPath();
        fenceID = newTexture("fence2.png");
        redEnemyID = newTexture("redEnemy.png");
        hpBarID = newTexture("hpBar.png");
        Enemy::initEnemies(MAX_ENEMIES);
        turretBaseID = newTexture("td_basic_towers\\Tower.png");
        turretCannonID = newTexture("td_basic_towers\\Cannon.png");

        CoinID = newTexture("coin.png");
        updateCoins(0);

        Dmg.btnID = newTexture("button_turq.png");
        buttons.push_back(new Button(Dmg.btnTexRect, &statUpgF, this, &Dmg));
        Dmg.textTex = renderText(std::to_string(Dmg.level) + ": " + std::to_string(Dmg.upgradeCost), {0,0,0}, renderer, font);
        Dmg.iconID = newTexture("bullet.png");

        CoinDrop.btnID = Dmg.btnID;
        buttons.push_back(new Button(CoinDrop.btnTexRect, &statUpgF, this, &CoinDrop));
        CoinDrop.textTex = renderText(std::to_string(CoinDrop.level) + ": " + std::to_string(CoinDrop.upgradeCost), {0,0,0}, renderer, font);
        CoinDrop.iconID = CoinID;

        SpawnRate.btnID = Dmg.btnID;
        buttons.push_back(new Button(SpawnRate.btnTexRect, &statUpgF, this, &SpawnRate));
        SpawnRate.textTex = renderText(std::to_string(SpawnRate.level) + ": " + std::to_string(SpawnRate.upgradeCost), {0,0,0}, renderer, font);
        SpawnRate.iconID = newTexture("Hourglass.png");

        updateState(gsGame);
    }else if(state == gsGame){
        drawTexture(fenceID, fenceRect);
        drawTexture(turretBaseID, turretBaseRect);
        drawTurret(turretCannonID, turretCannonRect, turretAngle);
        if(spawnTicker > 0.0){
            spawnTicker -= dTime;
        }else{
            int newID = spawnNewEnemy();
            spawnTicker = spawnDelay / SpawnRate.stat;
        }
        if(lClick){
            if(targettedEnemy != nullptr){
                targettedEnemy->takeDamage(Dmg.stat);
            }else{
                targetEnemy();
                if(targettedEnemy != nullptr){
                    targettedEnemy->takeDamage(Dmg.stat);
                }
            }
        }
        for(std::vector<Enemy*>::iterator it = enemies.begin(); it != enemies.end();){
            if((*it)->dying){
                if((*it) == targettedEnemy){
                    targettedEnemy = nullptr;
                }
                Enemy::clearID((*it)->id);
                delete (*it);
                it = enemies.erase(it);
                updateCoins(CoinDrop.stat);
            }else{
                (*it)->update(dTime);
                if((*it) == targettedEnemy){
                    DrawCircle(renderer, (*it)->x + ((*it)->w / 2.0), (*it)->y + ((*it)->h / 2.0), (*it)->w);
                }
                SDL_Rect enemyRect = {int((*it)->x), int((*it)->y), int((*it)->w), int((*it)->h)};
                drawTexture(redEnemyID, enemyRect);
                SDL_Rect hpRect = enemyRect;
                hpRect.y -= 8;
                hpRect.h = 4;
                hpRect.w = int(hpRect.w * double(double((*it)->hp) / double((*it)->maxHp)));
                drawTexture(hpBarID, hpRect);
                it++;
            }
        }

        drawUpgBtn(Dmg);
        drawUpgBtn(CoinDrop);
        drawUpgBtn(SpawnRate);

        int txw, txh;
        drawTexture(CoinID, CoinRect);
        SDL_QueryTexture(CoinsTx, NULL, NULL, &txw, &txh);
        CoinsTxRect.w = txw * 3;
        CoinsTxRect.h = txh * 3;
        SDL_RenderCopy(renderer, CoinsTx, NULL, &CoinsTxRect);

    }

    for(std::vector<Button*>::iterator it = buttons.begin(); it != buttons.end(); it++){
        Button& B = *(*it);
        B.update(dTime);
        if(lClick){
            if(B.checkClick(mouseX, mouseY)){
                break;
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
