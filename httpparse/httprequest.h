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

    //TODO ????
    // std::string GetPost()

    bool IsKeepAlive() const;


private:
    bool __ParseRequestLine(const std::string& line);
    void __ParseHeader(const std::string& line);
    void __ParseBody(const std::string& line);

    void __ParsePath();
    void __ParsePost();
    void __ParseFromUrlencoded(); 

private:
    PARSE_STATE state_;
    std::string method_, path_, version_, body_;
    std::unordered_map<std::string, std::string>header_;
    std::unordered_map<std::string, std::string>pos_;

private:
    static const std::unordered_set<std::string>DEFAULT_HTML;
    static const std::unordered_map<std::string, int>DEFAULT_HTML_TAG;

    static int ConverHex(char ch);
};


#endif // HTTPREQUEST_H