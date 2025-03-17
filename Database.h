#pragma once
#include <map>

#include "StructureElements.h"


struct Database {
    std::map<std::string, Table> tables;

    auto execute(std::string const& query) -> void;

private:
    auto create(std::string const& name, std::vector<std::string> const& columnNames, std::vector<std::string> const& columnTypes) -> void;
    auto add(std::string const& tableName, std::string const& columnName, std::string const& columnType) ->void;
    auto drop(std::string const& name) -> void;
    auto insert(std::string const& tableName, std::vector<std::string> const& values) -> void;
    auto select(std::string const& tableName, std::vector<std::string> const& queriedColumns, std::vector<std::string> const& conditions) -> void;
    void deleteRows(std::string const& tableName, std::vector<std::string> const& conditions);
    auto save(std::string const& path) -> void;
    auto load(std::string const& path) -> void;
};
