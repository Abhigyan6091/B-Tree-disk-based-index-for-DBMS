#include "BPlusTree.h"
#include <iostream>
#include <algorithm>

#include <vector>
#include <cstdlib>

BPlusTree::BPlusTree() {

    dm = new DiskManager();
    Page* meta = dm->getPage(0);
    PageHeader* mh = meta->getHeader();


    if (mh->page_type == PAGE_INVALID) {
        initPage(meta, 0, INVALID_PAGE_ID, PAGE_META);
        root_page_id = dm->allocatePage();
        Page* root = dm->getPage(root_page_id);
        initPage(root, root_page_id, INVALID_PAGE_ID, PAGE_LEAF);
        
        MetaPageData* mp = reinterpret_cast<MetaPageData*>(meta->data + sizeof(PageHeader));

        mp->root_page_id = root_page_id;

        mp->total_pages_allocated = 2;

        dm->sync();

    } else {
        MetaPageData* mp = reinterpret_cast<MetaPageData*>(meta->data + sizeof(PageHeader));

        root_page_id = mp->root_page_id;
    }

}


BPlusTree::~BPlusTree() {

    if (dm) delete dm;

}


void BPlusTree::flush() { dm->sync(); }


void BPlusTree::initPage(Page* p, int id, int parent, int type) {
    std::memset(p->data, 0, PAGE_SIZE);
    PageHeader* h = p->getHeader();

    h->page_id = id;
    h->parent_id = parent;

    h->page_type = type;

    h->num_items = 0;

    h->next_leaf = INVALID_PAGE_ID;
    h->extra_ptr = INVALID_PAGE_ID;

}

char* BPlusTree::find(int key) {

    int leaf_id = findLeaf(key);

    Page* leaf = dm->getPage(leaf_id);

    if (!leaf) return nullptr;


    PageHeader* h = leaf->getHeader();
    LeafEntry* entries = reinterpret_cast<LeafEntry*>(leaf->data + sizeof(PageHeader));


    int l = 0, r = h->num_items - 1;
    while (l <= r) {

        int mid = l + (r - l) / 2;
        if (entries[mid].key == key) {
            char* result = (char*)malloc(TUPLE_SIZE);

            std::memcpy(result, entries[mid].data, TUPLE_SIZE);

            return result;

        }
        if (entries[mid].key < key) l = mid + 1;

        else r = mid - 1;
    }
    return nullptr;

}



int BPlusTree::findLeaf(int key) {

    int curr = root_page_id;
    while(true) {
        Page* p = dm->getPage(curr);

        PageHeader* h = p->getHeader();

        if (h->page_type == PAGE_LEAF) return curr;

        InternalEntry* entries = reinterpret_cast<InternalEntry*>(p->data + sizeof(PageHeader));
        
        int bl = 0, br = h->num_items - 1;
        int idx = -1;
        int child = h->extra_ptr;

        while(bl <= br) {

            int mid = bl + (br - bl) / 2;
            if (entries[mid].key <= key) {

                idx = mid;

                bl = mid + 1;
            } else {
                br = mid - 1;

            }
        }



        if (idx != -1) child = entries[idx].ptr;
        curr = child;
    }
}


bool BPlusTree::insert(int key, const char* val) {

    int leaf_id = findLeaf(key);
    Page* leaf = dm->getPage(leaf_id);

    PageHeader* h = leaf->getHeader();

    LeafEntry* entries = reinterpret_cast<LeafEntry*>(leaf->data + sizeof(PageHeader));


    for(int i=0; i<h->num_items; i++) {
        if (entries[i].key == key) return false;
    }



    if (h->num_items < LEAF_CAPACITY) {

        int idx = 0;
        while(idx < h->num_items && entries[idx].key < key) idx++;

        
        if (idx < h->num_items) {

            std::memmove(&entries[idx+1], &entries[idx], (h->num_items - idx) * sizeof(LeafEntry));
        }

        entries[idx].key = key;
        std::memcpy(entries[idx].data, val, TUPLE_SIZE);

        h->num_items++;
        return true;

    }



    insertSplitLeaf(leaf_id, leaf, key, val);
    return true;

}


void BPlusTree::insertSplitLeaf(int old_id, Page* old_leaf, int key, const char* val) {
    PageHeader* old_h = old_leaf->getHeader();
    LeafEntry* old_entries = reinterpret_cast<LeafEntry*>(old_leaf->data + sizeof(PageHeader));

    std::vector<LeafEntry> buffer(LEAF_CAPACITY + 1);
    std::memcpy(buffer.data(), old_entries, old_h->num_items * sizeof(LeafEntry));



    int idx = 0;

    while(idx < old_h->num_items && buffer[idx].key < key) idx++;

    
    for(int i=old_h->num_items; i>idx; i--) buffer[i] = buffer[i-1];
    buffer[idx].key = key;
    std::memcpy(buffer[idx].data, val, TUPLE_SIZE);

    int new_id = dm->allocatePage();

    Page* new_leaf = dm->getPage(new_id);
    initPage(new_leaf, new_id, old_h->parent_id, PAGE_LEAF);

    PageHeader* new_h = new_leaf->getHeader();

    LeafEntry* new_entries = reinterpret_cast<LeafEntry*>(new_leaf->data + sizeof(PageHeader));

    int total = old_h->num_items + 1;

    int mid = total / 2;

    old_h->num_items = mid;
    std::memcpy(old_entries, buffer.data(), mid * sizeof(LeafEntry));


    int new_count = total - mid;
    new_h->num_items = new_count;
    std::memcpy(new_entries, &buffer[mid], new_count * sizeof(LeafEntry));

    new_h->next_leaf = old_h->next_leaf;

    old_h->next_leaf = new_id;

    insertIntoParent(old_id, new_entries[0].key, new_id);

}

