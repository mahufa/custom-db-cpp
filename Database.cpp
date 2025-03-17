#include "Database.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <__ranges/transform_view.h>

#include "ProjectUtilities.h"
#include "fmt/base.h"


auto Database::create(std::string const& name, std::vector<std::string> const& columnNames, std::vector<std::string> const& columnTypes) -> void {
    if (tables.contains(name)) {
        fmt::println( "Table <{}> already exists!" , name);
    }else {
        auto table = Table();
        for (auto i = 0; i < columnNames.size(); i++) {
            table.columns[columnNames[i]] = ColumnData(table.columns.size(), columnTypes[i]);
        }
        tables[name] = table;
        fmt::println("Table <{}> created.", name);
    }
}

auto Database::add(std::string const& tableName, std::string const& columnName, std::string const& columnType) -> void {
    if (!tables.contains(tableName)) {
        fmt::println("Table <{}> doesn't exist!", tableName);
        return;
    }

    auto& table = tables[tableName];

    if (table.columns.contains(columnName)) {
        fmt::println( "Column <{}> already exists in table <{}>!", columnName, tableName);
        return;
    }

    table.columns[columnName] = ColumnData(table.columns.size(), columnType);

    for (auto& row : table.data) {
        row.addEmptyCell();
    }
    fmt::println( "Column <{}> added to table <{}>.", columnName, tableName);
}

auto Database::drop(std::string const& name) -> void{
    if (!tables.contains(name)) {
        fmt::println("Table <{}> doesn't exist!", name);
    }else{
        tables.erase(name);
        fmt::println("Table <{}> deleted.", name);
    }
}


auto Database::insert(std::string const& tableName, std::vector<std::string> const& values) -> void {
    if (!tables.contains(tableName)) {
        fmt::println("Table <{}> doesn't exist!", tableName);
        return;
    }

    auto& table = tables[tableName];

    if (values.size() != table.columns.size()) {
        fmt::println("Incorrect number of values!");
        return;
    }

    try {
        table.data.push_back(rowFromValues(table, values));
        fmt::println("Data inserted correctly.");

    } catch (std::out_of_range&) {
        fmt::println("Argument out of range!");
    }catch (std::invalid_argument&) {
        fmt::println("Incorrect data type!");
    }
}


auto Database::select(std::string const& tableName,
                    std::vector<std::string> const& queriedColumns,
                    std::vector<std::string> const& conditions)
 -> void {
    if (!tables.contains(tableName)) {
        fmt::println("Table <{}> doesn't exist!", tableName);
        return;
    }

    auto& table = tables[tableName];

    for (const auto& column : queriedColumns) {
        if (!table.columns.contains(column)) {
            fmt::println("Column <{}> doesn't exist!", column);
            return;
        }
    }

    auto indexedConditions = std::map<size_t, std::variant<std::string, int, float, std::chrono::year_month_day>>();
    try {
        indexedConditions = ProjectUtilities::prepareIndexedConditions(table, conditions);
    }catch (std::invalid_argument& e) {
        fmt::println("{}", e.what());
        return;
    }


    auto rowsToDisplay = ProjectUtilities::getMatchingRows(table, indexedConditions);

    for (auto const& row : rowsToDisplay) {
        for (auto const& col : queriedColumns) {
            auto& currCell = (*row)[table.columns[col].index];
            std::visit([](auto& val) {
                using namespace std::chrono;
                using T = std::decay_t<decltype(val)>; // decay_t source - chat gpt
                if constexpr (std::is_same_v<T, year_month_day>) {    //constexpr source - chat gpt
                    const auto date = static_cast<year_month_day>(val);
                    fmt::print("{}.{}.{}\t", static_cast<unsigned>(date.day()),
                                                    static_cast<unsigned>(date.month()),
                                                    static_cast<int>(date.year()));
                }else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, int> || std::is_same_v<T, float>) {
                    fmt::print("{}\t", val);
                }},
                currCell);
        }
        fmt::print("\n");
    }
}

auto Database::deleteRows(std::string const& tableName, std::vector<std::string> const& conditions) ->void {
    if (!tables.contains(tableName)) {
        fmt::println("Table <{}> doesn't exist!", tableName);
        return;
    }

    Table& table = tables[tableName];

    auto indexedConditions = std::map<size_t, std::variant<std::string, int, float, std::chrono::year_month_day>>();
    try {
        indexedConditions = ProjectUtilities::prepareIndexedConditions(table, conditions);
    }catch (std::invalid_argument& e) {
        fmt::println("{}", e.what());
        return;
    }

    auto rowsToDelete = ProjectUtilities::getMatchingRows(table, indexedConditions);

    table.data.erase(
    std::ranges::remove_if(table.data,[&rowsToDelete](const Row& row)
        {return std::ranges::any_of(rowsToDelete,[&row](const Row* rowPtr)
            { return &row == rowPtr;});}).begin(),
    table.data.end());

    fmt::println("Data deleted correctly.");
}

