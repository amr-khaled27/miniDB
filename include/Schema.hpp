#pragma once
#include <vector>
#include <string>
#include <variant>

enum SupportedTypes {INT, DOUBLE, STRING, BOOLEAN,TIMESTAMP};

using Row = std::vector<std::variant<int, double, std::string, bool, long long>>;

using TableData = std::vector<Row>;

using Schema = std::vector<std::pair<std::string, SupportedTypes>>;