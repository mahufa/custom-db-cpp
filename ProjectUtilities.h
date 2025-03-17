#pragma once
#include <map>
#include <__chrono/year_month_day.h>

#include "StructureElements.h"

namespace ProjectUtilities {
    auto prepareIndexedConditions(Table& table, std::vector<std::string> const& conditions) -> std::map<size_t, std::variant<std::string, int, float, std::chrono::year_month_day>>;

    auto getMatchingRows(Table& table, std::map<size_t, std::variant<std::string, int, float, std::chrono::year_month_day>> const& indexedConditions) -> std::vector<Row*>;
}
