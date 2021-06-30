/*
 * @Author: rosonlee 
 * @Date: 2021-06-17 21:46:17 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-06-30 20:20:19
 */

#include <string.h>
#include "Server/Server.h"

int main(){
    Server server(9006, 30000, false);
    server.Loop();

    return 0;
}