void BPlusTree::insertIntoParent(int left_id, int key, int right_id) {

    Page* left = dm->getPage(left_id);

    int parent_id = left->getHeader()->parent_id;

    if (parent_id == INVALID_PAGE_ID) {
        int new_root_id = dm->allocatePage();

        Page* root = dm->getPage(new_root_id);

        initPage(root, new_root_id, INVALID_PAGE_ID, PAGE_INTERNAL);

        
        PageHeader* rh = root->getHeader();
        InternalEntry* re = reinterpret_cast<InternalEntry*>(root->data + sizeof(PageHeader));


        rh->extra_ptr = left_id;
        re[0].key = key;
        re[0].ptr = right_id;

        rh->num_items = 1;


        left->getHeader()->parent_id = new_root_id;

        dm->getPage(right_id)->getHeader()->parent_id = new_root_id;
        updateRoot(new_root_id);

        return;

    }


    Page* parent = dm->getPage(parent_id);
    PageHeader* ph = parent->getHeader();


    if (ph->num_items < INTERNAL_CAPACITY) {
        InternalEntry* pe = reinterpret_cast<InternalEntry*>(parent->data + sizeof(PageHeader));

        int idx = 0;
        while(idx < ph->num_items && pe[idx].key < key) idx++;



        if (idx < ph->num_items) {
            std::memmove(&pe[idx+1], &pe[idx], (ph->num_items - idx) * sizeof(InternalEntry));
        }

        pe[idx].key = key;
        pe[idx].ptr = right_id;
        ph->num_items++;
    } else {

        insertSplitInternal(parent_id, parent, key, right_id);
    }

}

void BPlusTree::insertSplitInternal(int old_id, Page* old_node, int key, int right_id) {
    PageHeader* old_h = old_node->getHeader();

    InternalEntry* old_entries = reinterpret_cast<InternalEntry*>(old_node->data + sizeof(PageHeader));

    std::vector<InternalEntry> buffer(INTERNAL_CAPACITY + 1);

    std::memcpy(buffer.data(), old_entries, old_h->num_items * sizeof(InternalEntry));



    int idx = 0;
    while(idx < old_h->num_items && buffer[idx].key < key) idx++;
    
    for(int i=old_h->num_items; i>idx; i--) buffer[i] = buffer[i-1];
    buffer[idx].key = key;

    buffer[idx].ptr = right_id;


    int new_id = dm->allocatePage();

    Page* new_node = dm->getPage(new_id);
    initPage(new_node, new_id, old_h->parent_id, PAGE_INTERNAL);

    PageHeader* new_h = new_node->getHeader();
    InternalEntry* new_entries = reinterpret_cast<InternalEntry*>(new_node->data + sizeof(PageHeader));


    int total = old_h->num_items + 1;
    int mid = total / 2;

    int up_key = buffer[mid].key;


    old_h->num_items = mid;
    std::memcpy(old_entries, buffer.data(), mid * sizeof(InternalEntry));


    new_h->extra_ptr = buffer[mid].ptr;


    int new_count = total - (mid + 1);
    new_h->num_items = new_count;

    if (new_count > 0) {
        std::memcpy(new_entries, &buffer[mid + 1], new_count * sizeof(InternalEntry));
    }



    Page* childP0 = dm->getPage(new_h->extra_ptr);
    if(childP0) childP0->getHeader()->parent_id = new_id;

    for(int i=0; i<new_count; i++) {

        Page* child = dm->getPage(new_entries[i].ptr);
        if(child) child->getHeader()->parent_id = new_id;
    }

    insertIntoParent(old_id, up_key, new_id);

}


void BPlusTree::updateRoot(int new_root) {
    root_page_id = new_root;
    Page* meta = dm->getPage(0);

    MetaPageData* mp = reinterpret_cast<MetaPageData*>(meta->data + sizeof(PageHeader));

    mp->root_page_id = root_page_id;
}


bool BPlusTree::remove(int key) {
    int leaf_id = findLeaf(key);

    Page* leaf = dm->getPage(leaf_id);

    PageHeader* h = leaf->getHeader();
    LeafEntry* entries = reinterpret_cast<LeafEntry*>(leaf->data + sizeof(PageHeader));


    int idx = -1;

    for(int i=0; i<h->num_items; i++) {
        if(entries[i].key == key) { idx = i; break; }
    }

    if (idx == -1) return false;

    if (idx < h->num_items - 1) {

        std::memmove(&entries[idx], &entries[idx+1], (h->num_items - idx - 1) * sizeof(LeafEntry));
    }
    h->num_items--;
    return true;
}


char** BPlusTree::range(int start, int end, int& count) {
    std::vector<char*> res;

    int leaf_id = findLeaf(start);
    int visited = 0;


    while(leaf_id != INVALID_PAGE_ID && visited < 50000) {

        Page* leaf = dm->getPage(leaf_id);

        PageHeader* h = leaf->getHeader();

        LeafEntry* entries = reinterpret_cast<LeafEntry*>(leaf->data + sizeof(PageHeader));

        for(int i=0; i<h->num_items; i++) {

            if (entries[i].key >= start) {

                if (entries[i].key > end) goto done;

                char* buf = (char*)malloc(TUPLE_SIZE);

                std::memcpy(buf, entries[i].data, TUPLE_SIZE);

                res.push_back(buf);

            }
        }

        leaf_id = h->next_leaf;
        visited++;
    }



done:

    count = res.size();
    if (count == 0) return nullptr;

    char** ret = (char**)malloc(count * sizeof(char*));
    std::copy(res.begin(), res.end(), ret);

    return ret;
}
