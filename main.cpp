/*
 * @Author: rosonlee 
 * @Date: 2021-03-22 19:51:49 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-03-29 19:52:28
 */

#include "server.h"
#include <exception>
#include <assert.h>


int main(){// paramater do not provide for now 
    Server server;
    server.Init();
    server.Listen();
    server.Loop();
}
