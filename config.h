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

const int MAX_NUMBER = 1024;