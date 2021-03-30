/*
 * @Author: rosonlee 
 * @Date: 2021-03-22 19:51:45 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-03-30 18:57:41
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "server.h"

using namespace std;

class Config{
public:
    Config();
    ~Config(){};

    void ParseArg(int argc, char* argv[]);

    int PORT;
    
    int LOGWrite;

    int OPT_LINGER;

    int sql_num;

    int thread_num;

    int close_log;

    int actor_model;
    int TRIGMode;
    int LISTENTrigmode;
    int CONNTrigmode;
};

#endif // CONFIG_H

