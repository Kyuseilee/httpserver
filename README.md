# A Simple WebServer
## Introduction
本项目使用C++11编写了一个Web服务器
- 支持解析GET/HEAD/POST请求
- 支持长连接（优雅关闭连接？）
- 可处理静态资源
- 采用Reactor?模拟Proactor?模式
- 支持非活动长连接的回收
- 支持日志系统 （同步/异步）？
  
通过线程池实现轻量级并发，主线程仅负责监听是否有事件发生，工作线程负责处理读写数据、建立连接、处理请求。
目录
测试网页
运行测试
详解