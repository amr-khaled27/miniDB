#pragma once
#include <string>
#include <fstream>
#include "RowStore.hpp"
#include "Schema.hpp"
#include "FileManager.hpp"

class Table {
private:
    std::string filename;
    std::fstream file;
    Schema schema;
    RowStore rowStore;
    FileManager fileManager;

public:
    Table(const std::string& name, std::ios::openmode mode, const Schema& schema);
    ~Table();
    
    void load();
    
    void flush();
    
    RowStore& getRowStore();
    
    const Schema& getSchema() const;
};
