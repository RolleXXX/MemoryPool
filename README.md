# MemoryPool
一个多线程的高性能内存池

为了实现线程安全，又确保性能，采用了不同线程在不同内存段分配内存的方法

没有采用互斥锁实现了线程安全

MemoryPool含义一个vector，该vector根据线程数量需求创建相应个数的Thread_pool

主线程默认为第0号Thread_pool，子线程的Thread_pool从下标为1开始

该内存池采用智能指针进行管理，防止在一些小作用域里频繁申请释放内存，忘记回收，造成泄漏的情况

MemoryPool提供make_shared接口来创建对象，线程池主要逻辑实现在MemoryAlloc件中

注意：在子线程调用函数时，需要传入id号，来找寻该线程的内存分配区，具体调用可以查看test.cpp文件



![image](https://github.com/RolleXXX/MemoryPool/blob/master/%E5%86%85%E5%AD%98%E6%B1%A0.png)
