#include "Table.hpp"

Table::Table(const std::string& name, std::ios::openmode mode, const Schema& schema)
    : filename(name), file(name, mode), schema(schema), rowStore(schema) {}

Table::~Table() {
    if (file.is_open()) {
        file.close();
    }
}

void Table::load() {
    TableData tempData;
    fileManager.read(file, schema, tempData);
    rowStore.loadData(tempData);
}

void Table::flush() {
    fileManager.write(file, rowStore.getData());
}

RowStore& Table::getRowStore() {
    return rowStore;
}

const Schema& Table::getSchema() const {
    return schema;
}