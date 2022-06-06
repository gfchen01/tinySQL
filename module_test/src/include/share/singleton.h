//
// Created by luke on 22-6-6.
//

#ifndef TINYSQL_TEST_SINGLETON_H
#define TINYSQL_TEST_SINGLETON_H

template <class SingletonClass>
class Singleton{
public:
    static SingletonClass * instance(){
        static SingletonClass instance;
        return &instance;
    }
    SingletonClass* operator ->() { return instance(); }
    const SingletonClass* operator ->() const { return instance(); }
private:
    Singleton(){}
    ~Singleton(){}
};

#endif //TINYSQL_TEST_SINGLETON_H
