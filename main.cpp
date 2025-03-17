/*
- creating tables: CREATE  table (column1 type1, column2 type2, ...)
- deleting tables: DROP table;
- adding columns: ADD table column type
- adding data: INSERT table (value1, value2, ...)
- selecting data: SELECT column1, column2, ... FROM table WHERE someColumn=value
- deleting data: DELETE table WHERE someColumn="value", ...   conditions after WHERE in both cases works as them conjunction
- saving: SAVE path
- loading: LOAD path
*/

#include <iostream>
#include <string>
#include "Database.h"
#include "fmt/args.h"


auto main() -> int {
    auto db = Database();
    auto query = std::string();

    while (query != "EXIT") {
        fmt::print("# ");
        std::getline(std::cin, query);

        db.execute(query);
    }
}
