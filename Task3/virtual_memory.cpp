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