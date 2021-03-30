/*
 * @Author: rosonlee 
 * @Date: 2021-03-22 19:51:49 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-03-30 20:27:15
 */

#include "config.h"
#include <exception>
#include <assert.h>


int main(int argc, char *argv[]){// paramater do not provide for now 

    string user = "root";
    string passwd = "123456";
    string databasename = "mydb";

    Config config;
    config.ParseArg(argc, argv);
    
    Server server;

    server.Init(config.PORT, user, passwd, databasename, config.thread_num,config.LOGWrite, config.sql_num,config.close_log);

    server.LogWrite();
    server.SqlPool();

    server.Listen();
    server.Loop();
}
