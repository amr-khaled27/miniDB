# MiniDB - Foundational Database Storage Engine

A page-based database storage engine built in modern C++17, representing the foundational I/O layer that forms the basis of production database systems. This is my first systems engineering project, focusing on proper layering, type safety, and extensibility.

**Status:** Core storage primitives complete. Future expansions planned (B-Tree indexes, buffer pooling, transactions).

---

## Architecture Overview

### Layered Design Pattern

```
┌─────────────────────────────────────────┐
│         Application Layer               │  User code
│         (main.cpp)                      │
├─────────────────────────────────────────┤
│    Facade/Orchestrator Layer            │
│    Table - High-level API               │  ← Single entry point
├─────────────────────────────────────────┤
│    Business Logic Layer                 │
│    RowStore - In-memory operations      │  ← Data manipulation
├─────────────────────────────────────────┤
│    Persistence Layer                    │
│    FileManager - Serialization/I/O      │  ← Disk interaction
├─────────────────────────────────────────┤
│    Data Structure Layer                 │
│    Page - 4KB memory blocks             │  ← Storage format
├─────────────────────────────────────────┤
│         Physical Storage                │
│         Binary files on disk            │
└─────────────────────────────────────────┘

──────────────────────────────────────────────────────────────────────────────────

┌─────────────────────────────────────────┐
│           Table.hpp                     │  ← **Complete Table Entity**
│  (Orchestration Layer)                  │     User works here
│  - filename, file handle                │
│  - owns Schema                          │
│  - owns RowStore                        │
│  - owns FileManager                     │
│  - load() / flush()                     │
└────────┬─────────────┬──────────────────┘
         │             │
         │             │ delegates to
    ┌────▼────┐   ┌────▼────────┐
    │ Schema  │   │  RowStore   │        ← **In-Memory Operations**
    │         │   │             │           (insert, select, etc.)
    └─────────┘   └─────────────┘
                        │
                        │ uses for validation
                   ┌────▼────────┐
                   │  Schema     │        ← **Type System**
                   └─────────────┘
                        │
         ┌──────────────┴──────────────┐
         │                             │
    ┌────▼────────┐            ┌───────▼──────┐
    │FileManager  │            │  RowStore    │  ← **Dual Purpose**
    │             │            │              │
    └──────┬──────┘            └──────────────┘
           │
           │ uses
      ┌────▼────┐
      │  Page   │                      ← **Storage Primitive**
      └─────────┘
```

**Key Principle:** Each layer only knows about the layer directly below it. Changes to lower layers don't affect upper layers (Dependency Inversion).

---

## Design Patterns Used

### 1. **Facade Pattern** - `Table` Class
The Table class acts as a simplified interface to the complex subsystem:
- Hides complexity of RowStore, FileManager, and Page interactions
- Provides simple `load()` and `flush()` methods
- Manages file handles and coordinates between components
- Single point of contact for users

**Benefits:**
- Easy to use API
- Decouples user code from implementation details
- Can change internal structure without breaking user code

---

### 2. **Repository Pattern** - `RowStore` Class
Separates in-memory data manipulation from persistence logic:
- Manages collection of rows in memory
- Provides CRUD-like operations on data
- No knowledge of how data is stored on disk
- Pure business logic layer

**Benefits:**
- Easy to test (no I/O dependencies)
- Can swap storage backend without changing logic
- Clear separation of concerns

---

### 3. **Strategy Pattern** - Serialization
Different serialization strategies for different types:
- Integers: Fixed 4-byte binary
- Doubles: Fixed 8-byte binary
- Strings: Length-prefixed variable encoding

**Benefits:**
- Easy to add new data types
- Type-specific optimization
- Flexiable schema definition
- Extensible without modifying existing code

---

### 4. **RAII (Resource Acquisition Is Initialization)**
File handles and resources managed automatically:
- Table owns the file stream
- Destructor ensures proper cleanup
- No manual memory management needed
- Exception-safe resource handling

**Benefits:**
- No resource leaks
- Exception-safe code
- Automatic cleanup

---

### 5. **Data Transfer Object (DTO)** - `Row` Type
Simple data structure (`std::vector<std::variant>`) for moving data between layers.

**Benefits:**
- Type-safe heterogeneous data
- No coupling to database internals
- Easy to serialize/deserialize

---

## Architectural Principles

### 1. **Separation of Concerns**
Each class has exactly ONE responsibility:
- **Table:** Coordinate operations and manage file handle
- **RowStore:** Manipulate in-memory data
- **FileManager:** Handle serialization and disk I/O
- **Page:** Represent fixed-size memory blocks
- **Schema:** Define table structure

**Result:** Easy to test, understand, and modify each component independently.

---

### 2. **Single Responsibility Principle (SRP)**
No "bloated classes" that do everything. Each class is focused and cohesive.

**Example:** FileManager only knows about bytes and pages. It doesn't know what a "row" means semantically - that's RowStore's job.

---

### 3. **Dependency Inversion Principle**
High-level modules (Table) don't depend on low-level modules (Page). Both depend on abstractions (interfaces/APIs).

**Result:** Can swap implementations without breaking higher layers.

---

### 4. **Open/Closed Principle**
Open for extension, closed for modification.

**Example:** Adding a new data type requires adding cases to serialization, not modifying existing code paths.

---

### 5. **Encapsulation**
Internal implementation details are hidden:
- Page internals hidden from FileManager
- Serialization format hidden from RowStore
- File format hidden from Table users

**Benefits:** Can change internals without breaking API.

---

## Storage Architecture

### Page-Based Storage Model
**Inspiration:** PostgreSQL, MySQL InnoDB, SQLite

**Why pages?**
- Industry standard (4KB = OS page size)
- Efficient disk I/O (read/write in blocks)
- Easy to manage in memory
- Natural unit for caching/buffering

