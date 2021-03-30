#include <mysql/mysql.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>


using namespace std;

int main(){
    MYSQL *con = nullptr;
    con = mysql_init(con);
    string host ="127.0.0.1";
    string user = "root";
    string passwd = "123456";
    string db = "mydb";
    con = mysql_real_connect(con, host.c_str(), user.c_str(), passwd.c_str(), db.c_str(), 3306, nullptr, 0);
    char *sql_insert = (char *)malloc(sizeof(char) * 200);
    strcpy(sql_insert, "INSERT INTO user(username, passwd) VALUES('ro', '123456')");
    printf("hi.\n");
    int ret = mysql_query(con, sql_insert);
    //     printf("%s", mysql_error(con));
    printf("%d", ret);
    // }

}