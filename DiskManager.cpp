#include "DiskManager.h"
#include <iostream>
#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/mman.h>
#include <cstdlib>

const char* DB_FILE = "index.bin";



DiskManager::DiskManager() {

    fd = open(DB_FILE, O_RDWR | O_CREAT, 0644);
    if (fd < 0) { perror("DB Open Failed"); exit(1); }

    struct stat st;

    fstat(fd, &st);

    
    if (st.st_size < (off_t)MAX_DB_SIZE) {

        if (ftruncate(fd, MAX_DB_SIZE) != 0) { perror("Resize Failed"); exit(1); }
    }

    map_addr = (char*)mmap(NULL, MAX_DB_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map_addr == MAP_FAILED) { perror("mmap Failed"); exit(1); }


    Page* meta = getPage(0);

    if (meta->getHeader()->page_type == PAGE_INVALID) {

        next_page_id = 1; 
    } else {

        MetaPageData* mp = reinterpret_cast<MetaPageData*>(meta->data + sizeof(PageHeader));
        next_page_id = mp->total_pages_allocated;

        if (next_page_id < 1) next_page_id = 1;
    }
}


DiskManager::~DiskManager() {
    if (map_addr != MAP_FAILED) {
        Page* meta = getPage(0);

        if (meta->getHeader()->page_type == PAGE_META) {
            MetaPageData* mp = reinterpret_cast<MetaPageData*>(meta->data + sizeof(PageHeader));

            mp->total_pages_allocated = next_page_id;
        }
        msync(map_addr, MAX_DB_SIZE, MS_SYNC);
        munmap(map_addr, MAX_DB_SIZE);
    }
    if (fd > 0) close(fd);
}

Page* DiskManager::getPage(int page_id) {

    if (page_id < 0 || (long)page_id * PAGE_SIZE >= MAX_DB_SIZE) return nullptr;

    return reinterpret_cast<Page*>(map_addr + ((long)page_id * PAGE_SIZE));
}


int DiskManager::allocatePage() {

    int id = next_page_id++;

    if ((long)id * PAGE_SIZE >= MAX_DB_SIZE) {
        std::cerr << "Disk Full!" << std::endl;

        exit(1);

    }
    Page* p = getPage(id);

    std::memset(p->data, 0, PAGE_SIZE);

    
    Page* meta = getPage(0);
    if (meta->getHeader()->page_type == PAGE_META) {

        reinterpret_cast<MetaPageData*>(meta->data + sizeof(PageHeader))->total_pages_allocated = next_page_id;

    }

    return id;
}


void DiskManager::sync() { 
    msync(map_addr, MAX_DB_SIZE, MS_SYNC); 
}