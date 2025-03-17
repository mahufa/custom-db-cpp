#include "StructureElements.h"

#include <__ranges/elements_view.h>


auto ColumnData::saveToFile(std::ofstream& file) -> void {
    file.write(reinterpret_cast<const char*>(&index), sizeof(size_t));
    auto lenght = type.size();
    file.write(reinterpret_cast<const char*>(&lenght), sizeof(lenght));
    file.write(type.c_str(), lenght);
}

auto ColumnData::loadFromFile(std::ifstream& file) -> ColumnData {
    auto cd = ColumnData();

    file.read(reinterpret_cast<char*>(&cd.index), sizeof(size_t));

    auto lenght = size_t();
    file.read(reinterpret_cast<char*>(&lenght), sizeof(lenght));

    cd.type.resize(lenght);
    file.read(&cd.type.front(), lenght);
    return cd;
}



Row::Row(size_t size) {
    for (int i = 0; i < size; i++) {
        push_back(std::variant<std::string, int, float, std::chrono::year_month_day>());
    }
}

auto Row::addEmptyCell() -> void {
    push_back(std::variant<std::string, int, float, std::chrono::year_month_day>());
}

auto Row::saveToFile(std::ofstream& file)  -> void {
    auto lenght = size();
    file.write(reinterpret_cast<const char*>(&lenght), sizeof(lenght));

    for (std::variant<std::string, int, float, std::chrono::year_month_day>& cell : *this) {
        auto typeIndex = cell.index();
        file.write(reinterpret_cast<const char*>(&typeIndex), sizeof(typeIndex));

        std::visit([&file](auto& value) {
            using T = std::decay_t<decltype(value)>;

            if constexpr (std::is_same_v<T, int>) {
                file.write(reinterpret_cast<char*>(&value), sizeof(value));

            } else if constexpr (std::is_same_v<T, float>) {
                file.write(reinterpret_cast<char*>(&value), sizeof(value));

            } else if constexpr (std::is_same_v<T, std::string>) {
                auto strLength = value.size();
                file.write(reinterpret_cast<char*>(&strLength), sizeof(strLength));
                file.write(value.c_str(), strLength);

            } else if constexpr (std::is_same_v<T, std::chrono::year_month_day>) {
                using namespace std::chrono;

                auto year = static_cast<int>(value.year());
                auto month = static_cast<unsigned>(value.month());
                auto day = static_cast<unsigned>(value.day());

                file.write(reinterpret_cast<const char*>(&year), sizeof(year));
                file.write(reinterpret_cast<const char*>(&month), sizeof(month));
                file.write(reinterpret_cast<const char*>(&day), sizeof(day));
            }},
        cell);
    }
}

auto Row::loadFromFile(std::ifstream& file) -> Row {
    auto lenght = size_t();
    file.read(reinterpret_cast<char*>(&lenght), sizeof(lenght));

    auto row = Row(lenght);

    for (auto i = 0; i < lenght; i++) {
        auto typeIndex = size_t();
        file.read(reinterpret_cast<char*>(&typeIndex), sizeof(typeIndex));

        switch (typeIndex) {
            case 0: { // std::string
                auto strLength = size_t();
                file.read(reinterpret_cast<char*>(&strLength), sizeof(strLength));
                auto value = std::string();
                value.resize(strLength);
                file.read(&value.front(), strLength);
                row[i] = value;
                break;

            }
            case 1: { // int
                auto value = int();
                file.read(reinterpret_cast<char*>(&value), sizeof(value));
                row[i] = value;
                break;

            }
            case 2: { // float
                auto value = float();
                file.read(reinterpret_cast<char*>(&value), sizeof(value));
                row[i] = value;
                break;
            }
            case 3: { //date
                auto y = int();
                auto m = unsigned();
                auto d = unsigned();

                file.read(reinterpret_cast<char*>(&y), sizeof(y));
                file.read(reinterpret_cast<char*>(&m), sizeof(m));
                file.read(reinterpret_cast<char*>(&d), sizeof(d));

                using namespace std::chrono;
                auto date = year_month_day(year(y), month(m), day(d));
                row[i] = date;
            }
            default: throw std::io_errc();
        }
    }

    return row;
}

auto Table::saveToFile(std::ofstream &file)  -> void {
    auto columnsLenght = columns.size();
    file.write(reinterpret_cast<const char*>(&columnsLenght), sizeof(columnsLenght));

    for (auto i = columns.begin(); i != columns.end(); i++) {
        auto strLength = i->first.size();
        file.write(reinterpret_cast<char*>(&strLength), sizeof(strLength));
        file.write(i->first.c_str(), strLength);

        i->second.saveToFile(file);
    }

    auto dataLenght = data.size();
    file.write(reinterpret_cast<const char*>(&dataLenght), sizeof(dataLenght));

    for (auto i = 0; i < dataLenght; i++) {
        data[i].saveToFile(file);
    }
}

auto Table::loadFromFile(std::ifstream &file) -> Table {
    auto table = Table();

    auto columnsLenght = size_t();
    file.read(reinterpret_cast<char*>(&columnsLenght), sizeof(columnsLenght));

    for (auto i = 0; i < columnsLenght; i++) {
        auto strLength = size_t();
        file.read(reinterpret_cast<char*>(&strLength), sizeof(strLength));
        auto key = std::string();
        key.resize(strLength);
        file.read(&key.front(), strLength);

        table.columns[key] = ColumnData::loadFromFile(file);
    }

    auto dataLenght = size_t();
    file.read(reinterpret_cast<char*>(&dataLenght), sizeof(dataLenght));

    for (auto i = 0; i < dataLenght; i++) {
        table.data.push_back(Row::loadFromFile(file));
    }

    return table;
}




auto createCell(std::string const& type, std::string const& val) -> std::variant<std::string, int, float, std::chrono::year_month_day> {
    if (type == "string") {
        return  {val};
    }
    if (type == "int") {
        return  {std::stoi(val)};
    }
    if (type == "float") {
        return  {std::stof(val)};
    }
    if (type == "date") {
        auto const pos1 = val.find('.');
        auto const pos2 = val.find('.', pos1 + 1);

        auto const day = std::chrono::day(std::stoi(val.substr(0, pos1)));
        auto const month = std::chrono::month(std::stoi(val.substr(pos1+1, pos2)));
        auto const year = std::chrono::year(std::stoi(val.substr(pos2+1)));

        return {std::chrono::year_month_day(year, month, day)};
    }

    throw std::invalid_argument("Incorrect type!");
}

auto rowFromValues(Table const& table, std::vector<std::string> const& values) -> Row {
    auto row = Row(values.size());

    for(auto &[index, type]: table.columns | std::ranges::views::values) { //views source - chatGPT
        row[index] = createCell(type, values[index]);
    }

    return row;
}

