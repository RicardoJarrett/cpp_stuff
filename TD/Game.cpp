#include "Game.h"

void Game::changeBay(int _newBayID){
    if((_newBayID >= 0) && (_newBayID < 21)){
        if(selectedBayID == _newBayID){
            if(activeTurrets[selectedBayID] != nullptr){
                activeTurrets[selectedBayID]->deactivateButtons();
            }
            selectedBayID = -1;
        }else{
            if(selectedBayID != -1){
                if(activeTurrets[selectedBayID] != nullptr){
                    activeTurrets[selectedBayID]->deactivateButtons();
                }
                if(!((base.turretBays_2[selectedBayID]->isUpgrading) || (base.turretBays_2[selectedBayID]->isSwapping))){
                    base.startSwap(selectedBayID, _newBayID);
                }
            }
            selectedBayID = _newBayID;
        }
    }else{
        std::cout << "ERROR: Change to invalid bayID: " << _newBayID << "\n";
    }
}

bool Game::checkBayClicks(){
    bool _bayClicked = false;
    for(int i = 0; i < 21; i++){
        double scaledMX = double(mouseX);
        double scaledMY = double(mouseY);
        screenToLevel(scaledMX, scaledMY);
        if(vecInRect(scaledMX, scaledMY, base.turretBays_2[i]->turret_Rect)){
            changeBay(i);
            _bayClicked = true;
            return true;
        }
    }
    return _bayClicked;
}

