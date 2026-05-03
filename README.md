# CodexDB

CodexDB is a very simple DBMS. It has one built-in table and can only perform basic CRUD operations without any fancy keywords like WHERE, JOIN, or ORDER BY, but it works. I built it primarily for fun and learning, and I actually learned a lot along the way. I faced different challenges like B+trees, linked lists, managing raw memory, and database internals. Here is a quick explanation of how this project works:

This project is structured into three primary modules:
- **Database Engine**
- **Index**
- **Frontend**

---

# Database Engine

The Database Engine is the core of the system. It handles disk I/O, manages pages, and executes CRUD operations.

We store data in tables (currently limited to one). Since tables can grow quite large, we cannot load an entire table into memory. Instead, we break data into **Pages** (typically 4KB, 8KB, or 16KB). I chose 4KB as the standard size because it aligns with standard hardware architecture and file system allocation units. We load only the specific pages we need into memory, modifying them and writing them back to disk as necessary.

## Page

To support variable-length records (such as strings), I implemented a **Slotted Page** architecture. The page header stores metadata, such as the page ID, parent pointer, slot count, and garbage count. The slots grow downward from the top, while the record data grows upward from the bottom. Each slot stores a pointer to the start of a record and its size, allowing us to track variable-sized data within raw memory.

```text
+------------------------+  <- Start of Page
| HEADER (Page Info)     |
|========================|
| SLOT DIRECTORY         |  (Indices pointing to data)
| (Grows Downward)       |
|------------------------|
| Entry 1 |  -> Ptr 1    |
|---------|              |
| Entry 2 |  -> Ptr 2    |
|---------|              |
| Entry 3 |  -> Ptr 3    |
|========================|
|                        |
|      FREE SPACE        |  (The gap that shrinks)
|                        |
|========================|
| RECORD 3 (Data)        |  <- Records added from 
|------------------------|     the bottom up
| RECORD 2 (Data)        |
|------------------------|
| RECORD 1 (Data)        |
+------------------------+  <- End of Page
```
# Database Engine Architecture

## 1. The Page: Slotted Architecture
The page is the fundamental unit of storage. To handle variable-sized records (such as strings), we utilize a **slotted page architecture**.

* **Page Header:** Stores critical metadata, including the Page ID, parent ID, slot count, and garbage collection metrics.
* **Slots:** Stored at the beginning of the page and grow downward. Each slot contains a pointer to the start of a record and the record's length.
* **Records:** Variable-sized data stored within the page, growing upward from the bottom.
* **Why this structure?** Because records vary in size, we cannot determine specific offsets via simple arithmetic. Fixed-size slots provide a consistent index to track where each record begins and ends.

## 2. The Pager
The Pager acts as the abstraction layer for I/O operations. Its responsibility is simplified to managing the physical movement of pages:
* **Read:** Loading a page from disk into memory.
* **Write:** Flushing a page from memory to disk.

## 3. The Table: File Management
A table is represented as a file containing a collection of pages. It handles the organizational heavy lifting:
* **Page Allocation/Deallocation:** Manages the creation of new pages and the recycling of empty pages.
* **Dirty Bits:** Maintains a bitset to track which pages have been modified in memory. This ensures we only write back modified pages to the disk, significantly improving performance.
* **SuperBlock:** The mandatory first page of the file. It acts as the anchor, containing table metadata such as the location of the root page and the first free page.

## 4. The Cursor: Traversal Mechanism
The Cursor is a utility for data iteration rather than storage.
* **Role:** Acts as a pointer to a specific record.
* **Functionality:** Includes an `advance()` method, allowing the engine to traverse through records sequentially. This is essential for operations like `SELECT *`.

# The Index: B+Tree Implementation
The Index utilizes a B+Tree structure to optimize query performance from $O(n)$ linear scans to $O(\log n)$ tree traversals. It is a self-balancing tree consisting of two node types:

* **Internal Nodes:** Act as signposts. They guide the search algorithm to the correct path based on key ranges.
* **Leaf Nodes:** Contain the actual data records. All leaf nodes exist on the same level and are linked together to facilitate efficient range scans.

```text
                  [              40             ]  <-- ROOT (Directs the search)
                 /                               \
        +-------|---------+             +--------|--------+
        |   10  |    20   |             |   50   |   70   |  <-- INTERNAL (Guide keys)
        +---|---|---|-----+             +---|----|---|----+
           /     |     \                     /      |      \
    [ 5,8 ]<->[10,15]<->[ 20,30 ]     [ 50,60 ]<->[65,68]<->[ 75,80 ]
       ^         (Linked)     ^              (Linked)   (Linked)    ^
       |                      |                                     |
       +------------------ LEAF NODES (Actual Data Storage) --------+
```

# Frontend Architecture

The **Frontend** serves as the primary interface and gatekeeper of the database system. Its core responsibility is to receive raw user input, parse it, and translate it into a structured format that the underlying B-Tree engine can process.



---

## 1. The Role of the Frontend
The Frontend functions as the translation layer between user intent (SQL-like commands) and low-level system operations. The process follows these logical steps:

1.  **Input Parsing:** The frontend tokenizes the input query to determine the intended CRUD operation (Create, Read, Update, Delete).
2.  **Statement Construction:** It extracts values from the query and packs them into a standardized `Statement` object.
3.  **Method Mapping:** It maps the parsed operation to the corresponding B-Tree API method.
4.  **Handoff:** The constructed statement is passed to the **Executor**.

## 2. The `Statement` Structure
To ensure consistent communication between the frontend and the backend, all operations are encapsulated within the `Statement` struct. This structure provides the necessary context for the B-Tree to perform the requested action.

```cpp
struct Statement {
    StatementType type;    // Enumeration of the operation (e.g., INSERT, SELECT)
    Row row;               // The data payload, if applicable
    uint32_t target_id;    // Key identifier for specific targeted operations
};
