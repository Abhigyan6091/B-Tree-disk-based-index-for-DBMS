#ifndef C_API_H

#define C_API_H
#ifdef __cplusplus
extern "C" {
#endif

    void init();
    int writeData(int key, unsigned char* data);
    unsigned char* readData(int key);

    int deleteData(int key);
    unsigned char** readRangeData(int lowerKey, int upperKey, int* n);
    void closeIndex();

#ifdef __cplusplus
}
#endif

#endif
