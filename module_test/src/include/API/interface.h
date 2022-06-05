//
// Created by luke on 22-6-3.
//

#ifndef TINYSQL_TEST_INTERFACE_H
#define TINYSQL_TEST_INTERFACE_H

#include "API/exec_engine.h"
#include "hsql/SQLParser.h"

class Interface{
public:
    Interface(){
        executor = new Exec_Engine();
    }
    void run();

private:
    Exec_Engine *executor;
};

#endif //TINYSQL_TEST_INTERFACE_H
