# Task 3: Virtual Memory System with Paging

## Overview

In Task 3, I've implemented a simple virtual memory system with paging. The program simulates a process with virtual memory addresses that are mapped to physical memory using a page table. The system handles page faults and implements the **LRU (Least Recently Used)** page replacement algorithm to bring pages into memory when a page fault occurs. The system also keeps track of which pages are in memory and handles page swaps with minimal overhead.

## Key Features

1. **Virtual Memory Simulation**: The program simulates virtual memory addresses and maps them to physical memory using a page table.
2. **Page Fault Handling**: When a page fault occurs (i.e., the requested page is not in memory), the system loads the required page from disk into memory.
3. **LRU Page Replacement**: The **Least Recently Used (LRU)** algorithm is used to decide which page to replace when a page fault occurs.
4. **Page Table Management**: The system maintains a page table to track which pages are in memory and their corresponding frames in physical memory.
5. **Disk Simulation**: The program simulates reading from and writing to disk when pages are swapped in and out of memory.

## Files and Their Roles

### 1. `virtual_memory.cpp`

This file contains the implementation of the virtual memory system. It defines a `VirtualMemory` class that handles virtual memory operations, including reading, writing, and page fault handling.
```
#include <iostream>
#include <unordered_map>
#include <list>
#include <vector>
#include <cstdint>

class VirtualMemory {
private:
    static const int PAGE_SIZE = 4096;
    static const int FRAME_COUNT = 128;
    static const int PAGE_COUNT = 256;

    struct Page {
        int frame_number = -1;
        bool present = false;
        bool dirty = false;
        std::vector<uint8_t> data;

        Page() : data(PAGE_SIZE, 0) {}
    };

    std::vector<Page> pages;
    std::vector<bool> frames;
    std::list<int> lru_list;
    std::unordered_map<int, std::list<int>::iterator> page_to_lru;

    int allocateFrame() {
        if (lru_list.size() < FRAME_COUNT) {
            for (int i = 0; i < FRAME_COUNT; i++) {
                if (!frames[i]) {
                    frames[i] = true;
                    return i;
                }
            }
        }
        
        int victim_page = lru_list.back();
        lru_list.pop_back();
        page_to_lru.erase(victim_page);
        
        int frame = pages[victim_page].frame_number;
        if (pages[victim_page].dirty) {
            writePageToDisk(victim_page);
        }
        
        pages[victim_page].present = false;
        pages[victim_page].frame_number = -1;
        return frame;
    }

    void updateLRU(int page_number) {
        if (page_to_lru.find(page_number) != page_to_lru.end()) {
            lru_list.erase(page_to_lru[page_number]);
        }
        lru_list.push_front(page_number);
        page_to_lru[page_number] = lru_list.begin();
    }

    void handlePageFault(int page_number) {
        int frame = allocateFrame();
        loadPageFromDisk(page_number, frame);
        pages[page_number].frame_number = frame;
        pages[page_number].present = true;
        pages[page_number].dirty = false;
        updateLRU(page_number);
    }

    void writePageToDisk(int page_number) {
        std::cout << "Writing page " << page_number << " to disk" << std::endl;
    }

    void loadPageFromDisk(int page_number, int frame) {
        std::cout << "Loading page " << page_number << " into frame " << frame << std::endl;
    }

public:
    VirtualMemory() : pages(PAGE_COUNT), frames(FRAME_COUNT, false) {}

    uint8_t read(int virtual_address) {
        int page_number = virtual_address / PAGE_SIZE;
        int offset = virtual_address % PAGE_SIZE;

        if (!pages[page_number].present) {
            std::cout << "Page fault on read: " << page_number << std::endl;
            handlePageFault(page_number);
        }

        updateLRU(page_number);
        return pages[page_number].data[offset];
    }

    void write(int virtual_address, uint8_t value) {
        int page_number = virtual_address / PAGE_SIZE;
        int offset = virtual_address % PAGE_SIZE;

        if (!pages[page_number].present) {
            std::cout << "Page fault on write: " << page_number << std::endl;
            handlePageFault(page_number);
        }

        pages[page_number].data[offset] = value;
        pages[page_number].dirty = true;
        updateLRU(page_number);
    }
};

int main() {
    VirtualMemory vm;
    
    // Test virtual memory operations
    vm.write(0, 42);
    vm.write(4096, 100);  // Write to second page
    vm.write(8192, 200);  // Write to third page
    
    std::cout << "Reading from address 0: " << (int)vm.read(0) << std::endl;
    std::cout << "Reading from address 4096: " << (int)vm.read(4096) << std::endl;
    std::cout << "Reading from address 8192: " << (int)vm.read(8192) << std::endl;

    return 0;
}
```cpp


#### How to Run:
1. Compile the file using a C++ compiler (e.g., `g++`):
   ```bash
   g++ virtual_memory.cpp -o virtual_memory
   ```
2. Run the compiled program:
   ```bash
   ./virtual_memory
   ```
3. The program will simulate virtual memory operations, including reading and writing to memory, handling page faults, and swapping pages using the LRU algorithm.
4. The output will display the results of memory operations, such as loading pages from disk, writing pages to disk, and reading values from memory.

#### Output:
![](output1.png)

---

## Conclusion

Task 3 focuses on implementing a virtual memory system with paging. The system simulates virtual memory addresses, handles page faults, and uses the LRU algorithm for page replacement. The program demonstrates key concepts in memory management, including:

- **Virtual Memory**: Simulating virtual addresses and mapping them to physical memory.
- **Page Fault Handling**: Loading pages from disk when they are not in memory.
- **LRU Algorithm**: Replacing the least recently used page when memory is full.

The `virtual_memory.cpp` file contains the complete implementation of the virtual memory system, and the instructions for compiling and running the program are provided above. This task provides a hands-on understanding of how operating systems manage memory and handle page faults efficiently.