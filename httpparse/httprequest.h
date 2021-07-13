/*
 * @Author: rosonlee 
 * @Date: 2021-06-30 22:43:46 
 * @Last Modified by: rosonlee
 * @Last Modified time: 2021-06-30 22:44:30
 */

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <regex>
#include <errno.h>
#include <mysql/mysql.h>

#include "../buffer/buffer.h"
#include "../log/log.h"
#include "../pool/sqlconnpool.h"
#include "../pool/sqlconnRAII.h"

class HttpRequest
{
public:
    enum PARSE_STATE{
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH,
    };

    enum HTTP_CODE{
        NO_REQUEST = 0,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDENT_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION,
    };

public:
    HttpRequest() { Init(); };
    ~HttpRequest() = default;

    void Init();
    bool Parse();//TODO Buffer

    std::string& Path();
    std::string Path() const;
    std::string Method()const;
    std::string Version() const;

    std::string GetPost(const std::string& key) const;
    std::string GetPos(const char* key) const ;

    bool IsKeepAlive() const;


private:
    bool __ParseRequestLine(const std::string& line);
    void __ParseHeader(const std::string& line);
    void __ParseBody(const std::string& line);

    void __ParsePath();
    void __ParsePost();
    void __ParseFromUrlencoded(); 

private:
    static bool UserVerify(const std::string& name, const std::string& pwd, bool isLogin);

private:
    PARSE_STATE state_;
    std::string method_, path_, version_, body_;
    std::unordered_map<std::string, std::string>header_;
    std::unordered_map<std::string, std::string>post_;

private:
    static const std::unordered_set<std::string>DEFAULT_HTML;
    static const std::unordered_map<std::string, int>DEFAULT_HTML_TAG;

    static int ConverHex(char ch);
};


#endif // HTTPREQUEST_H