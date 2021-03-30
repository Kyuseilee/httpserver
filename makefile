CXX ?= g++

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g
else
    CXXFLAGS += -O2

endif
server: main.cpp  ./http/http_conn.cpp ./timer/timer.cpp server.cpp config.cpp ./log/log.cpp ./Sql/sql_connection_pool.cpp
	$(CXX) -o server  $^ $(CXXFLAGS) -lpthread -lmysqlclient
INC = -I ./http/http_conn -I ./locker/locker -I ./threadpool/threadpool

clean:
	rm  -r server