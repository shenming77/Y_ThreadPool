# YThreadPool

基于std库纯实现的线程池，无任何第三方依赖。该线程池简单易用、性能优异、功能强大、稳定可靠。

基本上所有提升并发的优化，都是有两个最基本出发点：**就是批量进出，批量执行**

## 功能

* 基于std库纯手工实现，兼容mac/linux/windows跨平台使用，无任何第三方依赖；
* 无脑往threadpool中塞任务即可，支持任意格式（入参、返回值）的任务执行；
* 尽可能减少调度过程中各种细节损耗（比如，new、copy构造等）；
* local-thread机制：n个线程的私有的n个WorkStealing类的对象，线程执行任务时避免了从pool中获取去**争抢**任务了；
* work-stealing机制：WorkStealing类实现了**任务窃取机制**，当thread本地没有task的时候，从附近的thread去窃取任务；
* lock-free机制：基于atomic的、基于内部封装mutex的**无锁机制**，如基于atomic的WorkStealingLockFreeQueue类；
* 还具有自动扩缩容机制，批量处理机制，以及简单的负载均衡机制； 

## 环境要求

* Linux
* C++14

## 项目运行

	cmake -B build        # 生成构建目录，-B 指定生成的构建系统代码放在 build 目录
	cmake --build build   # 执行构建
	./build/y_thread      # 运行 y_thread 程序

## 目录树

```
.
├── code           源代码
│   ├── buffer
│   ├── config
│   ├── http
│   ├── log
│   ├── timer
│   ├── pool
│   ├── server
│   └── main.cpp
├── test           单元测试
│   ├── Makefile
│   └── test.cpp
├── resources      静态资源
│   ├── index.html
│   ├── image
│   ├── video
│   ├── js
│   └── css
├── bin            可执行文件
│   └── server
├── log            日志文件
├── webbench-1.5   压力测试
├── build          
│   └── Makefile
├── Makefile
├── LICENSE
└── readme.md
```



**WorkStealingLockFreeQueue类**： 一个包含盗取功能的无锁环形队列

​        该类包含：往尾部写入数据的tryPush函数、往头部读取数据的tryPop函数、以及往尾部读取数据的trySteal函数;

利用了 compare_exchange_strong 来确保 head_ 和 tail_ 的更新是原子性的，从而避免并发环境下的竞争问题；通过额外的 head_update 和 tail_update 来同步往尾部读取数据的trySteal函数，以确保队列的线程安全和数据的一致性。



![1](\picture\1.png)
