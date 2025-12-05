#pragma once
#include <vector>
#include <string>
#include <fstream>
#include "Page.hpp"
#include "Schema.hpp"

class FileManager {
  private:
    size_t serializeRow(const Row& row, char* buffer, size_t bufferSize);

    size_t deserializeRow(const char* buffer, size_t bufferSize, const Schema& schema,Row& row);

    void writePage(std::fstream& file, const Page& page, size_t pageNum);

    bool readPage(std::fstream& file, Page& page, size_t pageNum);

    void writeHeader(std::fstream& file, uint32_t pageCount);
    uint32_t readHeader(std::fstream& file);

  public:
    void write(std::fstream& file, const TableData& tableData);
    void read(std::fstream& file, const Schema& schema, TableData& tableData);
};
