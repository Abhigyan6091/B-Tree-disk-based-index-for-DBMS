#ifndef COMMON_H

#define COMMON_H
#include <cstring>
#include <cstdint>

extern const char* DB_FILE;
const int PAGE_SIZE = 4096;
const int TUPLE_SIZE = 100;

const int INVALID_PAGE_ID = -1;

const long long MAX_DB_SIZE = 256L * 1024 * 1024; 
enum PageType { PAGE_INVALID = 0, PAGE_INTERNAL = 1, PAGE_LEAF = 2, PAGE_META = 3 };
struct PageHeader {
    int page_id;

    int parent_id;

    int page_type;
    int num_items;

    int next_leaf; 
    int extra_ptr; 
};

struct LeafEntry {
    int key;

    char data[TUPLE_SIZE];

};

struct InternalEntry {

    int key;

    int ptr;

};
struct MetaPageData {

    int root_page_id;
    int total_pages_allocated;

};
struct Page {

    char data[PAGE_SIZE];
    PageHeader* getHeader() { return reinterpret_cast<PageHeader*>(data); }
};
const int LEAF_CAPACITY = (PAGE_SIZE - sizeof(PageHeader)) / sizeof(LeafEntry);

const int INTERNAL_CAPACITY = (PAGE_SIZE - sizeof(PageHeader)) / sizeof(InternalEntry);
#endif
