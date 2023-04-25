#define SDL_MAIN_HANDLED
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include <iostream>
#include <fstream>
#include <Windows.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <ctime>
#include <cmath>
#include <queue>
#include <algorithm>

#define _countof(array) (sizeof(array) / sizeof(array[0]))

#include "helper.h"
using namespace helper;
#include "Anim.h"
#include "Enemy.h"
#include "Stat.h"
#include "Button.h"
#include "Turret.h"
#include "Base.h"
#include "Game.h"
#include "Log.h"

int SCR_W = 1280;//480;
int SCR_H = 720;//360;
int LEVEL_W = 4096;
int LEVEL_H = 4096;
bool FULLSCREEN = false;
bool PLAY_TEST = false;
bool HOLD_CONSOLE_ON_EXIT = false;
Log logger;

int main(int argc, char* argv[]){
    //note
    //fix disappeared sprites, extra levels.
    //split to class files.
    //add z layers for render.
    Game game;
    game.setLogger(logger);
    logger.mode = Log::LogMode::LOGMODE_CONSOLE + Log::LogMode::LOGMODE_SAVE;
    std::srand(time(nullptr));
    SDL_Window* window = NULL;
    SDL_Texture* ScrSurface;
    SDL_Texture* LevelSurface;
    SDL_Renderer* renderer;

    bool initSuccess = true;
    if(PLAY_TEST){
        FULLSCREEN = true;
    }
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        std::cout << "Error - Init Video\n";
        initSuccess = false;
    }else if(TTF_Init() == -1){
        std::cout << "Error - Init TTF\n";
        initSuccess = false;
    }else{
        window = SDL_CreateWindow("TD", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCR_W, SCR_H, SDL_WINDOW_SHOWN);
        if(window == NULL){
            std::cout << "Error - Create window\n";
            initSuccess = false;
        }else{
            if(FULLSCREEN){
                SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
            }
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
            ScrSurface = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, SCR_W, SCR_H);
            SDL_SetTextureBlendMode(ScrSurface, SDL_BLENDMODE_BLEND);
            LevelSurface = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, LEVEL_W, LEVEL_H);
            SDL_SetTextureBlendMode(LevelSurface, SDL_BLENDMODE_BLEND);
        }
    }

    if(initSuccess){
        //AspectRatio aspect;
        SDL_DisplayMode dm;
        if(SDL_GetCurrentDisplayMode(0, &dm) >= 0){
            double div = double(dm.w) / double(dm.h);
            if(div > (1.7) && div < (1.8)){
                //aspect = r16x9;
            }else if(div == (1.5)){
                //aspect = r3x2;
            }
        }
        game.setRenderer(renderer, ScrSurface, LevelSurface);
        bool quit = false;

        Uint64 dTimeNow = SDL_GetPerformanceCounter();
        Uint64 dTimePrev = 0;
        double deltaTime = 0;
        time_t startTime = time(NULL);
        double perfFrequency =(double)SDL_GetPerformanceFrequency();
        while(!quit){
            dTimePrev = dTimeNow;
            dTimeNow = SDL_GetPerformanceCounter();
            deltaTime = (double)((dTimeNow - dTimePrev) * 1000 / perfFrequency);
            quit = game.update(deltaTime);
        }// !quit
        time_t timeNow = time(NULL);

        if(PLAY_TEST){
            std::fstream playTestTimeFile;
            playTestTimeFile.open(getPath() + "playTestTime.sav");
            std::stringstream ss;
            std::vector<std::string> lines;
            if(playTestTimeFile.is_open()){
                std::string currentLine;
                while(std::getline(playTestTimeFile, currentLine)){
                    if(currentLine != ""){
                        int totalRunTime = std::stoi(currentLine);
                        ss.str("");
                        ss << totalRunTime << "\n";
                        lines.push_back(ss.str());
                    }
                }
            }
            playTestTimeFile.close();
            ss.str("");
            ss << (timeNow - startTime);
            lines.push_back(ss.str());
            int timeDif = std::stoi(ss.str());
            ss.str("");
            int totalTimeDif = std::stoi(lines[0]);
            if(int(lines.size()) != 1){
                totalTimeDif += timeDif;
            }
            ss << totalTimeDif << "\n";
            lines[0] = ss.str();
            playTestTimeFile.open(getPath() + "playTestTime.sav", std::ios::out);
            for(std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); it++){
                playTestTimeFile << (*it);
            }
            std::cout << "Total Test time: " << totalTimeDif << " seconds.\nLast Test time: " << timeDif << " seconds\n";
            playTestTimeFile.close();
        }
    }//initsuccess
    if(HOLD_CONSOLE_ON_EXIT){
        std::cout << "Enter to close.\n";
        std::cin.get();
    }
    SDL_DestroyWindow(window);
    SDL_DestroyTexture(ScrSurface);
    SDL_DestroyTexture(LevelSurface);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
    return 0;
}
