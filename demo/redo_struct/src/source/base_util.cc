/**
 * @Auther: zhoubihui
 * @Date: 2018/8/29 11:10
 * @Description: 基本公共函数的实现
 */

#include <math.h>
#include "../include/base_util.h"
using std::string;

namespace extract {
string GetByteToHex(const char *bytes, int len) {
    string str;
    for (int i = 0; i < len; ++i) {
        int high = (bytes[i] & 0xff) / 16;
        int low = (bytes[i] & 0xff) % 16;
        char h_ascii = (char)((high < 10) ? ('0' + high) : ('a' + high - 10));
        str += h_ascii;
        char l_ascii = (char)((low < 10) ? ('0' + low) : ('a' + low - 10));
        str += l_ascii;
    }
    return str;
}

string GetByteToHexOrder(const char *bytes, int len) {
    string str;
    for (int i = (len - 1); i >= 0; --i) {
        int high = (bytes[i] & 0xff) / 16;
        int low = (bytes[i] & 0xff) % 16;
        char h_ascii = (char)((high < 10) ? ('0' + high) : ('a' + high - 10));
        str += h_ascii;
        char l_ascii = (char)((low < 10) ? ('0' + low) : ('a' + low - 10));
        str += l_ascii;
    }
    return str;
}

int DecimalStrToInt(std::string str, int base) {
    //
    //TODO data_obj的类型还需要判断，目前暂定是int类型
    //
    return (int) strtol(str.c_str(), nullptr, base);
}

long int DecimalStrToLongInt(std::string str, int base) {
    return (long int) strtol(str.c_str(), nullptr, base);
}


int Bq4Byte(int len) {
    int bq_len = len;
    if (bq_len % 4 != 0) {
        bq_len += 4 - (len % 4);
    }
    return bq_len;
}

};  //namespace extract
