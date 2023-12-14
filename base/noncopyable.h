#pragma once
/*它通过私有化拷贝构造函数和赋值运算符来阻止类的拷贝。通过继承 noncopyable 类，可以防止派生类被复制。这样可以确保该类的派生类在任何情况下都不会被复制，保证了类的唯一性，有助于避免意外的拷贝和赋值操作。*/
class noncopyable
{
protected:
    noncopyable() {}
    ~noncopyable() {}
    private:
    noncopyable(const noncopyable&);
    const noncopyable &operator=(const noncopyable&);
};