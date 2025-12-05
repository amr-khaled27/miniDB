#include "RowStore.hpp"
#include <iostream>
#include <algorithm>

RowStore::RowStore(const Schema& schema) : schema(schema) {}

bool RowStore::validateRow(const Row &row) const {
    if (row.size() != schema.size()) return false;

    for (int i = 0; i < schema.size(); i++) {
        switch (schema[i].second) {
            case INT:
                if (!std::holds_alternative<int>(row[i])) return false;
            case DOUBLE:
                if (!std::holds_alternative<double>(row[i])) return false;
            case STRING:
                if (!std::holds_alternative<std::string>(row[i])) return false;

            default:
                break;
        }
    }

    return true;
}

TableData& RowStore::getData() {
    return table_data;
}

const TableData& RowStore::getData() const {
    return table_data;
}

void RowStore::loadData(const TableData& data) {
    table_data = data;
}

void RowStore::clear() {
    table_data.clear();
}

void RowStore::printAll() const {
    for (const auto& row : table_data) {
        for (const auto& field : row) {
            std::visit([](auto&& val) { std::cout << val << " "; }, field);
        }
        std::cout << "\n";
    }
}

size_t RowStore::rowCount() const {
    return table_data.size();
}

bool RowStore::insert(const Row& row) {
    if (!validateRow(row)) {
        return false;
    }
    
    table_data.push_back(row);
    return true;
}

std::optional<Row> RowStore::getRow(size_t index) const {
    if (index >= table_data.size()) {
        return std::nullopt;
    }
    return table_data[index];
}

TableData RowStore::select(std::function<bool(const Row&)> predicate) const {
    TableData result;
    
    for (const auto& row : table_data) {
        if (predicate(row)) {
            result.push_back(row);
        }
    }
    
    return result;
}

bool RowStore::update(size_t index, const Row& newRow) {
    if (index >= table_data.size()) {
        return false;
    }
    
    if (!validateRow(newRow)) {
        return false;
    }
    
    table_data[index] = newRow;
    return true;
}

size_t RowStore::updateWhere(
    std::function<bool(const Row&)> predicate,
    std::function<Row(const Row&)> updateFunc)
{
    size_t updateCount = 0;
    
    for (auto& row : table_data) {
        if (predicate(row)) {
            auto newRow = updateFunc(row);
            if (validateRow(newRow)) {
                row = newRow;
                updateCount++;
            }
        }
    }
    
    return updateCount;
}

bool RowStore::deleteRow(size_t index) {
    if (index >= table_data.size()) {
        return false;
    }
    
    table_data.erase(table_data.begin() + index);
    return true;
}

size_t RowStore::deleteWhere(std::function<bool(const Row&)> predicate) {
    size_t initialSize = table_data.size();
    
    table_data.erase(
        std::remove_if(table_data.begin(), table_data.end(), predicate),
        table_data.end()
    );
    
    return initialSize - table_data.size();
}