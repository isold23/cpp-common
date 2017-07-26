#ifndef NONCOPYABLE_H
#define NONCOPYABLE_H
/*
  *file NonCopyable.h
  *brief 这个是从boost库copy来的代码,用来实现防止拷贝构造的
  *      原理很简单就是私有化了构造函数和拷贝构造函数,让子类
  *      无法被拷贝构造
  *author isold.wang@gmail.com
  *version 0.1
  *date 2010.11.26
  */
class CNonCopyable
{
protected:
    CNonCopyable() {}
    ~CNonCopyable() {}
private:
    CNonCopyable(const CNonCopyable&);
    const CNonCopyable& operator= (const CNonCopyable&);
};

#endif
