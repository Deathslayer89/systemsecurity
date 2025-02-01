// Virtual Memory System with Paging
#include <iostream>
#include <vector>
#include <unordered_map>
#include <list>
#include <cstdint>

const size_t PAGE_SIZE = 4096;
const size_t NUM_PAGES = 256;
const size_t NUM_FRAMES = 128;

struct Page {
    uint8_t data[PAGE_SIZE];
    bool dirty = false;
    bool present = false;
};

struct PageTableEntry {
    uint32_t frame_number;
    bool present;
    bool dirty;
};

class VirtualMemory {
private:
    std::vector<Page> physical_memory;
    std::vector<PageTableEntry> page_table;
    std::list<uint32_t> lru_list;
    std::unordered_map<uint32_t, std::list<uint32_t>::iterator> page_to_lru;
    size_t free_frames;

public:
    VirtualMemory() : 
        physical_memory(NUM_FRAMES),
        page_table(NUM_PAGES),
        free_frames(NUM_FRAMES) {
        for (auto& pte : page_table) {
            pte.present = false;
            pte.dirty = false;
        }
    }

    uint8_t read(uint32_t virtual_address) {
        uint32_t page_number = virtual_address / PAGE_SIZE;
        uint32_t offset = virtual_address % PAGE_SIZE;
        
        if (!page_table[page_number].present) {
            handlePageFault(page_number);
        }
        
        // Update LRU
        updateLRU(page_number);
        
        uint32_t frame_number = page_table[page_number].frame_number;
        return physical_memory[frame_number].data[offset];
    }

    void write(uint32_t virtual_address, uint8_t value) {
        uint32_t page_number = virtual_address / PAGE_SIZE;
        uint32_t offset = virtual_address % PAGE_SIZE;
        
        if (!page_table[page_number].present) {
            handlePageFault(page_number);
        }
        
        // Update LRU and mark page as dirty
        updateLRU(page_number);
        page_table[page_number].dirty = true;
        
        uint32_t frame_number = page_table[page_number].frame_number;
        physical_memory[frame_number].data[offset] = value;
        physical_memory[frame_number].dirty = true;
    }

private:
    void handlePageFault(uint32_t page_number) {
        uint32_t frame_number;
        
        if (free_frames > 0) {
            frame_number = NUM_FRAMES - free_frames;
            free_frames--;
        } else {
            // Use LRU to select page to evict
            uint32_t victim_page = lru_list.back();
            frame_number = page_table[victim_page].frame_number;
            
            // Write back if dirty
            if (page_table[victim_page].dirty) {
                writePageToDisk(victim_page, frame_number);
            }
            
            page_table[victim_page].present = false;
            lru_list.pop_back();
            page_to_lru.erase(victim_page);
        }
        
        // Load page from disk
        loadPageFromDisk(page_number, frame_number);
        page_table[page_number].frame_number = frame_number;
        page_table[page_number].present = true;
        page_table[page_number].dirty = false;
    }

    void updateLRU(uint32_t page_number) {
        if (page_to_lru.find(page_number) != page_to_lru.end()) {
            lru_list.erase(page_to_lru[page_number]);
        }
        lru_list.push_front(page_number);
        page_to_lru[page_number] = lru_list.begin();
    }

    void writePageToDisk(uint32_t page_number, uint32_t frame_number) {
        // Simulate writing to disk
        std::cout << "Writing page " << page_number << " to disk" << std::endl;
    }

    void loadPageFromDisk(uint32_t page_number, uint32_t frame_number) {
        // Simulate loading from disk
        std::cout << "Loading page " << page_number << " from disk to frame " 
                  << frame_number << std::endl;
    }
};

int main() {
    VirtualMemory vm;
    
    // Example usage
    vm.write(0, 42);
    vm.write(PAGE_SIZE, 84);
    
    std::cout << "Value at address 0: " << (int)vm.read(0) << std::endl;
    std::cout << "Value at address " << PAGE_SIZE << ": " 
              << (int)vm.read(PAGE_SIZE) << std::endl;
    
    return 0;
}