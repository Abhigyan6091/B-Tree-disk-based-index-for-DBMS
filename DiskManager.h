#ifndef DISK_MANAGER_H
#define DISK_MANAGER_H
#include "common.h"
class DiskManager {

    int fd;

    char* map_addr;
    int next_page_id;
public:
    DiskManager();
    ~DiskManager();
    Page* getPage(int page_id);

    int allocatePage();

    void sync();
};

#endif
