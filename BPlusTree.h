#ifndef B_PLUS_TREE_H

#define B_PLUS_TREE_H

#include "DiskManager.h"

#include "common.h"
#include <vector>

class BPlusTree {
    DiskManager* dm;
    int root_page_id;
    
    void initPage(Page* p, int id, int parent, int type);
    void updateRoot(int new_root);
    int findLeaf(int key);

    void insertSplitLeaf(int old_id, Page* old_leaf, int key, const char* val);
    void insertIntoParent(int left_id, int key, int right_id);
    void insertSplitInternal(int old_id, Page* old_node, int key, int right_id);
public:

    BPlusTree();

    ~BPlusTree();
    void flush();
    
    char* find(int key);
    bool insert(int key, const char* val);

    bool remove(int key);

    char** range(int start, int end, int& count);
};
#endif