void Game::handleEvents(SDL_Event& _e){
    while(SDL_PollEvent(&_e) != 0){
        switch(_e.type){
            case SDL_QUIT:
                updateState(gsClose);
                break;
            case SDL_KEYDOWN:
                switch(_e.key.keysym.sym){
                    case SDLK_q:
                        updateState(gsClose);
                        break;
                    case SDLK_LEFT:
                        if(ViewAngle > (-360.0)){
                            ViewAngle -= 90.0;
                        }else{
                            ViewAngle = 0.0;
                        }
                        break;
                    case SDLK_RIGHT:
                        if(ViewAngle < (360.0)){
                            ViewAngle += 90.0;
                        }else{
                            ViewAngle = 0.0;
                        }
                        break;
                    case SDLK_UP:
                        if(viewScale > 0.5){
                            viewScale -= 0.1;
                        }else{
                            viewScale = 0.5;
                        }
                        break;
                    case SDLK_DOWN:
                        if(viewScale < 2.0){
                            viewScale += 0.1;
                        }else{
                            viewScale = 2.0;
                        }
                        break;
                    case SDLK_RETURN:
                        if(gameOver){
                            updateState(gsInit);
                        }
                        break;
                    case SDLK_p:
                        paused = !paused;
                        break;
                }
                break;
            case SDL_MOUSEMOTION:
                SDL_GetMouseState(&mouseX, &mouseY);
                break;
            case SDL_MOUSEBUTTONDOWN:
                if(_e.button.button == SDL_BUTTON_LEFT){
                    if(clickBounce <= 0.0){
                        if(gameOver){
                            updateState(gsInit);
                        }
                        lClick = true;
                        clickBounce = 10.0;
                    }else{
                        lClick = false;
                    }
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if(_e.button.button == SDL_BUTTON_LEFT){
                    lClick = false;
                }
                break;
            case SDL_MOUSEWHEEL:
                if(_e.wheel.y > 0){
                    if(viewScale < 2.0){
                        viewScale += _e.wheel.y * 0.1;
                    }
                    if(viewScale > 2.0){
                        viewScale = 2.0;
                    }
                }else if(_e.wheel.y < 0){
                    if(viewScale > 0.5){
                        viewScale += _e.wheel.y * 0.1;
                    }
                    if(viewScale < 0.5){
                        viewScale = 0.5;
                    }
                }
                break;
        }
    }// poll &e
}

void Game::drawBG(){
    SDL_Rect tRect = grassBGRect;
    for(int i = 0; i < bgXtiles; i++){
        for(int j = 0; j < bgYtiles; j++){
            tRect.x = i * 1280;
            tRect.y = j * 720;
            drawTexture(grassBGID, tRect);
        }
    }
}

void Game::updateButtons(double _dTime){
    if(btnsActive){
        for(std::vector<Button*>::iterator it = buttons.begin(); it != buttons.end(); it++){
            Button& B = *(*it);
            B.update(_dTime);
            if(B.checkOver(mouseX, mouseY)){
                if(lClick){
                    if(B.checkClick()){
                        break;
                    }
                }
                Stat* _s = B.getStat();
                SDL_Rect tRect;
                if(_s == nullptr){
                    tRect = ngbRect;
                }else{
                    tRect = _s->btnTexRect;
                }
                tRect.x += 16;
                tRect.y -= 16;
                int txw, txh;
                SDL_Texture* _tex = B.getTexture();
                SDL_QueryTexture(_tex, NULL, NULL, &txw, &txh);
                tRect.w = txw;
                tRect.h = txh;

                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderFillRect(renderer, &tRect);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderDrawRect(renderer, &tRect);
                SDL_RenderCopy(renderer, _tex, NULL, &tRect);
            }
        }
    }
}

void Game::levelToScreen(double& _x, double& _y){
    _x = ((double(_x) / LEVEL_W) * (ViewRect.w * viewScale));
    _y = ((double(_y) / LEVEL_H) * (ViewRect.h * viewScale));
}

void Game::screenToLevel(double& _x, double& _y){
    SDL_Point _p = {int(LEVEL_W / 2.0), int(LEVEL_H / 2.0)};
    SDL_Rect scaledViewRect = ViewRect;
    scaledViewRect.w *= viewScale;
    scaledViewRect.h *= viewScale;
    scaledViewRect.x = _p.x - (scaledViewRect.w / 2.0);
    scaledViewRect.y = _p.y - (scaledViewRect.h / 2.0);
    _x = ((double(_x) / SCR_W) * scaledViewRect.w) + scaledViewRect.x;
    _y = ((double(_y) / SCR_H) * scaledViewRect.h) + scaledViewRect.y;
}

void Game::setTurret(int _bayID, Turret* _turret){
    if((_bayID >= 0) && (_bayID < 21)){
        activeTurrets[_bayID] = _turret;
    }
}

int Game::TURRET_CYCLE = 0;
bool Game::cycleTurrets(Turret** _t, Turret** _tVec=nullptr){
    if(_tVec == nullptr){
        _tVec = activeTurrets;
    }
    if(Game::TURRET_CYCLE >= 21){
        Game::TURRET_CYCLE = 0;
        *_t = nullptr;
        return false;
    }
    *_t = *(_tVec + Game::TURRET_CYCLE);
    Game::TURRET_CYCLE++;
    return true;
}

Game::Game(){
    _logger = nullptr;
    state = gsInit;
    ViewAngle = 0.0;
    ScrnRect = {0, 0, SCR_W, SCR_H};

    LevelRect = {0, 0, LEVEL_W, LEVEL_H};
    ViewRect = {0, 0, SCR_W, SCR_H};
    viewScale = 1.0;

    hiscoreTexs[0] = nullptr;
    hiscoreTexs[1] = nullptr;
    hiscoreTexs[3] = nullptr;
    hiscoreTexs[4] = nullptr;

    for(int i = 0; i < 8; i++){
        walkAnims[i] = nullptr;
    }

    fps = 30.0;
    fpsTicker = 1000.0 / fps;

    base.setOwner(*(this));
    base.pos = {int(LEVEL_W / 2.0) - 128, int(LEVEL_H / 2.0) - 128, 256, 256};
    logoRect = {int((SCR_W / 2.0) - (SCR_W / 4.0)), int(SCR_H / 8.0), int(SCR_W / 2.0), int(SCR_H / 4.0)};
    ngbRect = {int(SCR_W / 2.0) - 128, int((SCR_H / 4.0) * 3.0), 256, 64};
    nextWaveRect = {int(SCR_W / 2.0) - 64, int(SCR_H / 0.75), 128, 32};

    bgXtiles = (LEVEL_W / 1280) + 1;
    bgYtiles = (LEVEL_H / 720) + 1;
    grassBGRect = {0, 0, 1280, 720};

    //turretBaseRect = {int(LEVEL_W / 2.0) - 32, int(LEVEL_H / 2.0) - 32, 64, 64};
    //turretCannonRect = {turretBaseRect.x + 20, turretBaseRect.y - 20, 24, 64};
    //turretPivotOffset = {12, 52};

    SDL_GetMouseState(&mouseX, &mouseY);

    MAX_ENEMIES = 250;

    //baseRange = 220.0;

    renderer = nullptr;
    selectedBayID = -1;

    CoinIconRect = {int(SCR_W / 8.0), 10, 32, 32};
    CoinTextRect = {int(SCR_W / 8.0) + 32, 12, 64, 32};

    WaveTextRect = {int((SCR_W / 2.0)), 10, 64, 32};
    btnsActive = true;
    tick = false;
    fonts[0] = nullptr;
    fonts[1] = nullptr;
    fonts[2] = nullptr;
    logoID = -1;
    ngbID = -1;
    grassBGID = -1;
    dmgIconID = -1;
    autofireIconID = -1;
    fireRangeIconID = -1;
    healDelay = 10000.0;
    healTicker = healDelay;
}

void Game::setLogger(Log& _newLogger){
    _logger = &(_newLogger);
}

void Game::loadHiscores(){
    std::ifstream hiscoresF;
    hiscoresF.open(getPath() + "hiscores.sav");
    if(hiscoresF.is_open()){
        std::string line;
        while(std::getline(hiscoresF, line)){
            std::string tStrPlayer = line.substr(0, line.find(":"));
            std::string tStrScore = line.substr(line.find(":")+1);
            hiScores.push_back(make_pair(std::stoi(tStrScore), tStrPlayer));
        }
    }
    hiscoresF.close();
    sort(hiScores.begin(), hiScores.end());
    reverse(hiScores.begin(), hiScores.end());
    hiscoreTexs[0] = renderText("HiScores", {0,0,0}, renderer, fonts[2]);
    for(int i = 0; i < 3; i++){
        std::string tStr = hiScores[i].second + " : "  + std::to_string(hiScores[i].first);
        hiscoreTexs[i+1] = renderText(tStr, {0,0,0}, renderer, fonts[2]);
    }
}

void Game::init(){
    btnsActive = true;
    tick = false;
    fonts[0] = nullptr;
    fonts[1] = nullptr;
    fonts[2] = nullptr;
    gameOver = false;
    paused = false;

    //turretAngle = 0.0;
    muzzleAnim = nullptr;
    muzzleFlashID = -1;
    impactAnim = nullptr;
    impactAnimIDs[0] = -1;
    impactAnimIDs[1] = -1;
    impactAnimIDs[2] = -1;
    impactAnimIDs[3] = -1;

    //targettedEnemy = nullptr;
    lClick = false;
    clickBounce = 0.0;
    logoID = -1;
    ngbID = -1;
    nextWaveBtnID = -1;
    diedSplashID = -1;
    pausedSplashID = -1;

    redEnemyID = -1;
    blueEnemyID = -1;
    greenEnemyID = -1;

    spawnDelay = 1000.0;
    //autoFireDelay = 500.0;
    spawnTicker = 0.0;
    //autoFireTicker = 0.0;

    towerHp = 250.0;
    towerMaxHp = 250.0;
    hpBarID = -1;
    hpBarBgID = -1;

    base.baseTexID = -1;
    base.turretBaseTexID = -1;
    statBtnID = -1;

    base.coins = 0;
    CoinTxID = -1;
    CoinTextTx = nullptr;

    currentWave = 1;
    nextWaveCounter = 0;
    nextWaveReq = 5;
    WaveTextTx = nullptr;

    initStats();
    fonts[0] = TTF_OpenFont("slkscr.ttf", 8);
    fonts[1] = TTF_OpenFont("slkscr.ttf", 16);
    fonts[2] = TTF_OpenFont("slkscr.ttf", 24);
    loadHiscores();
    selectedBayID = -1;

}

void Game::initStats(){
    struct statNameType{
        std::string name;
        int type;
    };
    statNameType statDefs[6] = {{"CoinDrop", 4}, {"maxHP", 5}, {"regenHP", 6}, {"SpawnRate", 7}, {"EnemySpeed", 8}, {"maxEnemies", 9}};

    int statOffset = int(SCR_H / 4.0) * 3.0;
    int rowOffset = 80 + 5;
    int max_row = 4;
    Stat* _tmpStat = nullptr;
    for(int i = 0; i < 6; i++){
        _tmpStat = new Stat(10 + (rowOffset * (i / max_row)), statOffset + ((i % max_row) * 20), 80, 16);
        _tmpStat->type = statDefs[i].type;
        statMap[statDefs[i].name] = _tmpStat;
    }
}

void Game::destroyAnims(){
    for(std::vector<Anim*>::iterator it = anims.begin(); it != anims.end(); it++){
        delete (*it);
    }
    anims.clear();
    for(std::vector<AnimInst*>::iterator it = animInstances.begin(); it != animInstances.end(); it++){
        delete (*it);
    }
    animInstances.clear();
    for(std::vector<SpriteAnimInst*>::iterator it = spriteAnimInstances.begin(); it != spriteAnimInstances.end(); it++){
        delete (*it);
    }
    spriteAnimInstances.clear();
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

void Game::destroyTurrets(){
    std::vector<Turret*>::iterator it;
    for(it = StoredTurrets.begin(); it != StoredTurrets.end(); it++){
        delete (*it);
    }
    StoredTurrets.clear();
    if(base.turretBays_2 != nullptr){
        for(int i = 0; i < 21; i++){
            base.turretBays_2[i]->turret = nullptr;
            activeTurrets[i] = nullptr;
        }
    }
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

void Game::targetEnemy(int _tIndex){
    if((_tIndex < 21) && (_tIndex >= 0)){
        targetEnemy(activeTurrets[_tIndex]);
    }else{
        std::cout << "Invalid turret index: " << _tIndex << "\n";
    }
}

void Game::targetEnemy(Turret* _t){
    if(_t != nullptr){
        Turret& t = *(_t);
        double lowestDist = t.range + (10 * t.stats[Turret::tStat::tsRANGE]->stat);
        for(std::vector<Enemy*>::iterator it = enemies.begin(); it != enemies.end(); it++){
            Enemy& E = (*(*it));
            double xd = (E.x + (E.w / 2.0)) - (t.baseRect.x + (t.baseRect.w / 2.0));
            double yd = (t.baseRect.y + (t.baseRect.h / 2.0)) - (E.y + (E.h / 2.0));
            double dist = std::abs(sqrt((xd*xd)+(yd*yd)));
            if(dist < lowestDist){
                lowestDist = dist;
                double newAngle = (std::atan2(-yd, xd) * (180.0 / M_PI)) + 90.0;
                t.angle = newAngle;
                t.target = &E;
                t.targetID = E.id;
            }
        }
    }else{
        //null turret
    }
}

void printDir(OctDir _dir){
    std::string oStr = "";
    switch(_dir){
        case OctDir::dU:
            oStr = "Up\n";
            break;
        case OctDir::dUR:
            oStr = "Up - RIGHT\n";
            break;
        case OctDir::dR:
            oStr = "RIGHT\n";
            break;
        case OctDir::dDR:
            oStr = "DOWN - RIGHT\n";
            break;
        case OctDir::dD:
            oStr = "DOWN\n";
            break;
        case OctDir::dDL:
            oStr = "DOWN - LEFT\n";
            break;
        case OctDir::dL:
            oStr = "LEFT\n";
            break;
        case OctDir::dUL:
            oStr = "Up - LEFT\n";
            break;
    }
    std::cout << oStr;
}

double nAngle = -90.0;
bool ENEMY_SPAWN_TEST_ANGLE = false;
int Game::spawnNewEnemy(){
    if(int(enemies.size()) < (10 + (statMap["maxEnemies"]->stat * 5)) && (int(enemies.size()) < MAX_ENEMIES)){
        Enemy& E = *(new Enemy());
        E.id = Enemy::getID();
        E.maxHp = 5 * (1.0 + (currentWave / 2.0));
        E.hp = 5 * (1.0 + (currentWave / 2.0));
        double newAngle = ((rand() % 360) * _pi_over_180);
        if(ENEMY_SPAWN_TEST_ANGLE){
            newAngle = nAngle * _pi_over_180;
            nAngle += 18.0;
        }
        E.walkingAngleR = newAngle + (M_PI / 2.0);
        E.facingDir = RadToDir(E.walkingAngleR);
        double spawnRadius = LEVEL_W / 4.0;
        E.x = (int(LEVEL_W / 2.0) - 8) + (spawnRadius * std::cos(newAngle));
        E.y = (int(LEVEL_H / 2.0) - 8) + (spawnRadius * std::sin(newAngle));
        E.w = 32.0;
        E.h = 32.0;
        E.animInst = spawnSpriteAnim(walkAnims[int(E.facingDir)], SDL_Rect{int(E.x), int(E.y), int(E.w), int(E.h)}, int(E.w), int(E.h), 0.0, SDL_Point{0,0}, true, 1.0);
        E.spd = 32.0;
        E.coinDrop = 1 + (currentWave / 2.0);
        E.atkDelay = 1000.0;
        E.dmg = 1 + (currentWave / 2.5);
        E.aspd = 0.5;

        if((currentWave % 10) == 0){
            E.type = Enemy::Types::tBoss;
            E.w *= 4;
            E.h *= 4;
            E.maxHp *= 20;
            E.hp *= 20;
            E.spd *= 0.5;
            E.dmg *= 5;
            E.coinDrop *= 10;
        }else{
            if(((currentWave - 3) % 10) == 0){
                E.type = Enemy::Types::tFast;
                E.spd *= 2;
            }else if(((currentWave + 3) % 10) == 0){
                E.type = Enemy::Types::tHeavy;
                E.w *= 2;
                E.h *= 2;
                E.maxHp *= 2;
                E.hp *= 2;
                E.spd *= 0.75;
            }else{
                E.type = Enemy::Types::tRegular;
            }
        }

        enemies.push_back(&E);
        return E.id;
    }
    return -1;
}

Game::~Game(){
    clearVectors();
    renderer = nullptr;
    ScrSurface = nullptr;
    LevelSurface = nullptr;
    fonts[0] = nullptr;
    fonts[1] = nullptr;
    fonts[2] = nullptr;
    muzzleAnim = nullptr;
    impactAnim = nullptr;
    CoinTextTx = nullptr;
    WaveTextTx = nullptr;
}

void Game::newStatBtn(Stat* _stat, int _texID, std::string _toolTip){
    _stat->btnID = statBtnID;
    SDL_Texture* _tooltipTex = renderText(_toolTip, {0,0,0}, renderer, fonts[1]);
    buttons.push_back(new Button(_stat->btnTexRect, &statUpgF, this, _stat, _tooltipTex));
    SDL_Color _col;
    if(_stat->upgradeCost <= base.coins){
        _col = {21, 111, 48};
    }else{
        _col = {208, 0, 0};
    }
    _stat->textTex = renderText(" " + std::to_string(_stat->level) + " : " + std::to_string(_stat->upgradeCost), _col, renderer, fonts[1]);
    _stat->iconID = _texID;
}

void Game::newStatBtn(Stat* _stat, std::string _iconPath, std::string _toolTip){
    _stat->iconID = newTexture(_iconPath);
    newStatBtn(_stat, _stat->iconID, _toolTip);
}

Button* Game::newStatForTurret(Stat** _stat, int _texID, std::string _toolTip){
    (*_stat) = new Stat(0, 0, 0, 0);
    (*_stat)->btnID = statBtnID;
    SDL_Texture* _tooltipTex = renderText(_toolTip, {0,0,0}, renderer, fonts[1]);
    Button* _B = new Button((*_stat)->btnTexRect, &statBaseUpgF, this, (*_stat), _tooltipTex);
    buttons.push_back(_B);
    SDL_Color _col;
    if((*_stat)->upgradeCost <= base.coins){
        _col = {21, 111, 48};
    }else{
        _col = {208, 0, 0};
    }
    (*_stat)->textTex = renderText(" " + std::to_string((*_stat)->level) + " : " + std::to_string((*_stat)->upgradeCost), _col, renderer, fonts[1]);
    (*_stat)->iconID = _texID;
    return _B;
}

void Game::updateCoins(int _coins){
    base.coins += _coins;
    CoinTextTx = renderText(std::to_string(base.coins), {0,0,0}, renderer, fonts[2]);
}

void Game::updateWaves(){
    nextWaveCounter++;
    if(nextWaveCounter >= nextWaveReq){
        currentWave++;
        if((currentWave % 10) == 0){
            nextWaveReq = 1;
        }
        if(((currentWave - 1) % 5) == 0){
            nextWaveReq = (1 + (currentWave / 5)) * 5;
        }
        nextWaveCounter = 0;
    }
    std::string tStr = "Wave " + std::to_string(currentWave) + " (" + std::to_string(nextWaveCounter) + " / " + std::to_string(nextWaveReq) + ")";
    WaveTextTx = renderText(tStr, {0,0,0}, renderer, fonts[2]);
}

void Game::nextWaveBtnF(){
    nextWaveCounter = 0;
    std::string tStr = "Wave " + std::to_string(currentWave) + " (" + std::to_string(nextWaveCounter) + " / " + std::to_string(nextWaveReq) + ")";
    WaveTextTx = renderText(tStr, {0,0,0}, renderer, fonts[2]);
}

void Game::newGameBtn(){
    updateState(gsGameStart);
    destroyTextures();
    destroyButtons();
}

int Game::newTexture(std::string _path, bool _inImages){
    int retID = -1;
    std::string path = getPath();
    if(_inImages){
         path += "images\\";
    }
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
        if(textures[_id] != nullptr){
            SDL_RenderCopy(renderer, textures[_id], NULL, &_dest);
        }
    }
}

void Game::drawTexture(int _id, SDL_Rect _dest, double _angle){
    if(_id < int(textures.size())){
        if(textures[_id] != nullptr){
            SDL_RenderCopyEx(renderer, textures[_id], NULL, &_dest, _angle, NULL, SDL_FLIP_NONE);
        }
    }
}

void Game::drawTurret(int _id, SDL_Rect _dest, double _angle, SDL_Point _offset, double _range, double _swapScale){
    if(_id < int(textures.size())){
        SDL_Rect _scaledRect = _dest;
        _scaledRect.w = double(_scaledRect.w) * _swapScale;
        _scaledRect.h = double(_scaledRect.h) * _swapScale;
        _scaledRect.x = _scaledRect.x + ((_dest.w - _scaledRect.w) / 2.0);
        _scaledRect.y = _scaledRect.y + ((_dest.h - _scaledRect.h) / 2.0);
        SDL_RenderCopyEx(renderer, textures[_id], NULL, &_scaledRect, _angle, &_offset, SDL_FLIP_NONE);
    }
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    helper::DrawCircle(renderer, _dest.x + (_dest.w / 2.0), _dest.y + 20 + (_dest.h / 2.0), _range);
}

void Game::updateStatText(Stat& _stat){
    SDL_DestroyTexture(_stat.textTex);
    std::string _tLvl;
    std::string _tUpgC;

    _tLvl = NumToKNum(_stat.level);
    _tUpgC = NumToKNum(_stat.upgradeCost);

    SDL_Color _col;
    if(_stat.upgradeCost <= base.coins){
        _col = {21, 111, 48};
    }else{
        _col = {208, 0, 0};
    }
    _stat.textTex = renderText(_tLvl + ": " + _tUpgC, _col, renderer, fonts[1]);
}

void Game::statUpgF(Stat& _stat, bool _texOnly = false){
    if(!_texOnly){
        if(_stat.upgrade(base.coins)){
            updateCoins(-_stat.upgradeCost);
        }
    }
    SDL_DestroyTexture(_stat.textTex);
    std::string _tLvl;
    std::string _tUpgC;

    _tLvl = NumToKNum(_stat.level);
    _tUpgC = NumToKNum(_stat.upgradeCost);

    SDL_Color _col;
    if(_stat.upgradeCost <= base.coins){
        _col = {21, 111, 48};
    }else{
        _col = {208, 0, 0};
    }
    _stat.textTex = renderText(_tLvl + ": " + _tUpgC, _col, renderer, fonts[1]);
}

void Game::statBaseUpgF(Stat& _stat, bool _texOnly){
    if(base.turretBays_2[selectedBayID]->turret != nullptr){
        statUpgF(_stat, _texOnly);
    }

}

void Game::drawUpgBtns(){
    for(std::map<std::string, Stat*>::iterator it = statMap.begin(); it != statMap.end(); it++){
        Stat& s = *((*it).second);
        statUpgF(s, true);
        drawTexture(s.btnID, s.btnTexRect);
        drawTexture(base.turretBaseTexID, s.iconRect);
        drawTexture(s.iconID, s.iconRect);
        int txw, txh;
        SDL_QueryTexture(s.textTex, NULL, NULL, &txw, &txh);
        s.texRect.w = txw * 0.8;
        s.texRect.h = txh;
        SDL_RenderCopy(renderer, s.textTex, NULL, &(s.texRect));
    }
}

AnimInst* Game::spawnAnim(Anim* _anim, SDL_Rect _pos, double _angle, SDL_Point _offset, bool _loop, double _speed){
    AnimInst* _A = new AnimInst(_anim, _pos, _angle, _offset, _loop, _speed);
    return _A;
}

SpriteAnimInst* Game::spawnSpriteAnim(Anim* _anim, SDL_Rect _pos, int _w, int _h, double _angle, SDL_Point _offset, bool _loop, double _speed){
    SpriteAnimInst* _A = new SpriteAnimInst(_anim, _pos, _w, _h, _angle, _offset, _loop, _speed);
    return _A;
}

void Game::fireTurret(int _turretID){
    if((_turretID >= 0) && (_turretID < 21)){
        fireTurret(activeTurrets[_turretID]);
    }else{
        std::cout << "FireTurret: Invalid turret ID: " << _turretID << "\n";
    }
}

void Game::fireTurret(Turret* _t){
    if(_t != nullptr){
        Turret& t = *(_t);
        if((t.target != nullptr) && (Enemy::checkID(t.targetID) == true)){
            if(t.target->takeDamage(t.stats[Turret::tStat::tsDMG]->stat)){
                t.target = nullptr;
            }
            SDL_Rect pos = {t.cannonRect.x - 4, t.cannonRect.y - 32, 32, 32};
            SDL_Point offset = t.pivotOffset;
            offset.x += 4;
            offset.y += 32;
            animInstances.push_back(spawnAnim(muzzleAnim, pos, t.angle, offset, false, 1.0));
        }
    }else{
        std::cout << "FireTurret: Null pointer\n";
    }
}

void Game::drawBase(){
    drawTexture(base.baseTexID, base.pos);
    for(int i = 0; i < 21; i++){
        if(i == selectedBayID){
            SDL_Rect _selectedRect = base.turretBays_2[i]->turret_Rect;
            _selectedRect.w = double(_selectedRect.w) * 1.5;
            _selectedRect.h = double(_selectedRect.h) * 1.5;
            _selectedRect.x -= double(_selectedRect.w) / 6;
            _selectedRect.y -= double(_selectedRect.h) / 6;
            drawTexture(base.turretBaseTexID, _selectedRect);
        }else{
            drawTexture(base.turretBaseTexID, base.turretBays_2[i]->turret_Rect);
        }
    }
}

void Game::clearVectors(){
    destroyButtons();
    destroyEnemies();
    destroyTextures();
    destroyStats();
    destroyTurrets();
}

void Game::newTurret(Turret::turretType _turretType){
    int _newTurretID = base.getFreeBase();
    if((_newTurretID != -1) && (_newTurretID < 21)){
        double _x = base.turretBays_2[_newTurretID]->turret_Rect.x;
        double _y = base.turretBays_2[_newTurretID]->turret_Rect.y;
        Turret* t = new Turret(_x, _y, _turretType);
        activeTurrets[_newTurretID] = t;
        base.turretBays_2[_newTurretID]->turret = t;

        Button* statButtonPtr = newStatForTurret(&(t->stats[Turret::tsDMG]), dmgIconID, std::string("+ Dmg "));
        t->setStatButton(Turret::tsDMG, statButtonPtr);

        statButtonPtr = newStatForTurret(&(t->stats[Turret::tsAUTOFIRE]), autofireIconID, std::string("+ FireRate "));
        t->setStatButton(Turret::tsAUTOFIRE, statButtonPtr);

        statButtonPtr = newStatForTurret(&(t->stats[Turret::tsRANGE]), fireRangeIconID, std::string("+ Range "));
        t->setStatButton(Turret::tsRANGE, statButtonPtr);

        t->statButtons[Turret::tsDMG]->active = false;
        t->statButtons[Turret::tsAUTOFIRE]->active = false;
        t->statButtons[Turret::tsRANGE]->active = false;
    }
}

void Game::log(std::string _string){
    _logger->log(_string);
}

bool Game::update(double dTime){
    bool updateLog = false;
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
    SDL_SetRenderTarget(renderer, ScrSurface);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 240, 240, 0, 255);
    SDL_SetRenderTarget(renderer, LevelSurface);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, 0);
    SDL_RenderClear(renderer);

    SDL_Event e;

    tick = false;
    if(!paused){
        fpsTicker -= dTime;
        if(fpsTicker < 0){
            fpsTicker = 1000.0 / fps;
            tick = true;
        }

        if(clickBounce > 0.0){
            clickBounce -= dTime;
        }
        healTicker -= dTime;
        if(healTicker < 0){
            healTicker = healDelay;
            double healAmt = (statMap["regenHP"]->stat * 0.25);
            if((towerHp + healAmt) >= towerMaxHp){
                towerHp = towerMaxHp;
            }else{
                towerHp += healAmt;
            }
        }
    }
    lClick = false;

    handleEvents(e);
    updateButtons(dTime);
    if(state == gsInit){
        base.initTurretBases();
        clearVectors();
        init();
        logoID = newTexture("TowerDefence.png");
        ngbID = newTexture("NewGame.png");
        SDL_Texture* _ttTex = renderText("New Game Button", {0,0,0}, renderer, fonts[2]);
        buttons.push_back(new Button(ngbRect, &newGameBtn, this, _ttTex));
        towerHp = 250;
        towerMaxHp = 250;
        updateState(gsMainMenu);
    }else if(state == gsMainMenu){
        SDL_SetRenderTarget(renderer, ScrSurface);
        drawTexture(logoID, logoRect);
        drawTexture(ngbID, ngbRect);

        SDL_Rect scoreRect = {int((SCR_W / 2.0) - (SCR_W / 8.0)), int((SCR_H / 16.0) * 7.0), int(SCR_W / 4.0), int(SCR_H / 32.0)};
        SDL_RenderCopy(renderer, hiscoreTexs[0], NULL, &scoreRect);
        scoreRect.y = int((SCR_H / 16.0) * 8.0);
        SDL_RenderCopy(renderer, hiscoreTexs[1], NULL, &scoreRect);
        scoreRect.y = int((SCR_H / 16.0) * 9.0);
        scoreRect.x += int((scoreRect.w * 0.1));
        scoreRect.w = int(scoreRect.w * 0.8);
        scoreRect.h = int(scoreRect.h * 0.9);
        SDL_RenderCopy(renderer, hiscoreTexs[2], NULL, &scoreRect);
        scoreRect.y = int((SCR_H / 16.0) * 10.0);
        scoreRect.x += int(scoreRect.w * 0.1);
        scoreRect.w = int(scoreRect.w * 0.8);
        scoreRect.h = int(scoreRect.h * 0.9);
        SDL_RenderCopy(renderer, hiscoreTexs[3], NULL, &scoreRect);
        SDL_SetRenderTarget(renderer, 0);
    }else if(state == gsGameStart){
        std::string path = getPath();
        nextWaveBtnID = newTexture("nextWave.png");
        diedSplashID = newTexture("diedSplash.png");
        pausedSplashID = newTexture("pausedSplash.png");

        redEnemyID = newTexture("redEnemy.png");
        blueEnemyID = newTexture("blueEnemy.png");
        greenEnemyID = newTexture("greenEnemy.png");
        grassBGID = newTexture("grass_bg.jpg");

        hpBarID = newTexture("hpBar.png");
        hpBarBgID = newTexture("hpBar_bg.png");
        Enemy::initEnemies(MAX_ENEMIES);

        turretBaseID = newTexture("td_basic_towers\\Tower.png", false);
        turretCannonIDs[0] = newTexture("td_basic_towers\\MG3.png", false);
        turretCannonIDs[1] = newTexture("td_basic_towers\\Missile_Launcher.png", false);
        turretCannonIDs[2] = newTexture("td_basic_towers\\Cannon.png", false);
        base.baseTexID = newTexture("circle_fence.png");
        base.turretBaseTexID = newTexture("icon_Base.png");
        muzzleFlashID = newTexture("MuzzleFlash.png");

        impactAnimIDs[0] = newTexture("impact_0.png");
        impactAnimIDs[1] = newTexture("impact_1.png");
        impactAnimIDs[2] = newTexture("impact_2.png");
        impactAnimIDs[3] = newTexture("impact_3.png");

        walkAnimIDs[0] = newTexture("Walk_Sprites\\Z1WalkUp.png", false);
        walkAnimIDs[1] = newTexture("Walk_Sprites\\Z1WalkUpRight.png", false);
        walkAnimIDs[2] = newTexture("Walk_Sprites\\Z1WalkRight.png", false);
        walkAnimIDs[3] = newTexture("Walk_Sprites\\Z1WalkDownRight.png", false);
        walkAnimIDs[4] = newTexture("Walk_Sprites\\Z1WalkDown.png", false);
        walkAnimIDs[5] = newTexture("Walk_Sprites\\Z1WalkDownLeft.png", false);
        walkAnimIDs[6] = newTexture("Walk_Sprites\\Z1WalkLeft.png", false);
        walkAnimIDs[7] = newTexture("Walk_Sprites\\Z1WalkUpLeft.png", false);

        CoinTxID = newTexture("coin.png");
        updateCoins(100);
        WaveTextTx = renderText("Wave " + std::to_string(currentWave) + " (" + std::to_string(nextWaveCounter) + " / " + std::to_string(nextWaveReq) + ")", {0,0,0}, renderer, fonts[2]);

        statBtnID = newTexture("button_turq.png");

        dmgIconID = newTexture("bullet.png");
        newStatBtn(statMap["CoinDrop"], CoinTxID, "+ Coin Drop");
        newStatBtn(statMap["SpawnRate"], "Hourglass.png", "- Spawn Delay");
        newStatBtn(statMap["EnemySpeed"], "speedIcon.png", "+ Enemy Speed");
        autofireIconID = newTexture("autofireIcon.png");
        fireRangeIconID = newTexture("rangeIcon.png");
        newStatBtn(statMap["maxEnemies"], "maxEnemyIcon.png", "+ Max Enemies");
        newStatBtn(statMap["regenHP"], "regenHPIcon.png", "+ HP Regen");
        newStatBtn(statMap["maxHP"], "maxHPIcon.png", "+ Max HP");

        muzzleAnim = new Anim{30.0};
        muzzleAnim->addFrameTx(textures[muzzleFlashID]);
        muzzleAnim->addFrameTx(textures[muzzleFlashID]);

        impactAnim = new Anim{30.0};
        impactAnim->addFrameTx(textures[impactAnimIDs[0]]);
        impactAnim->addFrameTx(textures[impactAnimIDs[1]]);
        impactAnim->addFrameTx(textures[impactAnimIDs[2]]);
        impactAnim->addFrameTx(textures[impactAnimIDs[3]]);

        for(int i = 0; i < 8; i++){
            walkAnims[i] = new Anim(10.0);
            walkAnims[i]->addFrameTx(textures[walkAnimIDs[i]]);
            walkAnims[i]->addFrameTx(textures[walkAnimIDs[i]]);
        }

        int _turretCount = 3;
        for(int i = 0; i < _turretCount; i++){
            newTurret(static_cast<Turret::turretType>(i%3));
        }
        paused = false;
        updateState(gsGame);
    }else if(state == gsGame){
        SDL_SetRenderTarget(renderer, LevelSurface);
if(updateLog){log("Draw BG");};
        drawBG();
        towerMaxHp = 240 + (statMap["maxHP"]->stat * 10);
        if((lClick) && !(base.isSwapping)){
            if(!checkBayClicks()){
                if(selectedBayID != -1){
                    if(activeTurrets[selectedBayID] != nullptr){
                        activeTurrets[selectedBayID]->deactivateButtons();
                    }
                    selectedBayID = -1;
                }

            }
        }
if(updateLog){log("Updt Base");};
        std::vector<SDL_Rect*> upgProgressRects;
        int baseState = base.update(dTime, &upgProgressRects);
if(updateLog){log("Updt Stats");};
        for(std::map<std::string, Stat*>::iterator it = statMap.begin(); it != statMap.end(); it++){
            (*it).second->update(dTime);
        }
        if((baseState & Base::baseUpdState::usHALF_SWAP) == Base::baseUpdState::usHALF_SWAP){
            int _t1 = base.swapT1ID;
            int _t2 = base.swapT2ID;
            if(activeTurrets[_t1] == nullptr){
                activeTurrets[_t1] = activeTurrets[_t2];
                activeTurrets[_t2] = nullptr;
            }else if(activeTurrets[_t2] == nullptr){
                activeTurrets[_t2] = activeTurrets[_t1];
                activeTurrets[_t1] = nullptr;
            }else{
                Turret* _T1 = activeTurrets[_t1];
                activeTurrets[_t1] = activeTurrets[_t2];
                activeTurrets[_t2] = _T1;
            }

            if(activeTurrets[_t1] != nullptr){
                activeTurrets[_t1]->moveTurret(base.turretBays_2[_t1]->turret_Rect.x, base.turretBays_2[_t1]->turret_Rect.y);
            }
            if(activeTurrets[_t2] != nullptr){
                activeTurrets[_t2]->moveTurret(base.turretBays_2[_t2]->turret_Rect.x, base.turretBays_2[_t2]->turret_Rect.y);
            }
        }
        if((baseState & Base::baseUpdState::usSWAP_FINISH) == Base::baseUpdState::usSWAP_FINISH){
            //selectedBayID = -1;
        }
        if((baseState & Base::baseUpdState::usFINISH_UPGRADE) == Base::baseUpdState::usFINISH_UPGRADE){
            //
        }
if(updateLog){log("Draw Base");};
        drawBase();

        for(int i = 0; i < 21; i++){
            if(activeTurrets[i] != nullptr){
                Turret& t = *(activeTurrets[i]);
                drawTexture(turretBaseID, t.baseRect);
                int tcID = int(t.type);
                drawTurret(turretCannonIDs[tcID], t.cannonRect, t.angle, t.pivotOffset, t.range + (t.stats[Turret::tsRANGE]->stat * 10), t.swapScale);

                if(!paused){
                    t.autoFireTicker -= dTime;
                    if((t.autoFireTicker <= 0) && !(gameOver)){
                        if((t.targetID != -1) && (Enemy::checkID(t.targetID) == true)){
                            fireTurret(i);
                            t.autoFireTicker = t.autoFireDelay * (1.0 / t.stats[Turret::tsAUTOFIRE]->stat);
                        }else{
                            targetEnemy(i);
                        }
                    }
                }
            }
        }
        if((baseState & Base::baseUpdState::usUPGRADE) == Base::baseUpdState::usUPGRADE){
            for(std::vector<SDL_Rect*>::iterator it = upgProgressRects.begin(); it != upgProgressRects.end(); it++){
                drawTexture(hpBarID, (*(*it)));
            }
        }
if(updateLog){log("Select Bay");};
        if((selectedBayID != -1) && !(base.turretBays_2[selectedBayID]->isSwapping)){
            if(base.turretBays_2[selectedBayID]->turret != nullptr){
                if(!(activeTurrets[selectedBayID]->buttonsActive)){
                    activeTurrets[selectedBayID]->activateButtons();
                    for(int i = 0; i < 3; i++){
                        double _angle = ((120.0 * i) + 90) * _pi_over_180;
                        double _x = base.turretBays_2[selectedBayID]->turret_Rect.x + (base.turretBays_2[selectedBayID]->turret_Rect.w / 2) - 40 + ((base.turretBays_2[selectedBayID]->turret_Rect.w) * std::cos(_angle));
                        double _y = base.turretBays_2[selectedBayID]->turret_Rect.y + (base.turretBays_2[selectedBayID]->turret_Rect.h / 2) - 16 + ((base.turretBays_2[selectedBayID]->turret_Rect.h) * std::sin(_angle));
                        SDL_Rect _dest = {int(_x), int(_y), 80, 16};
                        Stat* s = activeTurrets[selectedBayID]->stats[i];
                        s->moveStat(_dest);
                        _x = base.turretBays_2[selectedBayID]->turret_Rect.x - (base.turretBays_2[selectedBayID]->turret_Rect.w / 2);
                        _y = base.turretBays_2[selectedBayID]->turret_Rect.y - (base.turretBays_2[selectedBayID]->turret_Rect.h / 2);
                        levelToScreen(_x, _y);
                        _dest.x = _x + ((base.turretBays_2[selectedBayID]->turret_Rect.w * viewScale) * std::cos(_angle)) - 20;
                        _dest.y = _y + ((base.turretBays_2[selectedBayID]->turret_Rect.h * viewScale) * std::sin(_angle)) - 8;
                        activeTurrets[selectedBayID]->statButtons[i]->moveButton(_dest);
                    }
                }
                SDL_SetRenderTarget(renderer, LevelSurface);
                for(int i = 0; i < 3; i++){
                    Stat* s = activeTurrets[selectedBayID]->stats[i];
                    statUpgF(*s, true);
                    drawTexture(s->btnID, s->btnTexRect);
                    drawTexture(base.turretBaseTexID, s->iconRect);
                    drawTexture(s->iconID, s->iconRect);
                    int txw, txh;
                    SDL_QueryTexture(s->textTex, NULL, NULL, &txw, &txh);
                    s->texRect.w = txw * 0.8;
                    s->texRect.h = txh;
                    SDL_RenderCopy(renderer, s->textTex, NULL, &(s->texRect));
                }
                SDL_SetRenderTarget(renderer, LevelSurface);
            }else{
                //
            }
        }

        if(!paused){
if(updateLog){log("Spawn Ticker");};
            if(spawnTicker > 0.0){
                spawnTicker -= dTime;
            }else{
                if(int(enemies.size()) < (nextWaveReq - nextWaveCounter)){
                    int _randEnemyAmt = (rand() % ((currentWave / 5) + 1)) - 1;
                    if(_randEnemyAmt < 0){
                        _randEnemyAmt = 0;
                    }
                    for(int i = 0; i <= _randEnemyAmt; i++){
                        spawnNewEnemy();
                    }
                    spawnTicker = spawnDelay / statMap["SpawnRate"]->stat;
                }
            }

            if((lClick) && !(gameOver)){
if(updateLog){log("Active Fire");};
                for(int i = 0; i < 21; i++){
                    if(activeTurrets[i] != nullptr){
                        Turret& t = *(activeTurrets[i]);
                        if((t.targetID == -1) || (Enemy::checkID(t.targetID) == false)){
                            targetEnemy(&t);
                        }else{
                            fireTurret(&t);
                        }
                    }
                }
            }
        }

        SDL_SetRenderTarget(renderer, LevelSurface);
if(updateLog){log("Enemy Loop");};
        for(std::vector<Enemy*>::iterator it = enemies.begin(); it != enemies.end();){
            if((*it) == nullptr){
                it++;
                break;
            }
            Enemy& E = *(*it);
            if(E.dying){
                updateCoins(E.coinDrop * (1.0 + (statMap["CoinDrop"]->stat - 1)));
                for(int i = 0; i < 21; i++){
                    if(activeTurrets[i] != nullptr){
                        Turret& t = *(activeTurrets[i]);
                        if(&E == t.target){
                            t.target = nullptr;
                        }
                    }
                }
                if(E.type == Enemy::Types::tBoss){
                    newTurret(Turret::turretType::tAutoGun);
                }
                Enemy::clearID(E.id);
                delete (*it);
                it = enemies.erase(it);
                updateWaves();
            }else{
                if((!gameOver) && (!paused)){
                    E.update(dTime, statMap["EnemySpeed"]->stat);
                    if(E.isAttacking){
                        double _angle = E.walkingAngleR + (M_PI / 2.0);
                        double _x = E.x - 32 + (E.h * std::cos(_angle));
                        double _y = E.y - 32 + (E.h * std::sin(_angle));
                        SDL_Rect pos = {int(_x), int(_y), 64, 64};
                        SDL_Point offset = {0,0};
                        animInstances.push_back(spawnAnim(impactAnim, pos, 0.0, offset, false, 0.5));
                        towerHp -= E.dmg;
                        if(towerHp <= 0){
                            gameOver = true;
                        }
                    }
                }

                Turret* t = nullptr;
                while(cycleTurrets(&t)){
                    if(t != nullptr){
                        if(t->target != nullptr){
                            if(&E == t->target){
                                helper::DrawCircle(renderer, E.x + (E.w / 2.0), E.y + (E.h / 2.0), E.w);
                                break;
                            }
                        }
                    }
                }

                SDL_Rect enemyRect = {int(E.x), int(E.y), int(E.w), int(E.h)};
                SDL_Rect vRect = {int((LEVEL_W / 2.0) - (ViewRect.w / 2.0)), int((LEVEL_H / 2.0) - (ViewRect.h / 2.0)), ViewRect.w, ViewRect.h};
                //double _angle = 360.0 - ViewAngle;
                int _texID = -1;
                _texID = walkAnimIDs[int(E.facingDir)];
                if(_texID != -1){
                    SDL_Rect tR = {E.animInst->currentFrame * E.animInst->w, 0, 32, 32};
                    if(((E.x + E.w + 1) > vRect.x) || ((E.x + 1) < (vRect.x + vRect.w)) ||
                       ((E.y + E.h + 9) > vRect.y) || ((E.y + 9) < (vRect.y + vRect.h))){
                        if(textures[_texID] != nullptr){
                            //yellow glitch caused by rendercopy || drawtexture
                            SDL_RenderCopy(renderer, textures[_texID], &tR, &enemyRect);
                            SDL_Rect hpRect = enemyRect;
                            hpRect.y -= 8;
                            hpRect.h = 4;
                            hpRect.w = int(hpRect.w * double(double(E.hp) / double(E.maxHp)));
                            drawTexture(hpBarID, hpRect);
                        }else{
                            std::cout << "Null Texture\n";
                        }
                    }
                }else{
                    std::cout << "Invalid texID\n";
                }
                it++;
            }
        }
if(updateLog){log("Anim");};
        for(std::vector<SpriteAnimInst*>::iterator it = spriteAnimInstances.begin(); it != spriteAnimInstances.end();){
            SpriteAnimInst& s = *(*it);
            SDL_Rect _sXY = {s.w * s.currentFrame, 0, s.w, s.h};
            SDL_RenderCopyEx(renderer, s.anim->frames[0], &_sXY, &s.pos, s.angle, &s.offset, SDL_FLIP_NONE);

            if(!paused){
                if(s.update(dTime)){
                    delete (*it);
                    it = spriteAnimInstances.erase(it);
                }else{
                    it++;
                }
            }
        }

        for(std::vector<AnimInst*>::iterator it = animInstances.begin(); it != animInstances.end();){
            AnimInst& A = *(*it);
            SDL_RenderCopyEx(renderer, A.anim->frames[A.currentFrame], NULL, &A.pos, A.angle, &A.offset, SDL_FLIP_NONE);

            if(!paused){
                if(A.update(dTime)){
                    delete (*it);
                    it = animInstances.erase(it);
                }else{
                    it++;
                }
            }
        }

        SDL_SetRenderTarget(renderer, ScrSurface);
if(updateLog){log("Draw Upg Btns");};
        drawUpgBtns();

        SDL_Rect towerhpRect = {int(SCR_W / 2.0) - 129, 39, 258, 18};
        drawTexture(hpBarBgID, towerhpRect);
        towerhpRect = {int(SCR_W / 2.0) - 128, 40, int(256 * double(double(towerHp) / double(towerMaxHp))), 16};
        drawTexture(hpBarID, towerhpRect);

        int txw, txh;
        drawTexture(CoinTxID, CoinIconRect);
        SDL_QueryTexture(CoinTextTx, NULL, NULL, &txw, &txh);
        CoinTextRect.w = txw;
        CoinTextRect.h = txh;
        SDL_RenderCopy(renderer, CoinTextTx, NULL, &CoinTextRect);

        SDL_QueryTexture(WaveTextTx, NULL, NULL, &txw, &txh);
        WaveTextRect.x = (int((SCR_W - txw) / 2.0));
        WaveTextRect.w = txw;
        WaveTextRect.h = txh;
        SDL_RenderCopy(renderer, WaveTextTx, NULL, &WaveTextRect);

        if(gameOver){
            btnsActive = false;
            SDL_Rect splashRect = {0, 0, SCR_W, SCR_H};
            drawTexture(diedSplashID, splashRect);
        }
    }else if(state == gsClose){
        return true;
    }

    if(paused){
        SDL_Rect _scrRect = {0, 0, SCR_W, SCR_H};
        SDL_RenderCopy(renderer, textures[pausedSplashID], NULL, &_scrRect);
    }

    SDL_SetRenderTarget(renderer, 0);
    SDL_Point _p = {int(LEVEL_W / 2.0), int(LEVEL_H / 2.0)};
    SDL_Rect scaledViewRect = ViewRect;
    scaledViewRect.w *= viewScale;
    scaledViewRect.h *= viewScale;
    scaledViewRect.x = _p.x - (scaledViewRect.w / 2.0);
    scaledViewRect.y = _p.y - (scaledViewRect.h / 2.0);
    SDL_RenderCopyEx(renderer, LevelSurface, &scaledViewRect, &ScrnRect, ViewAngle, NULL, SDL_FLIP_NONE);
    SDL_RenderCopyEx(renderer, ScrSurface, NULL, &ScrnRect, 0.0, NULL, SDL_FLIP_NONE);
    SDL_RenderPresent(renderer);

    return false;
}

void Game::setRenderer(SDL_Renderer* _renderer, SDL_Texture* _ScrSurface, SDL_Texture* _LevelSurface){
    renderer = _renderer;
    ScrSurface = _ScrSurface;
    LevelSurface = _LevelSurface;
}

void Game::updateState(GState newState){
    state = newState;
}
