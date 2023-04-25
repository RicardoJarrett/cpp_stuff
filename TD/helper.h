#ifndef _HELPER_H_
#define _HELPER_H_

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include <string>
#include <windows.h>
#include <iostream>
#include <cmath>
#include <map>

namespace helper{
    extern double _180_over_pi;
    extern double _pi_over_180;
    enum AspectRatio{r16x9, r3x2};

    std::string getPath();
    SDL_Texture* loadTexture(std::string path, SDL_Renderer* renderer);
    SDL_Texture* renderText(std::string _text, SDL_Color _col, SDL_Renderer* renderer, TTF_Font* _font);
    void DrawCircle(SDL_Renderer* renderer, int32_t centreX, int32_t centreY, int32_t radius);

    enum OctDir{dU=0, dUR=1, dR=2, dDR=3, dD=4, dDL=5, dL=6, dUL=7};
    OctDir RadToDir(double _angle);

    extern std::map<int, std::string> KFactorMap;
    int getKFactor(int _num);
    std::string NumToKNum(int _num);

    std::string rectToString(SDL_Rect _r);
    bool vecInRect(double _x, double _y, SDL_Rect _rect);
}

#endif // _HELPER_H_
