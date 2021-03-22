/*
 * @Author: rosonlee 
 * @Date: 2021-03-22 19:51:49 
 * @Last Modified by:   rosonlee 
 * @Last Modified time: 2021-03-22 19:51:49 
 */

#include "config.h"

int main(){// paramater do not provide for now 

    Server server;
    server.Init();
    server.Listen();
    server.Loop();
}
