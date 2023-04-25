#include "Anim.h"

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
