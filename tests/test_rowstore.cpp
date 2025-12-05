#include <cassert>
#include <iostream>
#include "../include/RowStore.hpp"

void test_insert() {
    std::cout << "Testing insert...\n";
    Schema schema = {{"id", INT}, {"name", STRING}, {"score", DOUBLE}};
    RowStore store(schema);
    
    // Valid insert
    assert(store.insert({1, std::string("Alice"), 95.5}));
    assert(store.rowCount() == 1);
    
    // Invalid insert (wrong type)
    assert(!store.insert({std::string("invalid"), std::string("Bob"), 85.0}));
    assert(store.rowCount() == 1);
    
    // Invalid insert (wrong number of fields)
    assert(!store.insert({2, std::string("Charlie")}));
    assert(store.rowCount() == 1);
    
    std::cout << "✓ Insert tests passed\n";
}

void test_select() {
    std::cout << "Testing select...\n";
    Schema schema = {{"id", INT}, {"name", STRING}, {"score", DOUBLE}};
    RowStore store(schema);
    
    store.insert({1, std::string("Alice"), 95.5});
    store.insert({2, std::string("Bob"), 85.0});
    store.insert({3, std::string("Charlie"), 90.0});
    
    // Get single row
    auto row = store.getRow(1);
    assert(row.has_value());
    assert(std::get<int>((*row)[0]) == 2);
    
    // Get out of bounds
    assert(!store.getRow(10).has_value());
    
    // Select with predicate
    auto highScorers = store.select([](const auto& row) {
        return std::get<double>(row[2]) > 90.0;
    });
    assert(highScorers.size() == 1);
    
    std::cout << "✓ Select tests passed\n";
}

void test_update() {
    std::cout << "Testing update...\n";
    Schema schema = {{"id", INT}, {"name", STRING}, {"score", DOUBLE}};
    RowStore store(schema);
    
    store.insert({1, std::string("Alice"), 95.5});
    store.insert({2, std::string("Bob"), 85.0});
    
    // Valid update
    assert(store.update(0, {1, std::string("Alice"), 98.0}));
    auto row = store.getRow(0);
    assert(std::get<double>((*row)[2]) == 98.0);
    
    // Invalid update (wrong type)
    assert(!store.update(0, {std::string("invalid"), std::string("Alice"), 98.0}));
    
    // Update out of bounds
    assert(!store.update(10, {3, std::string("Dave"), 88.0}));
    
    // Update where
    size_t updated = store.updateWhere(
        [](const auto& row) { return std::get<int>(row[0]) == 2; },
        [](const auto& row) { 
            return std::vector<std::variant<int, double, std::string>>{
                std::get<int>(row[0]), 
                std::get<std::string>(row[1]), 
                std::get<double>(row[2]) + 5.0
            };
        }
    );
    assert(updated == 1);
    
    std::cout << "✓ Update tests passed\n";
}

void test_delete() {
    std::cout << "Testing delete...\n";
    Schema schema = {{"id", INT}, {"name", STRING}, {"score", DOUBLE}};
    RowStore store(schema);
    
    store.insert({1, std::string("Alice"), 95.5});
    store.insert({2, std::string("Bob"), 85.0});
    store.insert({3, std::string("Charlie"), 75.0});
    
    // Delete single row
    assert(store.deleteRow(1));
    assert(store.rowCount() == 2);
    
    // Delete out of bounds
    assert(!store.deleteRow(10));
    assert(store.rowCount() == 2);
    
    // Delete where
    size_t deleted = store.deleteWhere([](const auto& row) {
        return std::get<double>(row[2]) < 80.0;
    });
    assert(deleted == 1);
    assert(store.rowCount() == 1);
    
    std::cout << "✓ Delete tests passed\n";
}

int main() {
    std::cout << "\n=== RowStore Unit Tests ===\n";
    test_insert();
    test_select();
    test_update();
    test_delete();
    std::cout << "\n✓ All RowStore tests passed!\n";
    return 0;
}
