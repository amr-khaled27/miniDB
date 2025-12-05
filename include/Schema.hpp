#pragma once
#include <vector>
#include <string>
#include <variant>

enum SupportedTypes {INT, DOUBLE, STRING};

using Row = std::vector<std::variant<int, double, std::string>>;

using TableData = std::vector<Row>;

using Schema = std::vector<std::pair<std::string, SupportedTypes>>;