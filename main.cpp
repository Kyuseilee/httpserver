/*
 * @Author: rosonlee 
 * @Date: 2021-06-17 21:46:17 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-06-17 22:56:22
 */

#include<sys/types.h>
#include<signal.h>
#include<errno.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/socket.h>
#include<ctype.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<sys/epoll.h>
#include<poll.h>
#include <assert.h>
#include <memory>
#include <string.h>


#define BUFSIZ 1024

#include <iostream>


using namespace std;



int main(){
    
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listen >= 0);

    struct sockaddr_in address;
    bzero(&address, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(9006);

    int flag = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    
    int ret = 0;
    ret = bind(listen_fd, (struct sockaddr*)&address, sizeof(address));
    assert(ret >= 0);

    ret = listen(listen_fd, 5);
    assert(ret >= 0);

    int client_fd;
    struct sockaddr_in client_address;
    char buff[BUFSIZ];
    socklen_t len = sizeof(struct sockaddr);

    while(1){
        int state;
        client_fd = accept(listen_fd, (struct sockaddr*)&client_address, &len);

        assert(client_fd >= 0);

        printf("Got Connection from ip = %s, port = %d", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        state = recv(client_fd, buff, sizeof(buff), 0);
        assert(state >= 0);
        cout << "\n";

        for (int i = 0; i < BUFSIZ && buff[i] != '\0'; ++i){
            printf("%c", buff[i]);
        }
        cout << "\n";
        state = write(client_fd, "Hello, client, you are welcome!\r\n", 32);
        assert(state >= 0);
        printf("Write ok!\n");
        close(client_fd);
    }




}
