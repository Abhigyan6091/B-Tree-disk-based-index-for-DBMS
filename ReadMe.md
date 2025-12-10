# B+ Tree Index Implementation

## CSL303 : Assignment
**BpTree** - B+ Tree disk-based index for database management systems

## SYNOPSIS
```c

void init(void);
int writeData(int key, unsigned char* data);
unsigned char* readData(int key);
int deleteData(int key);
unsigned char** readRangeData(int lowerKey, int upperKey, int* n);
void closeIndex(void);
```
DESCRIPTION
This implementation provides a persistent B+ Tree index stored on disk using memory-mapped I/O. The index supports integer keys and fixed-size 100-byte tuples, with a page size of 4096 bytes. The implementation is designed to handle datasets larger than available RAM by utilizing mmap for efficient disk-backed storage.

### Key Features
- **Persistent Storage**: All data is stored in `index.bin` and persists across program executions
- **Memory-Mapped I/O**: Uses mmap for efficient file access without explicit buffer management
- **Sorted Leaf Pages**: Enables efficient range queries through linked-list traversal
- **Automatic Page Splitting**: Handles overflow by splitting full pages and propagating changes

### Architecture
The implementation consists of several logical components:

1. **Disk Manager**: Manages file operations and memory-mapped access
2. **Page Structure**: Defines internal and leaf page layouts
3. **Page Utilities**: Provides functions for page manipulation
4. **B+ Tree Logic**: Implements tree operations (insert, delete, search, split)
5. **C API**: Exposes functions for external use

## SETUP

### Prerequisites
- Linux-based operating system (Ubuntu 20.04+ recommended)
- GCC compiler with C++11 support or later
- Standard system libraries (sys/mman.h, unistd.h, fcntl.h)

### System Requirements
- Minimum 128MB RAM
- Minimum 64MB free disk space for index file
- POSIX-compliant filesystem with mmap support

### Installation
No installation is required. Simply compile the source files as described in the next section.

## COMPILATION

### Basic Compilation
To compile the B+ Tree implementation and driver:

```bash
g++ -o db_engine driver.cpp c_api.cpp BPlusTree.cpp DiskManager.cpp 
```

### Compilation Flags Explained
- `-std=c++11`: Use C++11 standard (required for modern C++ features)

### Debug Build
For debugging purposes, compile with debug symbols:

```bash
g++ -std=c++11 -g -o db_engine driver.cpp c_api.cpp BPlusTree.cpp DiskManager.cpp -Wall -Wextra
```

### Makefile

```make           # Compile the project 
make clean     # Remove generated files and index
make  # this is the final executable file and usage is <./db_engine <.txt> >
#example
./db_engine [sequential_input.txt/random_input.txt] 
```

## EXECUTION

### Running the Program
After compilation, execute the test driver:


### First Run vs Subsequent Runs
- **First Run**: Creates a new `index.bin` file and initializes an empty B+ Tree
- **Subsequent Runs**: Opens existing `index.bin` and loads the persisted index structure

### Cleaning Up
To start fresh with an empty index:

```bash
rm index.bin
./db_engine [.txt]

```

## API REFERENCE

### writeData()
```c
int writeData(int key, unsigned char* data);
```
**Description**: Inserts a key-value pair into the index. If the key already exists, the insertion fails.

**Parameters**:
- `key`: Integer key for indexing (must be unique)
- `data`: Pointer to 100-byte tuple data

**Returns**:
- `1` on successful insertion
- `0` on failure (duplicate key or page overflow errors)


---

### readData()
```c
unsigned char* readData(int key);
```
**Description**: Searches for a key in the index and returns the associated tuple.

**Parameters**:
- `key`: Integer key to search for

**Returns**:
- Pointer to 100-byte tuple data if key exists
- `NULL` if key does not exist


```

---

### deleteData()
```c
int deleteData(int key);
```
**Description**: Removes a key and its associated tuple from the index. Uses lazy deletion .

**Parameters**:
- `key`: Integer key to delete

**Returns**:
- `1` on successful deletion
- `0` if key does not exist

```

---

### readRangeData()
```c
unsigned char** readRangeData(int lowerKey, int upperKey, int* n);
```
**Description**: Retrieves all tuples with keys in the range [lowerKey, upperKey] (inclusive).

**Parameters**:
- `lowerKey`: Starting key of the range
- `upperKey`: Ending key of the range
- `n`: Pointer to integer that will store the count of returned tuples

**Returns**:
- Array of pointers to 100-byte tuples if keys exist in range
- `NULL` if no keys exist in the specified range


```

## CONFIGURATION

### Constants (defined in source)
```c
const char* DB_FILE = "index.bin";           // Index file name
const int PAGE_SIZE = 4096;                   // Page size in bytes
const int TUPLE_SIZE = 100;                   // Fixed tuple size
const size_t MAX_DB_SIZE = 256L * 1024 * 1024; 
```

### File Descriptions


**All Files:**

-`common.h`: Defines shared data structures, constants, and configurations (like page size and memory limits) used across the entire project.
- `DiskManager.h` / `DiskManager.cpp`: Manages reading from and writing to the index.bin file on disk, handling memory mapping and page allocation.
- `BPlusTree.h` / `BPlusTree.cpp`: Implements the core B+ Tree data structure, including logic for inserting, finding, deleting, and scanning records.
- `c_api.h` / `c_api.cpp`: Provides a simple C-style interface (API) to the C++ B+ Tree, allowing other programs to use the database engine.
- `driver.cpp`: A command-line program that reads instructions from a file to test the performance and correctness of the B+ Tree implementation.
- `input_seq.py`: the python files for generatinng the sequential inputs for the driver
- `input_random.py`: the python files for generating the random inputs for the driver


**Build Files:**
- `Makefile` - Automated build configuration
- `README.md` - Project documentation (this file)


## FILE FORMAT

### index.bin Structure
The index file is organized as a sequence of 4096-byte pages:

```
Page 0: Root page (initially a leaf)
Page 1: Additional pages as needed
Page 2: ...
...
```

## PERFORMANCE CHARACTERISTICS

### Time Complexity
- **Insert**: O(log n) average, O(log n + split overhead) worst case
- **Search**: O(log n)
- **Delete**: O(log n) (lazy deletion, no merging)
- **Range Query**: O(log n + k) where k is the number of results

### Space Complexity
- the space complexity if it gives disk full then increase the space allowed for the DB file in the common.h
- Minimal RAM usage due to mmap (OS handles paging)
- Each page can store ~39 leaf entries or ~509 internal entries

## EXAMPLES

### Basic Usage Example

int main() {
    // Initialize
    init();
    
    // Insert data
    unsigned char data[100];
    memset(data, 'A', 100);
    writeData(10, data);
    
    // Read data
    unsigned char* result = readData(10);
    if (result) {

        delete[] result;
    }
    
    // Range query
    int count;
    unsigned char** range = readRangeData(5, 15, &count);
    if (range) {
        for (int i = 0; i < count; i++) {
            delete[] range[i];
        }
        delete[] range;
    }
    
    // Cleanup
    closeIndex();
    return 0;
}


## COPYRIGHT
This implementation is provided by group of CSL303 - Arpit Kumar, Arpit Bhomia ,Ashish ,Sarthak and Abhigyan.