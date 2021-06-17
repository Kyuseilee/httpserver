/*
 * @Author: rosonlee 
 * @Date: 2021-06-17 21:46:33 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-06-17 22:04:05
 */

class Server{

public:
    Server();
    Server(); //提供另一种重载
    ~Server();

public:
    void Init();
    void Listen();
    void Loop();

    void HandleRead();
    void HandleWrite();
    void HandleConnect();

private:
    int port_;
    int host_;

};