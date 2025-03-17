
#include "ProjectUtilities.h"

auto ProjectUtilities::prepareIndexedConditions(Table& table, std::vector<std::string> const& conditions) -> std::map<size_t, std::variant<std::string, int, float, std::chrono::year_month_day>>{

    auto indexedConditions = std::map<size_t, std::variant<std::string, int, float, std::chrono::year_month_day>>();
    auto pos = std::string::npos;

    if (!conditions.empty()) {
        for (auto const& condition : conditions) {
            pos = condition.find('=');
            if (pos != std::string::npos) {
                auto const conditionColumnName = condition.substr(0, pos);
                if (!table.columns.contains(conditionColumnName)) {
                    throw std::invalid_argument("Column <{}> in condition doesn't exist!");
                }

                auto const conditionValueStr = condition.substr(pos + 1);

                auto conditionValue = std::variant<std::string, int, float, std::chrono::year_month_day>();
                auto columnType = table.columns[conditionColumnName].type;
                try {
                    conditionValue = createCell(columnType, conditionValueStr);
                } catch (std::invalid_argument&) {
                    throw std::invalid_argument("Incorrect column type!");
                }

                auto conditionIndex = table.columns[conditionColumnName].index;

                indexedConditions[conditionIndex] = conditionValue;
            }else {
                throw std::invalid_argument("Incorrect condition! (usage: someColumn=val) ");
            }
        }
    }
    return indexedConditions;
}

auto ProjectUtilities::getMatchingRows(Table& table, std::map<size_t, std::variant<std::string, int, float, std::chrono::year_month_day>> const& indexedConditions) -> std::vector<Row*> {

    auto rows = std::vector<Row*>();
    auto match = true;

    for (auto i = table.data.begin() ; i != table.data.end() ; ++i) {
        for (auto const& [condIndex, condVal] : indexedConditions) {
            if ((*i)[condIndex] != condVal) {
                match = false;
                break;
            }
        }
        if (match) {
            rows.push_back(&*i);
        }else {
            match = true;
        }
    }

    return rows;
}
