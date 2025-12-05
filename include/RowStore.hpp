#pragma once
#include <vector>
#include <string>
#include <optional>
#include <functional>
#include "Schema.hpp"

class RowStore {
    private:
        TableData table_data;
        Schema schema;
        
        bool validateRow(const Row& row) const;

    public:
        RowStore(const Schema& schema);
        
        TableData& getData();
        const TableData& getData() const;
        
        void loadData(const TableData& data);
        
        void clear();
        
        bool insert(const Row& row);
        
        std::optional<Row> getRow(size_t index) const;
        
        TableData select(std::function<bool(const Row&)> predicate) const;
        
        bool update(size_t index, const Row& newRow);
        
        size_t updateWhere(
            std::function<bool(const Row&)> predicate,
            std::function<Row(const Row&)> updateFunc
        );
        
        bool deleteRow(size_t index);
        
        size_t deleteWhere(std::function<bool(const Row&)> predicate);
        
        void printAll() const;
        size_t rowCount() const;
};