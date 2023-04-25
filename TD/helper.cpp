#include "helper.h"

double helper::_180_over_pi = (180.0 / M_PI);
double helper::_pi_over_180 = (M_PI / 180.0);

std::string helper::getPath(){
    char buf[256];
    GetCurrentDirectoryA(256, buf);
    std::string retStr = std::string(buf) + "\\";
    return retStr;
}

SDL_Texture* helper::loadTexture(std::string path, SDL_Renderer* renderer){
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

SDL_Texture* helper::renderText(std::string _text, SDL_Color _col, SDL_Renderer* renderer, TTF_Font* _font){
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

bool helper::vecInRect(double _x, double _y, SDL_Rect _rect){
    if((_x > _rect.x) && (_x < (_rect.x + _rect.w))
            && (_y > _rect.y) && (_y < (_rect.y + _rect.h))){
                return true;
    }
    return false;
}

std::string helper::rectToString(SDL_Rect _r){
    std::string retStr = std::to_string(_r.x) + "," + std::to_string(_r.y) + " " + std::to_string(_r.w) + "," + std::to_string(_r.h);
    return retStr;
}

helper::OctDir helper::RadToDir(double _angle){
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
    helper::OctDir _dir = helper::OctDir::dU;
    if(_angleD < _dirSplit){
        _dir = helper::OctDir::dU;
    }else if(_angleD < (3*_dirSplit)){
        _dir = helper::OctDir::dUR;
    }else if(_angleD < (5*_dirSplit)){
        _dir = helper::OctDir::dR;
    }else if(_angleD < (7*_dirSplit)){
        _dir = helper::OctDir::dDR;
    }else if(_angleD < (9*_dirSplit)){
        _dir = helper::OctDir::dD;
    }else if(_angleD < (11*_dirSplit)){
        _dir = helper::OctDir::dDL;
    }else if(_angleD < (13*_dirSplit)){
        _dir = helper::OctDir::dL;
    }else if(_angleD < (15*_dirSplit)){
        _dir = helper::OctDir::dUL;
    }else{
        _dir = helper::OctDir::dU;
    }
    return _dir;
}

void helper::DrawCircle(SDL_Renderer* renderer, int32_t centreX, int32_t centreY, int32_t radius){
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

std::map<int, std::string> helper::KFactorMap = {{0, "0"}, {1, "1"}, {2, "K"}, {3, "M"}, {4, "B"}, {5, "T"}};
int helper::getKFactor(int _num){
    if(_num == 0){
        return 0;
    }
    int _retFactor = 0;
    int currentFactor = 0;
    int kDiv = 1000;
    int _tNum = _num;
    while(std::abs(_tNum) >= 1){
        currentFactor++;
        _tNum /= kDiv;
    }
    _retFactor = currentFactor * (_num / std::abs(_num));
    return _retFactor;
}

std::string helper::NumToKNum(int _num){
    std::string _retStr = "";
    double _retNum = _num;
    int _kFactor = getKFactor(int(_retNum));
    if((std::abs(_kFactor) != 1) && (_kFactor != 0)){
        int _longFactor = std::pow(1000, std::abs(_kFactor) - 1);
        _retNum = _retNum / _longFactor;
        _retStr = std::to_string(_retNum);
        int preDotStrSize = (_retStr.substr(0, _retStr.find('.'))).size();
        _retStr = _retStr.substr(_retStr.find('.') - preDotStrSize, 4);
        if(_retStr.back() == '.'){
            _retStr = _retStr.substr(0, 3);
        }
        _retStr += helper::KFactorMap[_kFactor];
    }else{
        _retStr = std::to_string(int(_retNum));
    }
    return _retStr;
}
