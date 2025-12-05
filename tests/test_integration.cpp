#include <cassert>
#include <iostream>
#include <filesystem>
#include "../include/Table.hpp"

void cleanup_test_files() {
    std::filesystem::remove("test_crud.db");
    std::filesystem::remove("test_persistence.db");
}

void test_crud_persistence() {
    std::cout << "Testing CRUD with persistence...\n";
    
    const std::string filename = "test_crud.db";
    Schema schema = {{"id", INT}, {"name", STRING}, {"salary", DOUBLE}};
    
    // Phase 1: Insert and flush
    {
        Table table(filename, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc, schema);
        
        assert(table.getRowStore().insert({1, std::string("Alice"), 50000.0}));
        assert(table.getRowStore().insert({2, std::string("Bob"), 60000.0}));
        assert(table.getRowStore().insert({3, std::string("Charlie"), 55000.0}));
        
        table.flush();
    }
    
    // Phase 2: Load, update, and flush
    {
        Table table(filename, std::ios::in | std::ios::out | std::ios::binary, schema);
        table.load();
        
        assert(table.getRowStore().rowCount() == 3);
        
        // Update Bob's salary
        assert(table.getRowStore().update(1, {2, std::string("Bob"), 65000.0}));
        
        // Delete Charlie
        assert(table.getRowStore().deleteRow(2));
        
        table.flush();
    }
    
    // Phase 3: Verify changes persisted
    {
        Table table(filename, std::ios::in | std::ios::out | std::ios::binary, schema);
        table.load();
        
        assert(table.getRowStore().rowCount() == 2);
        
        auto bob = table.getRowStore().getRow(1);
        assert(bob.has_value());
        assert(std::get<double>((*bob)[2]) == 65000.0);
    }
    
    std::cout << "✓ CRUD persistence test passed\n";
}

void test_complex_queries() {
    std::cout << "Testing complex queries...\n";
    
    const std::string filename = "test_persistence.db";
    Schema schema = {{"id", INT}, {"department", STRING}, {"salary", DOUBLE}};
    
    {
        Table table(filename, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc, schema);
        
        table.getRowStore().insert({1, std::string("Engineering"), 80000.0});
        table.getRowStore().insert({2, std::string("Sales"), 60000.0});
        table.getRowStore().insert({3, std::string("Engineering"), 90000.0});
        table.getRowStore().insert({4, std::string("HR"), 55000.0});
        table.getRowStore().insert({5, std::string("Engineering"), 75000.0});
        
        // Select Engineering department
        auto engineers = table.getRowStore().select([](const auto& row) {
            return std::get<std::string>(row[1]) == "Engineering";
        });
        assert(engineers.size() == 3);
        
        // Update salaries > 70000
        size_t updated = table.getRowStore().updateWhere(
            [](const auto& row) { return std::get<double>(row[2]) > 70000.0; },
            [](const auto& row) {
                return std::vector<std::variant<int, double, std::string>>{
                    std::get<int>(row[0]),
                    std::get<std::string>(row[1]),
                    std::get<double>(row[2]) * 1.1
                };
            }
        );
        assert(updated == 3);
        
        // Delete HR department
        size_t deleted = table.getRowStore().deleteWhere([](const auto& row) {
            return std::get<std::string>(row[1]) == "HR";
        });
        assert(deleted == 1);
        assert(table.getRowStore().rowCount() == 4);
        
        table.flush();
    }
    
    // Verify persistence
    {
        Table table(filename, std::ios::in | std::ios::out | std::ios::binary, schema);
        table.load();
        
        assert(table.getRowStore().rowCount() == 4);
        
        auto engineers = table.getRowStore().select([](const auto& row) {
            return std::get<std::string>(row[1]) == "Engineering";
        });
        assert(engineers.size() == 3);
    }
    
    std::cout << "✓ Complex queries test passed\n";
}

void test_edge_cases() {
    std::cout << "Testing edge cases...\n";
    
    Schema schema = {{"id", INT}};
    RowStore store(schema);
    
    // Empty store operations
    assert(store.rowCount() == 0);
    assert(!store.getRow(0).has_value());
    assert(!store.deleteRow(0));
    assert(!store.update(0, {1}));
    
    auto results = store.select([](const auto&) { return true; });
    assert(results.empty());
    
    // Single element operations
    assert(store.insert({1}));
    assert(store.rowCount() == 1);
    assert(store.deleteRow(0));
    assert(store.rowCount() == 0);
    
    std::cout << "✓ Edge cases test passed\n";
}

int main() {
    std::cout << "\n=== Integration Tests ===\n";
    cleanup_test_files();
    
    test_crud_persistence();
    test_complex_queries();
    test_edge_cases();
    
    cleanup_test_files();
    std::cout << "\n✓ All integration tests passed!\n";
    return 0;
}