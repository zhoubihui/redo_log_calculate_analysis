#include <iostream>
#include <fstream>
#include <string>

using std::ifstream;
using std::ios;
using std::string;
using std::to_string;

// 每次只读512字节，如果redo log的块大小不等于512，手动更改
int GetBlockFromFile(ifstream *is, int block_number, int block_size, char* bytes){
    is->seekg(block_number * block_size, ios::beg);  /*从流的开始位置跳过offset个字节*/
    is->read(bytes, block_size);
    return is->gcount();
}

void Dump(char* bytes, int block_number, int block_size){
    string str;
    str.append("Block: ").append(to_string(block_number)).append("\n\n");
    for (int i = 0; i < block_size; ++i) {
        int high = (bytes[i] & 0xff) / 16;
        int low = (bytes[i] & 0xff) % 16;
        char h_ascii = (char)((high < 10) ? ('0' + high) : ('a' + high - 10));
        str += h_ascii;
        char l_ascii = (char)((low < 10) ? ('0' + low) : ('a' + low - 10));
        str += l_ascii;
        if((i + 1) % 4 == 0){
            str.append(" ");
        } 
        if((i + 1) % 32 == 0) {
            str.append("\n");
        }
    }
    std::cout << str << std::endl;
}


int main(int argc, char* args[]){
    char* file_name = args[1];
    int block_number = atoi(args[2]);
    int block_count = atoi(args[3]);
    ifstream is;
    is.open(file_name, ios::in | ios::binary);
    char bytes[512];
    for(int i = 0; i < block_count; ++i){
        GetBlockFromFile(&is, block_number + i, 512, bytes);
        Dump(bytes, block_number + i, 512);
    }
    return 0;
}