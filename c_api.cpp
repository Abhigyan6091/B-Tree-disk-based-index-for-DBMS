#include "c_api.h"
#include "BPlusTree.h"


BPlusTree* tree = nullptr;


extern "C" {
    void init() { 
        if (!tree) tree = new BPlusTree(); 
    }

    
    int writeData(int key, unsigned char* data) {

        init();
        return tree->insert(key, (const char*)data) ? 1 : 0;

    }

    unsigned char* readData(int key) {
        init();
        return (unsigned char*)tree->find(key);
    }

    int deleteData(int key) {
        init();
        return tree->remove(key) ? 1 : 0;
    }

    unsigned char** readRangeData(int lowerKey, int upperKey, int* n) {

        init();
        return (unsigned char**)tree->range(lowerKey, upperKey, *n);
    }

    void closeIndex() {
        if (tree) { 
            tree->flush(); 
            delete tree; 

            tree = nullptr; 
        }
    }

}