#ifndef _ANIM_H_
#define _ANIM_H_

#include <vector>
#include "SDL.h"

struct Anim{
    std::vector<SDL_Texture*> frames;
    double frameRate;

    Anim(Anim& _anim);
    Anim(double _frameRate);
    void addFrameTx(SDL_Texture* _newFrame);
};

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

struct SpriteAnimInst : public AnimInst{
    int w, h;
    SpriteAnimInst(Anim* _anim, SDL_Rect _pos, int _w, int _h, double _angle, SDL_Point _offset, bool _loop, double _speed);
    bool update(double dTime) override;
    ~SpriteAnimInst() override;
};


#endif // _ANIM_H_
