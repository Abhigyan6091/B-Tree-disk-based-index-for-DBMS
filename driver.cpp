#include <iostream>

#include <fstream>
#include <sstream>
#include <string>
#include <cstring>

#include <chrono>
#include <iomanip>
using namespace std;
using namespace chrono;

#define DATA_SIZE 100
extern "C" {
    int writeData(int key, unsigned char* data);
    int deleteData(int key);

    unsigned char* readData(int key);

    unsigned char** readRangeData(int lowerKey, int upperKey, int* n);

    void closeIndex();
}
struct Stats {
    int insert_count = 0;
    int insert_success = 0;

    int insert_fail = 0;

    double insert_time = 0.0;
    

    int read_count = 0;

    int read_found = 0;
    int read_notfound = 0;
    double read_time = 0.0;

    
    int delete_count = 0;

    int delete_success = 0;

    int delete_fail = 0;

    double delete_time = 0.0;
    

    int range_count = 0;

    double range_time = 0.0;
    int range_results = 0;
};
void padData(const string& str, unsigned char* data) {

    memset(data, 0, DATA_SIZE);
    int copyLen = min((int)str.length(), DATA_SIZE);
    memcpy(data, str.c_str(), copyLen);
}
string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");

    if (first == string::npos) return "";

    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));

}
int main(int argc, char* argv[]) {
    string inputFile = "input.txt";

    
    
    if (argc > 1) {

        inputFile = argv[1];

    }

    

    cout << "========================================" << endl;

    cout << "B+ Tree Performance Testing" << endl;
    cout << "========================================" << endl;
    cout << "Input file: " << inputFile << endl;

    cout << endl;

    
    ifstream infile(inputFile);

    if (!infile.is_open()) {

        cerr << "ERROR: Could not open input file: " << inputFile << endl;
        cerr << "Make sure you've generated the input file first using:" << endl;
        cerr << "  python3 input_generator.py" << endl;

        return 1;
    }
    

    Stats stats;
    int totalOperations = 0;

    string line;

    
    auto overallStart = high_resolution_clock::now();
    
    cout << "Starting test execution..." << endl;
    cout << "Progress: " << flush;
    

    while (getline(infile, line)) {
        line = trim(line);

        
        

        if (line.empty() || line[0] == '#') {
            continue;

        }

        

        istringstream iss(line);
        string operation;
        iss >> operation;
        
        
        for (char& c : operation) {
            c = toupper(c);

        }

        
        if (operation == "INSERT") {
            int key;
            string dataStr;

            

            if (!(iss >> key)) {
                cerr << "\nERROR: Invalid INSERT format: " << line << endl;
                continue;
            }

            
            

            getline(iss, dataStr);
            dataStr = trim(dataStr);

            

            unsigned char data[DATA_SIZE];

            padData(dataStr, data);
            

            auto start = high_resolution_clock::now();
            int result = writeData(key, data);

            auto end = high_resolution_clock::now();
            

            duration<double> elapsed = end - start;

            stats.insert_time += elapsed.count();
            stats.insert_count++;

            

            if (result == 1) {

                stats.insert_success++;
            } else {
                stats.insert_fail++;
                cout << "\n[FAIL] INSERT key=" << key << endl;

                cout << "Progress: " << flush;
            }

        }
        else if (operation == "READ") {

            int key;
            
            if (!(iss >> key)) {

                cerr << "\nERROR: Invalid READ format: " << line << endl;
                continue;
            }
            
            auto start = high_resolution_clock::now();
            unsigned char* result = readData(key);
            auto end = high_resolution_clock::now();

            
            duration<double> elapsed = end - start;

            stats.read_time += elapsed.count();
            stats.read_count++;
            

            if (result != nullptr) {
                stats.read_found++;
                
                delete[] result;
            } else {
                stats.read_notfound++;
            }

        }

        else if (operation == "DELETE") {

            int key;

            
            if (!(iss >> key)) {
                cerr << "\nERROR: Invalid DELETE format: " << line << endl;
                continue;
            }
            
            auto start = high_resolution_clock::now();
            int result = deleteData(key);

            auto end = high_resolution_clock::now();

            

            duration<double> elapsed = end - start;
            stats.delete_time += elapsed.count();
            stats.delete_count++;

            
            if (result == 1) {
                stats.delete_success++;

            } else {
                stats.delete_fail++;

            }

        }

        else if (operation == "RANGE") {

            int lowerKey, upperKey;
            

            if (!(iss >> lowerKey >> upperKey)) {
                cerr << "\nERROR: Invalid RANGE format: " << line << endl;
                continue;

            }
            

            int n = 0;

            auto start = high_resolution_clock::now();
            unsigned char** result = readRangeData(lowerKey, upperKey, &n);

            auto end = high_resolution_clock::now();

            

            duration<double> elapsed = end - start;

            stats.range_time += elapsed.count();

            stats.range_count++;
            stats.range_results += n;

            
            
            if (result != nullptr) {
                for (int i = 0; i < n; i++) {

                    delete[] result[i];
                }
                delete[] result;
            }
        }

        else {
            cerr << "\nWARNING: Unknown operation: " << operation << endl;
            cout << "Progress: " << flush;
            continue;
        }
        
        totalOperations++;

        
        
        if (totalOperations % 100 == 0) {

            cout << "." << flush;

        }

        
        
        if (totalOperations % 1000 == 0) {

            cout << " " << totalOperations << " " << flush;

        }
    }
    

    auto overallEnd = high_resolution_clock::now();
    duration<double> totalElapsed = overallEnd - overallStart;
    
    infile.close();
    
    cout << endl << endl;
    cout << "========================================" << endl;
    cout << "TEST COMPLETED" << endl;
    cout << "========================================" << endl;
    cout << endl;

    

    cout << "OVERALL STATISTICS:" << endl;

    cout << "-------------------" << endl;

    cout << "Total Operations:    " << totalOperations << endl;
    cout << "Total Time:          " << fixed << setprecision(3) 
         << totalElapsed.count() << " seconds" << endl;
    cout << "Avg Time/Operation:  " << fixed << setprecision(6) 
         << (totalElapsed.count() / totalOperations) << " seconds" << endl;
    cout << endl;

    

    if (stats.insert_count > 0) {
        cout << "INSERT OPERATIONS:" << endl;

        cout << "  Total:      " << stats.insert_count << endl;
        cout << "  Successful: " << stats.insert_success << endl;
        cout << "  Failed:     " << stats.insert_fail << endl;

        cout << "  Total Time: " << fixed << setprecision(3) 

             << stats.insert_time << " seconds" << endl;

        cout << "  Avg Time:   " << fixed << setprecision(6) 
             << (stats.insert_time / stats.insert_count) << " seconds" << endl;
        cout << endl;
    }

    
    if (stats.read_count > 0) {

        cout << "READ OPERATIONS:" << endl;

        cout << "  Total:      " << stats.read_count << endl;
        cout << "  Found:      " << stats.read_found << endl;
        cout << "  Not Found:  " << stats.read_notfound << endl;
        cout << "  Total Time: " << fixed << setprecision(3) 
             << stats.read_time << " seconds" << endl;
        cout << "  Avg Time:   " << fixed << setprecision(6) 
             << (stats.read_time / stats.read_count) << " seconds" << endl;

        cout << endl;
    }
    
    if (stats.delete_count > 0) {
        cout << "DELETE OPERATIONS:" << endl;
        cout << "  Total:      " << stats.delete_count << endl;
        cout << "  Successful: " << stats.delete_success << endl;

        cout << "  Failed:     " << stats.delete_fail << endl;

        cout << "  Total Time: " << fixed << setprecision(3) 

             << stats.delete_time << " seconds" << endl;
        cout << "  Avg Time:   " << fixed << setprecision(6) 

             << (stats.delete_time / stats.delete_count) << " seconds" << endl;
        cout << endl;

    }
    
    if (stats.range_count > 0) {
        cout << "RANGE OPERATIONS:" << endl;

        cout << "  Total:         " << stats.range_count << endl;
        cout << "  Total Results: " << stats.range_results << endl;
        cout << "  Avg Results:   " << (stats.range_results / stats.range_count) << endl;

        cout << "  Total Time:    " << fixed << setprecision(3) 

             << stats.range_time << " seconds" << endl;
        cout << "  Avg Time:      " << fixed << setprecision(6) 

             << (stats.range_time / stats.range_count) << " seconds" << endl;
        cout << endl;
    }

    
    cout << "========================================" << endl;
    
    
    closeIndex();
    

    return 0;
}