### Binary Serialization
**Why binary vs. text?**
- **Space efficiency:** 4 bytes for int vs. up to 11 bytes in text
- **Performance:** No parsing needed
- **Type preservation:** Exact representation of data
- **Industry standard:** All production databases use binary

### Multi-Page Management
**Automatic page allocation:**
- Start with one page
- When full, allocate next page
- FileManager handles all paging logic
- User code doesn't know pages exist (abstraction!)

**Scalability:** Can grow to any size, limited only by disk space.

---

## API Design Philosophy

### The Table Class - Orchestrator Pattern

**Core Philosophy:** Simple facade over complex operations

#### Usage Pattern
1. Create table with schema
2. Manipulate data through RowStore
3. Call `flush()` to persist to disk
4. Call `load()` to read from disk

**Design Goals:**
- Minimal API surface (only essential methods)
- Clear semantics (load = read, flush = write)
- No leaky abstractions (pages hidden from user)
- Resource safety (RAII handles file)

---

## Type System Design

### Why `std::variant` Over Alternatives

**Alternative 1: Fixed char[n] arrays** ❌
- Wastes space (short strings padded to n)
- No type safety
- Manual parsing/conversion

**Alternative 2: Inheritance hierarchy** ❌
- Requires virtual functions (runtime overhead)
- Heap allocations for each value
- Complex ownership semantics

**Alternative 3: `void*` with type tags** ❌
- Unsafe (wrong cast = crash)
- Manual memory management
- No compile-time checking

**My Choice: `std::variant`** ✅
- Zero-overhead abstraction
- Compile-time type safety
- Stack allocated (no heap)
- Expressive and safe

---

## Memory Management Strategy

### Zero Manual Memory Management

**Philosophy:** Let the compiler and STL manage memory

**No usage of:**
- `new` / `delete`
- Raw pointers for ownership
- Manual buffer allocation
- `malloc` / `free`

**Instead:**
- RAII for all resources
- STL containers for dynamic data
- Stack allocation where possible
- Smart pointers if heap needed (none currently)

**Result:** Zero memory leaks, exception-safe code.

---

## Cross-Platform Design

### Platform-Independent Code

**Used:**
- Standard C++17 only
- STL containers and algorithms
- Standard file I/O (`<fstream>`)
- No OS-specific headers

**Avoided:**
- Windows.h
- POSIX-specific calls
- Platform-specific optimizations
- OS-dependent file operations

**Result:** Compiles and runs on Linux, macOS, Windows, BSD, etc.

---

## Extensibility Points

### Where to Add Features

**1. New Data Types**

**2. Indexing Layer**

**3. Buffer Pool**

**4. Transactions**

**5. Query Language**

---

## Performance Characteristics

### Time Complexity
- **Insert (append):** O(1) amortized
- **Load all:** O(n) where n = number of rows
- **Flush all:** O(n) where n = number of rows
- **Page allocation:** O(1) per page

### Space Complexity
- **Per-row overhead:** Type tag + length prefix for strings
- **Per-page overhead:** 8 bytes (used_bytes metadata)
- **File header overhead:** 4 bytes (page count)
- **Space efficiency:** ~99.8% (4096 usable / 4104 total per page)

### I/O Efficiency
- Sequential writes (disk-friendly)
- Page-sized reads/writes (OS-optimal)
- No random seeks during normal operations
- Potential for read-ahead optimization

---

## Testing Strategy

### Current Approach
Manual verification through example programs in `main.cpp`

### Recommended Next Steps
1. **Unit Tests:** Test each class in isolation
2. **Integration Tests:** Test layer interactions
3. **Property Tests:** Verify serialization round-trips
4. **Stress Tests:** Large datasets, many pages
5. **Crash Recovery Tests:** Simulate failures

---

## Project Structure

```
database/
├── include/          # Header files
│   ├── Table.hpp          # Facade/Orchestrator
│   ├── RowStore.hpp       # Business logic
│   ├── FileManager.hpp    # Persistence
│   ├── Page.hpp           # Storage unit
│   └── Schema.hpp         # Type system
├── src/              # Implementation
│   ├── main.cpp           # Usage examples
│   ├── table.cpp
│   ├── rowStore.cpp
│   └── fileManager.cpp
└── README.md         # This file
```

---

## Build Instructions

```bash
# Run the provided script to make clean build
chmod +x clean-build.sh
./clean-build.sh

# Run examples
cd build
./minidb
```

**Requirements:**
- C++17 compliant compiler (GCC 7+, Clang 5+, MSVC 2017+)
- Standard library only (no external dependencies)

---

## Comparison to Production Databases

### What MiniDB Has
✅ Page-based storage (like PostgreSQL, MySQL)
✅ Binary serialization (like all production DBs)
✅ Type-safe values (like modern databases)
✅ Clean layered architecture
✅ Cross-platform code

### What MiniDB Lacks (By Design)
❌ SQL query language
❌ Indexes (planned)
❌ Transactions (planned)
❌ Concurrency control
❌ Query optimization
❌ Network protocol
❌ User management

**MiniDB implements the foundational storage layer - the I/O primitives upon which complete database systems are built.**

---

## Acknowledgments

This project was built to understand database internals through hands-on implementation. Inspiration drawn from:
- PostgreSQL's architecture
- SQLite's simplicity
- Modern C++ best practices
- Database internals literature

---

## Contributing

Feedback and suggestions welcome! Especially interested in:
- Architecture and design pattern feedback
- Performance optimization ideas
- Modern C++ usage improvements
- Ideas for the expansion roadmap

---

## License

MIT License - Free to use for learning and education

---

**Status:** Active Development (Core Complete)
**Version:** 1.0.0 - Storage Engine Foundation

---