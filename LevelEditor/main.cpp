#ifdef __unix__
	#include "SDL2/SDL.h"
	#include "SDL2/SDL_image.h"
#elif defined(_WIN32)
	#include "SDL.h"
	#include "SDL_image.h"
#endif

#include <stdio.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
#include "tinyfiledialogs.h"

using namespace std;

const int SCREEN_WIDTH = 1300;
const int SCREEN_HEIGHT = 640;

double spriteSheetX = 660.0;
double spriteSheetY = 0.0;
int maxLayers = 3;
int sshotCount = 0;
int maxSshot = 10;

FILE * lIn;
char lBuffer[1024];
char const * lFilterPatterns[1] = { "*.map" };

struct grid{
    double cellW;
    double cellH;
    int gridW;
    int gridH;
};

bool init(SDL_Window** win, SDL_Renderer** renderer);
bool loadMedia(std::vector<SDL_Texture*>* textures, SDL_Renderer** renderer);
void close(SDL_Window* win, std::vector<SDL_Texture*>* textures);

void save_texture(const char* file_name, SDL_Renderer* renderer, SDL_Texture* texture) {
    SDL_Texture* target = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, texture);
    int width, height;
    SDL_QueryTexture(texture, NULL, NULL, &width, &height);
    SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
    SDL_RenderReadPixels(renderer, NULL, surface->format->format, surface->pixels, surface->pitch);
    IMG_SavePNG(surface, file_name);
    SDL_FreeSurface(surface);
    SDL_SetRenderTarget(renderer, target);
}

SDL_Texture* loadTexture(std::string path, SDL_Renderer** renderer){
    SDL_Texture* newTexture = NULL;

    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if(loadedSurface == NULL){
        printf("Unable to load image %s! SDL_Image Error: %s\n", path.c_str(), IMG_GetError());
    }
    else{
        newTexture = SDL_CreateTextureFromSurface(*renderer, loadedSurface);
        if(newTexture == NULL){
            printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
        }
        SDL_FreeSurface(loadedSurface);
    }
    return newTexture;
}

struct spriteSheet{
    SDL_Texture* T;
    string fName;
};

