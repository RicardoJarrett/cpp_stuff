#ifndef _LOG_H_
#define _LOG_H_

#include <iostream>
#include <queue>
#include <string>
#include <fstream>
#include "helper.h"

struct Log{
    enum LogMode{LOGMODE_NULL=0, LOGMODE_SAVE=1, LOGMODE_CONSOLE=2};
    uint16_t mode;
    int saveStrBuffer;

    std::queue<std::string> logStrings;
    std::queue<std::string> saveStrings;
    void log(std::string _string);
    void printLog();
    void saveLog();
    Log();
};

#endif // _LOG_H_
