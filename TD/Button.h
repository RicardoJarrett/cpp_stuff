#ifndef _BUTTON_H_
#define _BUTTON_H_

#include "SDL.h"


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
    void (Game::*StatBaseUpgFuncPtr)(Stat& _stat, bool _texOnly);
public:
    bool active;

    Button(SDL_Rect _area, void (*F)(), SDL_Texture* _toolTipTex);
    Button(SDL_Rect _area, void (Game::*F)(), Game* _game, SDL_Texture* _toolTipTex);
    Button(SDL_Rect _area, void (Game::*F)(Stat& _stat, bool _texOnly), Game* _game, Stat* _stat, SDL_Texture* _toolTipTex);
    ~Button();
    bool checkClick();
    bool checkOver(int x, int y);
    void update(double dTime);
    SDL_Rect& getArea();
    void moveButton(SDL_Rect _pos);
    Stat* getStat();
    SDL_Texture* getTexture();
};

#endif // _BUTTON_H_