bool init(SDL_Window** win, SDL_Renderer** renderer){
    bool success = true;
    if(SDL_Init(SDL_INIT_VIDEO)){
        printf("SDL could not initialise! SDL_Error: %s\n", SDL_GetError());
        success = false;
    }
    else{
        *win = SDL_CreateWindow("Project-1-LvlEditor", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if(*win == NULL){
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
            success = false;
        }
        else{
            *renderer = SDL_CreateRenderer(*win, -1, SDL_RENDERER_ACCELERATED);
            if(*renderer == NULL){
                printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
                success = false;
            }
            else{
                SDL_SetRenderDrawColor(*renderer, 0xff, 0xff, 0xff, 0xff);
                int imgFlags = IMG_INIT_PNG;
                if(!(IMG_Init(imgFlags)&imgFlags)){
                    printf("SDL_Image could not initialise! SDL_Image Error: %s\n", IMG_GetError());
                    success = false;
                }
            }
        }
    }
    SDL_SetRenderDrawBlendMode(*renderer, SDL_BLENDMODE_BLEND);
    return success;
}

bool loadMedia(std::vector<spriteSheet*>* ssheets, SDL_Renderer** renderer){
    bool success = true;

    SDL_Texture* T;

    //T = loadTexture("C:/Users/ricar/Documents/cppProjects/LevelEditor/Overworld.png", renderer);
    //string fname = "C:/Users/ricar/Documents/cppProjects/LevelEditor/Overworld.png";
    T = loadTexture("Overworld.png", renderer);
    string fname = "Overworld.png";

    if(T == NULL)
    {
        printf("Unable to load image. error: %s\n", SDL_GetError());
        success = false;
    }
    else{
        spriteSheet* ss = new spriteSheet;
        ss->fName = fname;
        ss->T = T;
        (*ssheets).push_back(ss);
    }

    return success;
}

void close(SDL_Window* win, std::vector<spriteSheet*>* ssheets){
    for(std::vector<spriteSheet*>::iterator it = (*ssheets).begin(); it != (*ssheets).end(); it++){
        SDL_DestroyTexture((*it)->T);
        (*it)->T = NULL;
    }
    SDL_DestroyWindow(win);
    win = NULL;
    SDL_Quit();
}

void drawGrid(SDL_Renderer* renderer, int gridW, int gridH, double cellW, double cellH, int offsetX=0, int offsetY=0){
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    for(int i = 0; i < gridW; i++){
        for(int j = 0; j < gridH; j++){
            SDL_RenderDrawLine(renderer, i*cellW + offsetX, j*cellH + offsetY, (i*cellW) + cellW + offsetX, j*cellH + offsetY);
            SDL_RenderDrawLine(renderer, i*cellW + offsetX, j*cellH + offsetY, (i*cellW) + offsetX, (j*cellH) + cellH + offsetY);
        }
    }
    SDL_RenderDrawLine(renderer, gridW*cellW + offsetX, offsetY, gridW*cellW + offsetX, gridH*cellH + offsetY);
    SDL_RenderDrawLine(renderer, offsetX, gridH*cellH + offsetY, gridW*cellW + offsetX, gridH*cellH + offsetY);
}

struct vec2{
    int x, y;
    friend bool operator==(const vec2& lhs, const vec2& rhs){
        if((lhs.x == rhs.x) && (lhs.y == rhs.y)){
            return true;
        }
        return false;
    }
    friend bool operator!=(const vec2& lhs, const vec2& rhs){
        if((lhs.x == rhs.x) && (lhs.y == rhs.y)){
            return false;
        }
        return true;
    }
    void operator=(const vec2 &v){
         x = v.x;
         y = v.y;
    }
};

enum screenArea{
    MAP,
    SPRITE
};

struct layer{
    vec2 layerMap[40][40];
    grid layerGrid;
    layer(){
        layerGrid.cellW = 16.0;
        layerGrid.cellH = 16.0;
        layerGrid.gridW = 40;
        layerGrid.gridH = 40;
        for(int i = 0; i < 40; i++){
            for(int j = 0; j < 40; j++){
                layerMap[i][j].x = -1;
                layerMap[i][j].y = -1;
            }
        }
    }
};

void saveTiles(vec2 levelMap[40][40], grid mGrid, string inFname, SDL_Renderer* renderer, string imgFname){
    vector<vec2> tileList;
    vec2 eVec;
    eVec.x = -1;
    eVec.y = -1;
    for(int i = 0; i < 40; i++){
        for(int j = 0; j < 40; j++){
            if((levelMap[i][j].x == -1) || (levelMap[i][j].y == -1)){
            }else{
                bool newVec = true;
                for(vector<vec2>::iterator it = tileList.begin(); it != tileList.end(); it++){
                    if(((*it).x == levelMap[i][j].x) && ((*it).y == levelMap[i][j].y)){
                        newVec = false;
                    }
                }
                if(newVec){
                    tileList.push_back(levelMap[i][j]);
                }
            }
        }
    }
    int sqr = floor(sqrt(tileList.size()));
    int counter = 0;
    int counter2 = 0;
    vec2 newGrid[sqr][sqr];
    for(vector<vec2>::iterator it = tileList.begin(); it != tileList.end(); it++){
        vec2& v = *it;
        newGrid[counter][counter2] = v;
        counter++;
        if(counter == sqr){
            counter = 0;
            counter2++;
        }
    }
    SDL_Surface* loaded = IMG_Load(inFname.c_str());
    if(loaded == NULL){
        cout << "Null loaded.\n";
    }
    SDL_Surface* sshot = SDL_CreateRGBSurface(0,sqr*mGrid.cellW,sqr*mGrid.cellH,32,0,0,0,0);
    SDL_FillRect(sshot, NULL, 0xFFFFFF);
    for(int i = 0; i < sqr; i++){
        for(int j = 0; j < sqr; j++){
            if((newGrid[i][j].x == -1) && (newGrid[i][j].y == -1)){
                //
            }else{
                SDL_Rect src;
                src.x = newGrid[i][j].x * mGrid.cellW;
                src.y = newGrid[i][j].y * mGrid.cellH;
                src.w = mGrid.cellW;
                src.h = mGrid.cellH;
                SDL_Rect dest;
                dest.x = i * mGrid.cellW;
                dest.y = j * mGrid.cellH;
                dest.w = mGrid.cellW;
                dest.h = mGrid.cellH;
                if(SDL_BlitSurface(loaded, &src, sshot, &dest) != 0){
                    printf("Blit error. %s\n", IMG_GetError());
                }
            }
        }
    }
    SDL_Texture* newTexture = SDL_CreateTextureFromSurface(renderer, sshot);
    SDL_RenderCopy(renderer, newTexture, NULL, NULL);
    if(newTexture!=NULL){
        string newFname = imgFname.substr(0, imgFname.find_last_of("."));
        newFname += "_tiles.png";
        IMG_SavePNG(sshot, newFname.c_str());
    }
    SDL_FreeSurface(loaded);
    SDL_DestroyTexture(newTexture);
    newTexture = NULL;
}


void saveMap(vector<layer*>& layerVec, string inFname, SDL_Renderer* renderer){
	char const * lTheSaveFileName = tinyfd_saveFileDialog(
		"Save .map file",
		"",
		1,
		lFilterPatterns,
		"Maps");
	
	int gridW = layerVec[0]->layerGrid.gridW;
    int gridH = layerVec[0]->layerGrid.gridH;
    int cellW = layerVec[0]->layerGrid.cellW;
    int cellH = layerVec[0]->layerGrid.cellH;
	
	string fName = lTheSaveFileName;
	string imgFname = fName.substr(0, fName.find_last_of("."));
	imgFname += ".png";
	fName = fName.substr(fName.find_last_of("\\") + 1);
	SDL_Surface* loaded = IMG_Load(inFname.c_str());
	if(loaded == NULL){
		cout << "Null loaded.\n";
	}
	SDL_Surface* sshot = SDL_CreateRGBSurface(0,gridW*cellW,gridH*cellH,32,0,0,0,0);
	SDL_FillRect(sshot, NULL, 0xFFFFFF);
	if(fName != ""){
		ofstream ofile(fName);
		ofile << inFname << '\n';
		ofile << gridW << "," << gridH << "," << cellW << "," << cellH << "," << maxLayers << '\n';
		for(vector<layer*>::iterator it = layerVec.begin(); it != layerVec.end(); it++){
			layer& L = *(*it);
			for(int i = 0; i < gridW; i++){
				for(int j = 0; j < gridH; j++){
					ofile << L.layerMap[i][j].x << "," << L.layerMap[i][j].y << " ";
					if(L.layerMap[i][j].x != -1){
						SDL_Rect src;
						src.x = L.layerMap[i][j].x * cellW;
						src.y = L.layerMap[i][j].y * cellH;
						src.w = cellW;
						src.h = cellH;
						SDL_Rect dest;
						dest.x = i * cellW;
						dest.y = j * cellH;
						dest.w = cellW;
						dest.h = cellH;
						if(SDL_BlitSurface(loaded, &src, sshot, &dest) != 0){
							printf("Blit error. %s\n", IMG_GetError());
						}
					}
				}
				ofile << '\n';
			}
		}
		ofile.close();
		SDL_Texture* newTexture = SDL_CreateTextureFromSurface(renderer, sshot);
		SDL_RenderCopy(renderer, newTexture, NULL, NULL);
		if(newTexture!=NULL){
			IMG_SavePNG(sshot, imgFname.c_str());
		}
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderClear(renderer);
		//saveTiles(levelMap, mGrid, inFname, renderer, imgFname);
		SDL_FreeSurface(loaded);
		SDL_DestroyTexture(newTexture);
		newTexture = NULL;
	}
}

void loadMap(vector<layer*>& layerVec, string* texFname){
    char const * lTheOpenFileName = tinyfd_openFileDialog(
		"Open Map",
		"",
		1,
		lFilterPatterns,
		"map files",
		0);
    string fName = lTheOpenFileName;
    if(fName != ""){
        ifstream ifile(fName);
        string tStr;
        ifile >> *texFname;
        ifile >> tStr;
        layerVec[0]->layerGrid.gridW = stoi( tStr.substr(0, tStr.find(',', 0)) );
        tStr = tStr.substr(tStr.find(',') + 1);
        layerVec[0]->layerGrid.gridH = stoi( tStr.substr(0, tStr.find(',', 0)) );
        tStr = tStr.substr(tStr.find(',') + 1);
        layerVec[0]->layerGrid.cellW = stoi( tStr.substr(0, tStr.find(',', 0)) );
        tStr = tStr.substr(tStr.find(',') + 1);
        int maxL = 1;
        if(tStr.find(',') == -1){
            layerVec[0]->layerGrid.cellH = stoi( tStr);
        }else{
            layerVec[0]->layerGrid.cellH = stoi( tStr.substr(0, tStr.find(',', 0)) );
            tStr = tStr.substr(tStr.find(',') + 1);
            maxL = stoi( tStr);
        }
        for(int lCount = 0; lCount < maxL; lCount++){
            for(int i = 0; i < layerVec[lCount]->layerGrid.gridW; i++){
                for(int j = 0; j < layerVec[lCount]->layerGrid.gridH; j++){
                    ifile >> tStr;
                    layerVec[lCount]->layerMap[i][j].x = stoi(tStr.substr(0, tStr.find(',')));
                    layerVec[lCount]->layerMap[i][j].y = stoi(tStr.substr(tStr.find(',') + 1));
                }
            }
        }
        ifile.close();
    }else{
		printf("NULL FNAME");
	}
}

void getMouseToGrid(int* cX, int* cY, vec2 mouse, grid mGrid, screenArea activeArea){
    *cX = mouse.x;
    if(activeArea == screenArea::SPRITE){
        *cX -= spriteSheetX;
    }
    *cX -= *cX % int(mGrid.cellW);
    *cX /= mGrid.cellW;
    *cY = mouse.y;
    *cY -= *cY % int(mGrid.cellH);
    *cY /= mGrid.cellH;
}

void lowerUpper(int* xlower, int* xupper, int* ylower, int* yupper, int xrs, int xre, int yrs, int yre){
    if(xrs > xre){
        *xlower = xre;
        *xupper = xrs;
    }else{
        *xlower = xrs;
        *xupper = xre;
    }
    if(yrs > yre){
        *ylower = yre;
        *yupper = yrs;
    }else{
        *ylower = yrs;
        *yupper = yre;
    }
}


int main( int argc, char * argv[] ){
    #ifdef _WIN32
	    tinyfd_winUtf8 = 1;
    #endif

    std::vector<spriteSheet*> spriteSheets;
    SDL_Window* window = NULL;

    int activeLayer = 0;
    vector<layer*> tileLayer;
    layer* L;
    for(int i = 0; i < maxLayers; i++){
        L = new layer;
        tileLayer.push_back(L);
    }

    /*vec2 tileMap[40][40];
    grid mapGrid;
    mapGrid.cellW = 16.0;
    mapGrid.cellH = 16.0;
    mapGrid.gridW = 40;
    mapGrid.gridH = 40;*/

    vec2 selectedSpriteXRange;
    selectedSpriteXRange.x = -1;
    selectedSpriteXRange.y = -1;

    vec2 selectedSpriteYRange;
    selectedSpriteYRange.x = -1;
    selectedSpriteYRange.y = -1;

    int activeSS = 0;

    vec2 clickXrange;
    clickXrange.x = -1;
    clickXrange.y = -1;
    vec2 clickYrange;
    clickYrange.x = -1;
    clickYrange.y = -1;
    int startClickX = -1;
    int startClickY = -1;
    bool mbDown = false;

    vec2 mouse;
    screenArea activeArea;

    /*for(int i = 0; i < mapGrid.gridW; i++){
        for(int j = 0; j < mapGrid.gridH; j++){
            tileMap[i][j].x = -1;
            tileMap[i][j].y = -1;
        }
    }*/

    SDL_Renderer* renderer;

    double fps = 30.0;
    double fpsTicker = 1000.0 / fps;

    if(!init(&window, &renderer)){
        printf("Failed to initialise!\n");
    }
    else{
        if(!loadMedia(&spriteSheets, &renderer)){
            printf("Failed to load media!\n");
        }
        else{
            bool quit = false;
            SDL_Event e;

            Uint64 dTimeNow = SDL_GetPerformanceCounter();
            Uint64 dTimePrev = 0;
            double deltaTime = 0;

            while(!quit){
                dTimePrev = dTimeNow;
                dTimeNow = SDL_GetPerformanceCounter();
                deltaTime = (double)((dTimeNow - dTimePrev) * 1000 / (double)SDL_GetPerformanceFrequency());
                fpsTicker -= deltaTime;

                bool shiftDown = false;
                bool screenShot = false;

                while(SDL_PollEvent(&e)!=0){
                    if(e.type == SDL_QUIT){
                        quit = true;
                    }
                    if(e.type == SDL_KEYDOWN){
                        switch(e.key.keysym.sym){
                            case SDLK_ESCAPE:
                                selectedSpriteXRange.x = -1;
                                selectedSpriteXRange.y = -1;
                                selectedSpriteYRange.x = -1;
                                selectedSpriteYRange.y = -1;
                                startClickX = -1;
                                startClickY = -1;
                                mbDown = false;
                                break;
                            case SDLK_q:
                                quit = true;
                                break;
                            case SDLK_LEFT:
                                break;
                            case SDLK_RIGHT:
                                break;
                            case SDLK_SPACE:
                                screenShot = true;
                                break;
                            case SDLK_UP:
                                if(activeLayer < (maxLayers - 1)){
                                    activeLayer++;
                                }
                                break;
                            case SDLK_DOWN:
                                if(activeLayer > 0){
                                    activeLayer--;
                                }
                                break;
                            case SDLK_LSHIFT:
                                break;
                            case SDLK_s:
                                shiftDown = SDL_GetModState() & KMOD_LSHIFT;
                                if(shiftDown){
                                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                                    SDL_RenderClear(renderer);
                                    saveMap(tileLayer, spriteSheets[activeSS]->fName, renderer);
                                }
                                break;
                            case SDLK_l:
                                shiftDown = SDL_GetModState() & KMOD_LSHIFT;
                                if(shiftDown){
                                    string fName;
                                    loadMap(tileLayer, &fName);
                                    bool fnameFound = false;
                                    if(fName != ""){
                                        for(int i = 0; i < spriteSheets.size(); i++){
                                            if(spriteSheets[i]->fName == fName){
                                                activeSS = i;
                                                fnameFound = true;
                                                break;
                                            }
                                        }
                                        if(!fnameFound){
                                            SDL_Texture* T;
                                            T = loadTexture(fName, &renderer);
                                            if(T == NULL){
                                                printf("Unable to load image. error: %s\n", SDL_GetError());
                                            }else{
                                                spriteSheet* ss = new spriteSheet;
                                                ss->fName = fName;
                                                ss->T = T;
                                                spriteSheets.push_back(ss);
                                                activeSS = spriteSheets.size() - 1;
                                            }
                                        }
                                    }
                                }
                                break;
                        }
                    }
                    if(e.type == SDL_KEYUP){
                        switch(e.key.keysym.sym){
                            case SDLK_LEFT:
                                break;
                            case SDLK_RIGHT:
                                break;
                            case SDLK_UP:
                                break;
                            case SDLK_DOWN:
                                break;
                            case SDLK_SPACE:
                                break;
                            case SDLK_LSHIFT:
                                shiftDown = false;
                                break;
                        }
                    }
                    if(e.type == SDL_MOUSEMOTION){
                        SDL_GetMouseState(&mouse.x, &mouse.y);
                        if(mouse.x > spriteSheetX){
                            activeArea = screenArea::SPRITE;
                        }else{
                            activeArea = screenArea::MAP;
                        }
                    }
                    if((e.type == SDL_MOUSEBUTTONDOWN) && (clickXrange.x == -1)){
                        mbDown = true;
                        int cX, cY;
                        getMouseToGrid(&cX, &cY, mouse, tileLayer[activeLayer]->layerGrid, activeArea);
                        startClickX = cX;
                        startClickY = cY;
                        clickXrange.x = cX;
                        clickYrange.x = cY;
                        if(activeArea == screenArea::SPRITE){
                            selectedSpriteXRange.x = cX;
                            selectedSpriteYRange.x = cY;
                        }
                    }
                    if((e.type == SDL_MOUSEBUTTONUP)){
                        mbDown = false;
                        if(clickXrange.x != -1){
                            int cX, cY;
                            getMouseToGrid(&cX, &cY, mouse, tileLayer[activeLayer]->layerGrid, activeArea);
                            clickXrange.y = cX;
                            clickYrange.y = cY;
                            switch(activeArea){
                                case screenArea::MAP:
                                    if((selectedSpriteXRange.x != -1) && (selectedSpriteYRange.x != -1)){
                                        if((selectedSpriteXRange.y == -1) || (selectedSpriteYRange.y == -1)){
                                            selectedSpriteXRange.y = selectedSpriteXRange.x;
                                            selectedSpriteYRange.y = selectedSpriteYRange.x;
                                        }
                                        int xLen = selectedSpriteXRange.x - selectedSpriteXRange.y;
                                        int yLen = selectedSpriteYRange.x - selectedSpriteYRange.y;
                                        int sprXlower, sprXupper;
                                        int sprYlower, sprYupper;
                                        lowerUpper(&sprXlower, &sprXupper, &sprYlower, &sprYupper, selectedSpriteXRange.x, selectedSpriteXRange.y, selectedSpriteYRange.x, selectedSpriteYRange.y);
                                        int mapXlower, mapXupper;
                                        int mapYlower, mapYupper;
                                        lowerUpper(&mapXlower, &mapXupper, &mapYlower, &mapYupper, clickXrange.x, clickXrange.y, clickYrange.x, clickYrange.y);
                                        if(SDL_GetModState() & KMOD_LCTRL){
                                            for(int i = mapXlower; ((i < (mapXlower + abs(xLen) + 1)) && (i < tileLayer[activeLayer]->layerGrid.gridW)); i++){
                                                for(int j = mapYlower; ((j < (mapYlower + abs(yLen) + 1)) && (j < tileLayer[activeLayer]->layerGrid.gridH)); j++){
                                                    tileLayer[activeLayer]->layerMap[i][j].x = sprXlower + ((i - mapXlower) % (abs(xLen) + 1));
                                                    tileLayer[activeLayer]->layerMap[i][j].y = sprYlower + ((j - mapYlower) % (abs(yLen) + 1));
                                                }
                                            }
                                        }else{
                                            for(int i = mapXlower; i <= mapXupper; i++){
                                                for(int j = mapYlower; j <= mapYupper; j++){
                                                    tileLayer[activeLayer]->layerMap[i][j].x = sprXlower + ((i - mapXlower) % (abs(xLen) + 1));
                                                    tileLayer[activeLayer]->layerMap[i][j].y = sprYlower + ((j - mapYlower) % (abs(yLen) + 1));
                                                }
                                            }
                                        }
                                    }else{
                                        int mapXlower, mapXupper;
                                        int mapYlower, mapYupper;
                                        lowerUpper(&mapXlower, &mapXupper, &mapYlower, &mapYupper, clickXrange.x, clickXrange.y, clickYrange.x, clickYrange.y);

                                        for(int i = mapXlower; i <= mapXupper; i++){
                                            for(int j = mapYlower; j <= mapYupper; j++){
                                                tileLayer[activeLayer]->layerMap[i][j].x = -1;
                                                tileLayer[activeLayer]->layerMap[i][j].y = -1;
                                            }
                                        }
                                    }
                                    break;
                                case screenArea::SPRITE:
                                    selectedSpriteXRange.y = cX;
                                    selectedSpriteYRange.y = cY;
                                    break;

                            }
                        }
                        startClickX = -1;
                        startClickY = -1;
                    }
                }

                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderClear(renderer);

                SDL_Rect destRect;
                destRect.x = spriteSheetX;
                destRect.y = spriteSheetY;
                destRect.w = 640;
                destRect.h = 576;
                SDL_Rect srcRect;
                srcRect.x = 0;
                srcRect.y = 0;
                srcRect.w = 640;
                srcRect.h = 576;

                SDL_RenderCopy(renderer, spriteSheets[activeSS]->T, &srcRect, &destRect);
                for(int lyr = 0; lyr < maxLayers; lyr++){
                    if(lyr != activeLayer && (SDL_GetModState() & KMOD_LSHIFT)){
                        SDL_SetTextureAlphaMod(spriteSheets[activeSS]->T, 120);
                    }else{
                        SDL_SetTextureAlphaMod(spriteSheets[activeSS]->T, 255);
                    }
                    for(int i = 0; i < tileLayer[lyr]->layerGrid.gridW; i++){
                        for(int j = 0; j < tileLayer[lyr]->layerGrid.gridH; j++){
                            if((tileLayer[lyr]->layerMap[i][j].x != -1) && (tileLayer[lyr]->layerMap[i][j].y != -1)){
                                srcRect.x = tileLayer[lyr]->layerMap[i][j].x * tileLayer[lyr]->layerGrid.cellW;
                                srcRect.y = tileLayer[lyr]->layerMap[i][j].y * tileLayer[lyr]->layerGrid.cellW;
                                srcRect.w = tileLayer[lyr]->layerGrid.cellW;
                                srcRect.h = tileLayer[lyr]->layerGrid.cellH;

                                destRect.x = i*tileLayer[lyr]->layerGrid.cellW;
                                destRect.y = j*tileLayer[lyr]->layerGrid.cellH;
                                destRect.w = tileLayer[lyr]->layerGrid.cellW;
                                destRect.h = tileLayer[lyr]->layerGrid.cellH;

                                SDL_RenderCopy(renderer, spriteSheets[activeSS]->T, &srcRect, &destRect);
                            }
                        }
                    }
                }
                SDL_SetTextureAlphaMod(spriteSheets[activeSS]->T, 255);
                SDL_Rect shadeRect;
                SDL_SetRenderDrawColor(renderer, 120, 120, 255, 100);
                if(selectedSpriteXRange.x != -1){
                    int mX, mY;
                    getMouseToGrid(&mX, &mY, mouse, tileLayer[activeLayer]->layerGrid, activeArea);
                    int xLen;
                    int yLen;
                    int xlower, xupper;
                    int ylower, yupper;
                    if(selectedSpriteXRange.y != -1){
                        lowerUpper(&xlower, &xupper, &ylower, &yupper, selectedSpriteXRange.x, selectedSpriteXRange.y, selectedSpriteYRange.x, selectedSpriteYRange.y);
                    }else if((mX != clickXrange.x) || (mY != clickYrange.x)){
                        lowerUpper(&xlower, &xupper, &ylower, &yupper, selectedSpriteXRange.x, mX, selectedSpriteYRange.x, mY);
                    }else{
                        lowerUpper(&xlower, &xupper, &ylower, &yupper, selectedSpriteXRange.x, selectedSpriteXRange.x, selectedSpriteYRange.x, selectedSpriteYRange.x);
                    }
                    for(int i = xlower; i <= xupper; i++){
                        for(int j = ylower; j <= yupper; j++){
                            shadeRect.x = i*tileLayer[activeLayer]->layerGrid.cellW + spriteSheetX;
                            shadeRect.y = j*tileLayer[activeLayer]->layerGrid.cellH + spriteSheetY;
                            shadeRect.w = tileLayer[activeLayer]->layerGrid.cellW;
                            shadeRect.h = tileLayer[activeLayer]->layerGrid.cellH;
                            SDL_RenderFillRect(renderer, &shadeRect);
                            if(activeArea == screenArea::MAP){
                                srcRect.x = i * tileLayer[activeLayer]->layerGrid.cellW;
                                srcRect.y = j * tileLayer[activeLayer]->layerGrid.cellH;
                                srcRect.w = tileLayer[activeLayer]->layerGrid.cellW;
                                srcRect.h = tileLayer[activeLayer]->layerGrid.cellH;
                                if(mbDown){
                                    shadeRect.x = (clickXrange.x + (i - xlower)) * tileLayer[activeLayer]->layerGrid.cellW;
                                    shadeRect.y = (clickYrange.x + (j - ylower)) * tileLayer[activeLayer]->layerGrid.cellH;
                                }else{
                                    getMouseToGrid(&(shadeRect.x), &(shadeRect.y), mouse, tileLayer[activeLayer]->layerGrid, activeArea);
                                    shadeRect.x += i - xlower;
                                    shadeRect.y += j - ylower;
                                    shadeRect.x *= tileLayer[activeLayer]->layerGrid.cellW;
                                    shadeRect.y *= tileLayer[activeLayer]->layerGrid.cellH;
                                }

                                if(SDL_GetModState() & KMOD_LCTRL){
                                    SDL_SetTextureAlphaMod(spriteSheets[activeSS]->T, 190);
                                }else{
                                    SDL_SetTextureAlphaMod(spriteSheets[activeSS]->T, 120);
                                }
                                SDL_RenderCopy(renderer, spriteSheets[activeSS]->T, &srcRect, &shadeRect);
                                SDL_SetTextureAlphaMod(spriteSheets[activeSS]->T, 255);
                            }
                        }
                    }
                }

                if(mbDown && (activeArea == screenArea::MAP)){
                    int xlower, xupper;
                    int ylower, yupper;
                    int cX, cY;
                    getMouseToGrid(&cX, &cY, mouse, tileLayer[activeLayer]->layerGrid, activeArea);
                    lowerUpper(&xlower, &xupper, &ylower, &yupper, clickXrange.x, cX + 1, clickYrange.x, cY + 1);

                    for(int i = xlower; i < xupper; i++){
                        for(int j = ylower; j < yupper; j++){
                            shadeRect.x = (i * tileLayer[activeLayer]->layerGrid.cellW);
                            shadeRect.y = (j * tileLayer[activeLayer]->layerGrid.cellH);
                            shadeRect.w = tileLayer[activeLayer]->layerGrid.cellW;
                            shadeRect.h = tileLayer[activeLayer]->layerGrid.cellH;
                            SDL_RenderFillRect(renderer, &shadeRect);
                        }
                    }
                }else if((clickXrange.x != -1) && (clickYrange.x != -1) && (activeArea == screenArea::MAP)){
                    shadeRect.x = (clickXrange.x * tileLayer[activeLayer]->layerGrid.cellW);
                    shadeRect.y = (clickYrange.x * tileLayer[activeLayer]->layerGrid.cellH);
                    shadeRect.w = tileLayer[activeLayer]->layerGrid.cellW;
                    shadeRect.h = tileLayer[activeLayer]->layerGrid.cellH;
                    SDL_RenderFillRect(renderer, &shadeRect);
                }

                if(!mbDown){
                    clickXrange.x = -1;
                    clickXrange.y = -1;
                    clickYrange.x = -1;
                    clickYrange.y = -1;
                }else{
                    if(activeArea == screenArea::SPRITE){
                        selectedSpriteXRange.y = -1;
                        selectedSpriteYRange.y = -1;
                    }
                }

                drawGrid(renderer, tileLayer[activeLayer]->layerGrid.gridW, tileLayer[activeLayer]->layerGrid.gridH,
                         tileLayer[activeLayer]->layerGrid.cellW, tileLayer[activeLayer]->layerGrid.cellH);
                drawGrid(renderer, tileLayer[activeLayer]->layerGrid.gridW, tileLayer[activeLayer]->layerGrid.gridH,
                         tileLayer[activeLayer]->layerGrid.cellW, tileLayer[activeLayer]->layerGrid.cellH, spriteSheetX, spriteSheetY);

                if(screenShot && (sshotCount < maxSshot)){
                    //string ssFname = "C:/Users/Rico/Documents/cppProjects/Project-1/screenshot_";
		    string ssFname = "/screenshot_";
                    ssFname += to_string(sshotCount);
                    ssFname += ".png";
                    SDL_Surface* surface = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0, 0, 0, 0);
                    SDL_RenderReadPixels(renderer, NULL, surface->format->format, surface->pixels, surface->pitch);
                    IMG_SavePNG(surface, ssFname.c_str());
                    SDL_FreeSurface(surface);
                    sshotCount++;
                }

                SDL_RenderPresent(renderer);
            }
        }
    }

    close(window, &spriteSheets);
    return 0;
}
