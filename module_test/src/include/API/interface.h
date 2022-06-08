//
// Created by luke on 22-6-3.
//

#ifndef TINYSQL_TEST_INTERFACE_H
#define TINYSQL_TEST_INTERFACE_H

#include "API/exec_engine.h"
#include "hsql/SQLParser.h"
#include <iostream>

class Interface{
public:
    Interface(std::istream &is, std::ostream &os):_is(is), _os(os){
        executor = new Exec_Engine();
    }
    ~Interface(){
        delete executor;
    }
    void run();
    void showErrMsg(db_err_t &dbErr);
    void serialOutput(std::vector<MemoryTuple> &tuples);
    void serialOutput(std::vector<MemoryTuple> &tuples, std::vector<std::string> &attr_names);

private:
    Exec_Engine *executor;
    std::istream &_is;
    std::ostream &_os;
    void parseWhere(hsql::Expr *Clause, std::vector<Where> &where_vec);
    /**
     * @brief Read from a SQL query file.
     * @param query [out] The output query.
     */
    void readFromFile(std::string &query);
};

#endif //TINYSQL_TEST_INTERFACE_H
