#include "Log.h"

Log::Log(){
    saveStrBuffer = 25;
}

void Log::log(std::string _string){
    if((mode & Log::LogMode::LOGMODE_CONSOLE) == Log::LogMode::LOGMODE_CONSOLE){
        logStrings.push(_string);
        printLog();
    }
    if((mode & Log::LogMode::LOGMODE_SAVE) == Log::LogMode::LOGMODE_SAVE){
        saveStrings.push(_string);
        if(int(saveStrings.size()) == saveStrBuffer){
            saveLog();
        }
    }
};

void Log::printLog(){
    if(!logStrings.empty()){
        std::cout << logStrings.front() << "\n";
        logStrings.pop();
    }
};

void Log::saveLog(){
    std::ofstream logfile;
    logfile.open(helper::getPath() + "log.txt", std::ios::app);
    if(!saveStrings.empty()){
        logfile << saveStrings.front() << "\n";
        saveStrings.pop();
    }
    logfile.close();
};
