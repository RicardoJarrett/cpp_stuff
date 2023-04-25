#include "Enemy.h"

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
    if(checkID(_id) == true){
        ids.push(_id);
    }
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
        int dx = (LEVEL_W / 2.0) - (x + (w / 2.0));
        int dy = (LEVEL_H / 2.0) - (y + (h / 2.0));
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
