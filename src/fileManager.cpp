#include "FileManager.hpp"
#include <cstring>
#include <iostream>

constexpr size_t HEADER_SIZE = sizeof(uint32_t);
constexpr size_t DATA_PAGE_OFFSET = PAGE_SIZE + HEADER_SIZE;

size_t FileManager::serializeRow(
    const Row& row,
    char* buffer,
    size_t bufferSize)
{
    if (buffer == nullptr || bufferSize == 0) {
        return 0;
    }
    
    size_t offset = 0;
    
    for (const auto& field : row) {
        if (std::holds_alternative<int>(field)) {
            if (offset + sizeof(int) > bufferSize) return 0;
            int value = std::get<int>(field);
            std::memcpy(buffer + offset, &value, sizeof(int));
            offset += sizeof(int);
            
        } else if (std::holds_alternative<double>(field)) {
            if (offset + sizeof(double) > bufferSize) return 0;
            double value = std::get<double>(field);
            std::memcpy(buffer + offset, &value, sizeof(double));
            offset += sizeof(double);
            
        } else if (std::holds_alternative<std::string>(field)) {
            const std::string& s = std::get<std::string>(field);
            uint32_t len = s.size();
            
            // Validate string length to prevent overflow
            if (len > PAGE_SIZE || offset + sizeof(uint32_t) + len > bufferSize) return 0;
            
            std::memcpy(buffer + offset, &len, sizeof(uint32_t));
            offset += sizeof(uint32_t);
            std::memcpy(buffer + offset, s.data(), len);
            offset += len;
        }
    }
    
    return offset;
}

size_t FileManager::deserializeRow(
    const char* buffer,
    size_t bufferSize,
    const Schema& schema,
    Row& row)
{
    if (buffer == nullptr || bufferSize == 0) {
        return 0;
    }
    
    size_t offset = 0;
    row.clear();
    
    for (const auto& col : schema) {
        switch(col.second) {
            case INT: {
                if (offset + sizeof(int) > bufferSize) return 0;
                int value;
                std::memcpy(&value, buffer + offset, sizeof(int));
                offset += sizeof(int);
                row.push_back(value);
                break;
            }
            case DOUBLE: {
                if (offset + sizeof(double) > bufferSize) return 0;
                double value;
                std::memcpy(&value, buffer + offset, sizeof(double));
                offset += sizeof(double);
                row.push_back(value);
                break;
            }
            case STRING: {
                if (offset + sizeof(uint32_t) > bufferSize) return 0;
                uint32_t len;
                std::memcpy(&len, buffer + offset, sizeof(uint32_t));
                offset += sizeof(uint32_t);
                
                if (len > PAGE_SIZE || offset + len > bufferSize) return 0;
                
                std::string s(len, '\0');
                std::memcpy(s.data(), buffer + offset, len);
                offset += len;
                row.push_back(s);
                break;
            }
        }
    }
    
    return offset;
}

void FileManager::writeHeader(std::fstream& file, uint32_t pageCount) {
    file.seekp(0, std::ios::beg);
    file.write(reinterpret_cast<const char*>(&pageCount), sizeof(pageCount));
    file.flush();
}

uint32_t FileManager::readHeader(std::fstream& file) {
    file.seekg(0, std::ios::beg);
    uint32_t pageCount = 0;
    file.read(reinterpret_cast<char*>(&pageCount), sizeof(pageCount));
    if (file.fail()) {
        file.clear();
        return 0;
    }
    return pageCount;
}

void FileManager::writePage(std::fstream& file, const Page& page, size_t pageNum) {
    size_t offset = DATA_PAGE_OFFSET + (pageNum - 1) * (PAGE_SIZE + sizeof(size_t));
    
    file.seekp(offset, std::ios::beg);
    file.write(reinterpret_cast<const char*>(&page.used_bytes), sizeof(page.used_bytes));
    file.write(page.data.data(), PAGE_SIZE);
}

bool FileManager::readPage(std::fstream& file, Page& page, size_t pageNum) {
    size_t offset = DATA_PAGE_OFFSET + (pageNum - 1) * (PAGE_SIZE + sizeof(size_t));
    
    file.seekg(offset, std::ios::beg);
    
    file.read(reinterpret_cast<char*>(&page.used_bytes), sizeof(page.used_bytes));
    if (file.eof() || file.fail()) return false;
    
    file.read(page.data.data(), PAGE_SIZE);
    if (file.fail()) return false;
    
    return true;
}

void FileManager::write(std::fstream& file, const TableData& tableData) {
    file.seekp(0, std::ios::beg);
    
    Page currentPage;
    char tempBuffer[PAGE_SIZE];
    uint32_t pageCount = 0;
    
    for (const auto& row : tableData) {
        size_t rowSize = serializeRow(row, tempBuffer, PAGE_SIZE);
        
        if (rowSize == 0) {
            std::cerr << "Warning: Row too large to fit in a single page, skipping\n";
            continue;
        }
        
        if (!currentPage.hasSpace(rowSize)) {
            pageCount++;
            writePage(file, currentPage, pageCount);
            currentPage.clear();
        }
        
        std::memcpy(currentPage.getWritePtr(), tempBuffer, rowSize);
        currentPage.used_bytes += rowSize;
    }
    
    if (currentPage.used_bytes > 0) {
        pageCount++;
        writePage(file, currentPage, pageCount);
    }
    
    writeHeader(file, pageCount);
    
    file.flush();
}

void FileManager::read(std::fstream& file, const Schema& schema, TableData& tableData) {
    file.clear();
    tableData.clear();
    
    uint32_t pageCount = readHeader(file);
    
    if (pageCount == 0) {
        return;
    }
    
    for (uint32_t pageNum = 1; pageNum <= pageCount; pageNum++) {
        Page currentPage;
        
        if (!readPage(file, currentPage, pageNum)) {
            std::cerr << "Warning: Failed to read page " << pageNum << "\n";
            continue;
        }
        
        size_t offset = 0;
        
        while (offset < currentPage.used_bytes) {
            Row row;
            
            size_t bytesRead = deserializeRow(
                currentPage.getReadPtr(offset),
                currentPage.used_bytes - offset,
                schema,
                row
            );
            
            if (bytesRead == 0) break;
            
            tableData.push_back(row);
            offset += bytesRead;
        }
    }
}