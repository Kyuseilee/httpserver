# A Simple WebServer
## Introduction
本项目使用C++11编写了一个Web服务器
<!-- - 支持长连接（优雅关闭连接？） -->
- 支持解析GET/POST请求
- 可处理静态资源
- 采用Reactor模式
- 支持日志系统 
<!-- - 支持非活动长连接的回收 -->
  
通过线程池实现轻量级并发，主线程仅负责监听是否有事件发生，工作线程负责处理读写数据、建立连接、处理请求。

## 目录

## 测试环境
  - Ubuntu 18.04
  - MySQL 5.7
- 浏览器
  - chrome
## 测试网页
114.55.36.251:9006

## 构建：
- 准备工作：
```mysql
create database mydb;(名字与main.cpp中一致即可)
use mydb;
CREATE TABLE user(username char(50) NULL, passwd char(50)NULL) ENGINE=InnoDB;

INSERT INTO user(username, passwd) VALUES('name', 'passwd');
```

- build 
  ```bash
  sh ./build.sh
  ```
- 启动
  ```bash
  ./server
  ```

## 下一步工作
将过程中遇到的问题汇总，方便参照者快速解决相似的问题
完成内容详解
增加优雅关闭连接、完善日志系统、解析其他HTTP头

## 详解
(later update.....)