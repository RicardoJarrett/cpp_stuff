#ifndef _GAME_H_
#define _GAME_H_

#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <algorithm>

#include "SDL.h"
#include "SDL_ttf.h"
#include "Base.h"
#include "Anim.h"
#include "Stat.h"
#include "Turret.h"
#include "helper.h"
#include "Log.h"

using namespace helper;

extern int SCR_W;
extern int SCR_H;

enum GState{gsInit, gsMainMenu, gsGameStart, gsGame, gsClose};

class Game{
    SDL_Renderer* renderer;
    SDL_Texture* ScrSurface;
    SDL_Texture* LevelSurface;
    TTF_Font* fonts[3];
    Log* _logger;
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

    int healDelay;
    int healTicker;

    void initStats();

    void targetEnemy(int _tid);
    void targetEnemy(Turret* _t);
    void fireTurret(int _turretID);
    void fireTurret(Turret* _t);
    void drawBase();

    void statUpgF(Stat& _stat, bool _texOnly);
    void statBaseUpgF(Stat& _stat, bool _textOnly);
    void drawUpgBtns();

    void newStatBtn(Stat* _stat, int _texID, std::string _toolTip);
    void newStatBtn(Stat* _stat, std::string _iconPath, std::string _toolTip);
    Button* newStatForTurret(Stat** _stat, int _texID, std::string _toolTip);

    void updateCoins(int _coins);
    void updateWaves();

    bool updateAnims();
    void nextWaveBtnF();

    void loadHiscores();
    void newGameBtn();
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
    void destroyAnims();
    void init();
    void newTurret(Turret::turretType _turretType);
    void updateState(GState newState);
    int newTexture(std::string _path, bool _inImages=true);
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
    void updateStatText(Stat& _stat);
    void updateButtons(double _dTime);
    void drawBG();
    void handleEvents(SDL_Event& _e);
    bool checkBayClicks();
    void changeBay(int _newBayID);
    void clearVectors();
    void setLogger(Log& _newLogger);
    void log(std::string _string);
};

#endif // _GAME_H_
