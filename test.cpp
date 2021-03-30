#include <mysql/mysql.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

int main(){
    MYSQL *con = nullptr;
    con = mysql_init(con);
    string host ="127.0.0.1";
    string user = "root";
    string passwd = "123456";
    string db = "mydb";
    con = mysql_real_connect(con, host.c_str(), user.c_str(), passwd.c_str(), db.c_str(), 3306, nullptr, 0);
    if (con)
        printf("Succeed.\n");
    else{
        printf("Failed.\n");
    }
}