//
// Created by luke on 22-6-2.
//

#include "hsql/SQLParser.h"
#include "catalog/catalog.h"
// #include "hsql/SQLParserResult.h"

int main()
{
    // Basic Usage Example
//    const std::string query = "SELECT * FROM Student WHERE student.age > 13 GROUP BY grade";
//    hsql::SQLParserResult result;
//    hsql::SQLParser::parse(query, &result);
//
//    if (result.isValid() && result.size() > 0) {
//        const hsql::SQLStatement* statement = result.getStatement(0);
//        if (statement->isType(hsql::kStmtSelect)) {
//            const auto* select = static_cast<const hsql::SelectStatement*>(statement);
//            /* ... */
//        }
//    }
    CatalogManager c_manager;
//    c_manager.CreateTable("Student",)
}