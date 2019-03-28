#include <iostream>
#include <fstream>
#include <thread>
#include "src/include/oracle_extract.h"

using namespace std;
using namespace extract;

int main() {
    ifstream *is = new ifstream;
    is->open("/Users/zhoubihui/redo/redo01.log", ios::in | ios::binary);
    OracleExtract extract(is);
    int current_offset = extract.Extract();
    is->close();
    delete is;
    cout << current_offset << endl;

    return 0;
}