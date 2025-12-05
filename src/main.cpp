#include <iostream>
#include "Table.hpp"

void exampleBasic() {
    std::cout << "\n=== Example 1: Basic Write and Read ===\n";
    
    Table employees(
        "employees.db",
        std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc,
        {{"id", INT}, {"name", STRING}, {"salary", DOUBLE}}
    );

    TableData data = {
        {1, std::string("Alice"), 98.5},
        {2, std::string("Bob"), 75.0},
        {3, std::string("Charlie"), 88.2}
    };

    employees.getRowStore().loadData(data);
    employees.flush();
    std::cout << "✓ Wrote 3 rows to disk\n";

    employees.getRowStore().clear();
    std::cout << "✓ Cleared in-memory data\n";

    employees.load();
    std::cout << "✓ Loaded " << employees.getRowStore().rowCount() << " rows from disk:\n";
    employees.getRowStore().printAll();
}

void exampleCRUD() {
    std::cout << "\n=== Example 2: CRUD Operations ===\n";
    
    Table employees(
      "employees_crud.db",
      std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc,
      {{"id", INT}, {"name", STRING}, {"salary", DOUBLE}}
    );

    // Create
    std::cout << "Creating records...\n";
    employees.getRowStore().insert({1, std::string("Alice"), 95000.0});
    employees.getRowStore().insert({2, std::string("Bob"), 75000.0});
    employees.getRowStore().insert({3, std::string("Charlie"), 85000.0});
    std::cout << "✓ Inserted 3 rows\n";

    // Read
    std::cout << "\nReading record at index 1:\n";
    auto row = employees.getRowStore().getRow(1);
    if (row.has_value()) {
      std::cout << "ID: " << std::get<int>((*row)[0]) 
                << ", Name: " << std::get<std::string>((*row)[1])
                << ", Salary: " << std::get<double>((*row)[2]) << "\n";
    }

    // Select with condition
    std::cout << "\nEmployees with salary > 80000:\n";
    auto highEarners = employees.getRowStore().select([](const auto& row) {
      return std::get<double>(row[2]) > 80000.0;
    });
    for (const auto& r : highEarners) {
      std::cout << "  " << std::get<std::string>(r[1]) << ": $" << std::get<double>(r[2]) << "\n";
    }

    // Update
    std::cout << "\nUpdating Bob's salary...\n";
    employees.getRowStore().update(1, {2, std::string("Bob"), 80000.0});
    std::cout << "✓ Updated\n";

    // Delete
    std::cout << "\nDeleting Alice...\n";
    employees.getRowStore().deleteRow(0);
    std::cout << "✓ Deleted. Rows remaining: " << employees.getRowStore().rowCount() << "\n";

    employees.getRowStore().printAll();

    // Persist
    employees.flush();
    std::cout << "\n✓ Persisted to disk\n"; 
}

int main() {
    exampleBasic();
    exampleCRUD();
    return 0;
}