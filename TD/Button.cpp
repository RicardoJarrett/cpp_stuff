#include "Button.h"

SDL_Rect& Button::getArea(){
    return area;
}

Button::~Button(){
    gamePtr = nullptr;
    clickFuncPtr = nullptr;
    clickMFuncPtr = nullptr;
    clickStatFuncPtr = nullptr;
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

Button::Button(SDL_Rect _area, void (Game::*F)(), Game* _game, SDL_Texture* _toolTipTex) : area(_area), gamePtr(_game), toolTipTex(_toolTipTex), clickMFuncPtr(F){
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
                (gamePtr->*clickStatFuncPtr)(*upgStat, false);
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
