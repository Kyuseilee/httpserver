/*
 * @Author: rosonlee 
 * @Date: 2021-06-30 22:44:20 
 * @Last Modified by:   rosonlee 
 * @Last Modified time: 2021-06-30 22:44:20 
 */

#include "httprequest.h"

using namespace std;

const unordered_set<string> HttpRequest::DEFAULT_HTML{
    "/index", "/regeister", "/login",
    "/welcome", "/video", "picture",};

const unordered_map<string, int> HttpRequest::DEFAULT_HTML_TAG{
    {"/register.html", 0}, {"/login.html", 1}, };


void HttpRequest::Init(){
    method_ = path_ = version_ = body_ = "";
    state_ = REQUEST_LINE;
    header_.clear();
    post_.clear();
}

bool HttpRequest::IsKeepAlive() const{
    if (header_.count("Connection") == 1){
        return header_.find("Connection")->second == "keep-alive" && version_ == "1.1";
    }
    return false;
}