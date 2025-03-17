#pragma once
#include <map>
#include <vector>
#include <chrono>
#include <fstream>

struct ColumnData {
    size_t index;
    std::string type;

    auto saveToFile(std::ofstream& file) -> void;
    static auto loadFromFile(std::ifstream& file) -> ColumnData;
};


struct Row : std::vector<std::variant<std::string, int, float, std::chrono::year_month_day>> {
    explicit Row(size_t size); // explicit source - chatGPT
    auto addEmptyCell() -> void;

    auto saveToFile(std::ofstream& file)  -> void;
    static auto loadFromFile(std::ifstream& file) -> Row;
};

struct Table {
    std::map<std::string, ColumnData> columns; //<column name, data>
    std::vector<Row> data;

    auto saveToFile(std::ofstream& file) -> void;
    static auto loadFromFile(std::ifstream& file) -> Table;
};

auto rowFromValues(Table const& table, std::vector<std::string> const& values) -> Row;
auto createCell(std::string const& type, std::string const& val) -> std::variant<std::string, int, float, std::chrono::year_month_day>;