auto Database::save(std::string const& path) -> void{
    auto file = std::ofstream(path, std::ios::binary);
    if (!file) {
        fmt::println("Failed to open file!");
        return;
    }

    auto tablesLenght = tables.size();
    file.write(reinterpret_cast<const char*>(&tablesLenght), sizeof(tablesLenght));

    for (auto i = tables.begin(); i != tables.end(); i++) {
        auto tableNameSize = i->first.size();
        file.write(reinterpret_cast<char*>(&tableNameSize), sizeof(tableNameSize));
        file.write(i->first.c_str(), tableNameSize);

        i->second.saveToFile(file);
    }
    fmt::println("Saved successfully.");
}

void Database::load(std::string const& path) {
   auto file = std::ifstream(path, std::ios::binary);
    if (!file) {
        fmt::println("Failed to open file!");
        return;
    }

    auto tablesLenght = size_t();
    file.read(reinterpret_cast<char*>(&tablesLenght), sizeof(tablesLenght));

    for (auto i = 0; i < tablesLenght; i++) {
        auto tableNameSize = size_t();
        file.read(reinterpret_cast<char*>(&tableNameSize), sizeof(tableNameSize));
        auto key = std::string();
        key.resize(tableNameSize);
        file.read(&key.front(), tableNameSize);

        tables[key] = Table::loadFromFile(file);
    }

    fmt::println("Loaded successfully.");
}

void Database::execute(std::string const& query) {
    auto stream = std::istringstream(query);
    auto command = std::string();
    stream >> command;

    if (command == "CREATE") {
        auto tableName = std::string();
        auto line = std::string();

        stream >> tableName;
        std::getline(stream, line);
        auto columnsWithTypes = line.substr(line.find('(') + 1, line.find(')') - line.find('(') - 1);

        auto columnNames = std::vector<std::string>();
        auto columnTypes = std::vector<std::string>();
        auto colStream = std::istringstream(columnsWithTypes);

        auto columnData = std::string();
        auto name = std::string();
        auto type = std::string();
        while (std::getline(colStream, columnData, ',')) {
            auto columnDataStream = std::istringstream(columnData);
            columnDataStream >> name >> type;
            columnNames.push_back(name);
            columnTypes.push_back(type);
        }

        create(tableName, columnNames, columnTypes);

    }  else if (command == "ADD") {
        auto tableName = std::string();
        auto columnName = std::string();
        auto type = std::string();

        stream >> tableName >> columnName >> type;
        add(tableName, columnName, type);

    }else if (command == "DROP") {
        auto tableName = std::string();
        stream >> tableName;
        drop(tableName);

    } else if (command == "INSERT") {
        auto tableName = std::string();
        auto valuesLine = std::string();

        stream >> tableName;
        std::getline(stream, valuesLine);
        auto nakedValues = valuesLine.substr(valuesLine.find('(') + 1, valuesLine.find(')') - valuesLine.find('(') - 1);

        std::vector<std::string> values;
        auto valuesStream = std::istringstream(nakedValues);
        auto value = std::string();
        while (valuesStream >> value) {
            value.erase(value.find_last_not_of(',' )+ 1);
            values.push_back(value);
        }

        insert(tableName, values);

    } else if (command == "SELECT") {
        auto currStr = std::string();
        auto queriedColumns = std::vector<std::string>();
        auto tableName = std::string();

        while (stream >> currStr && currStr != "FROM") {
            currStr.erase(currStr.find_last_not_of(',') + 1);
            queriedColumns.push_back(currStr);
        }
        if (currStr != "FROM") {
            fmt::println("Incorrect query! (unable to find \"FROM\"");
            return;
        }

        if (!(stream >> tableName)) {
            fmt::println("Incorrect query! (unable to find table name after \"FROM\")");
            return;
        }

        if (!(stream >> currStr)) {
            select(tableName, queriedColumns, std::vector<std::string>());
        } else if (currStr != "WHERE") {
            fmt::println("Incorrect query! (unable to find \"WHERE\" strictly after table name)");
        } else {
            auto conditions = std::vector<std::string>();
            while (stream >> currStr) {
                currStr.erase(currStr.find_last_not_of(',') + 1);
                conditions.push_back(currStr);
            }
            select(tableName, queriedColumns, conditions);
        }

    } else if (command == "DELETE") {
        auto tableName = std::string();
        auto currStr = std::string();

        stream >> tableName;

        if (!(stream >> currStr)) {
            deleteRows(tableName, std::vector<std::string>());
        } else if (currStr != "WHERE") {
            fmt::println("Incorrect query! (unable to find \"WHERE\" strictly after table name)");
        }else {
            auto conditions = std::vector<std::string>();
            while (stream >> currStr) {
                currStr.erase(currStr.find_last_not_of(',') + 1);
                conditions.push_back(currStr);
            }
            deleteRows(tableName, conditions);
        }

    } else if (command == "SAVE") {
        auto path = std::string();
        stream >> path;
        save(path);

    } else if (command == "LOAD") {
       auto path = std::string();
        stream >> path;
        load(path);
    } else {
        fmt::println("Incorrect command!");
    }
}