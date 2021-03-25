/*
 * @Author: rosonlee 
 * @Date: 2021-03-22 19:51:49 
 * @Last Modified by:   rosonlee 
 * @Last Modified time: 2021-03-22 19:51:49 
 */

#include "server.h"
#include <exception>
#include <assert.h>

void AddSig(int sig, void (handler)(int), bool restart = true){
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    if(restart){
        sa.sa_flags |= SA_RESTART;
    }
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

int main(){// paramater do not provide for now 
    AddSig(SIGPIPE, SIG_IGN);
    Server server;
    server.Init();
    server.Listen();
    server.Loop();
}
