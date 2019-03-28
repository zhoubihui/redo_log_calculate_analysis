/**
 * @Auther: zhoubihui
 * @Date: 2018/8/29 11:10
 * @Description: 特定用于Oracle解析的公共函数的实现
 */

#include <cctype>
#include <fstream>

#include "../include/base_util.h"
#include "../include/template_method.h"

using std::ifstream;
using std::ios;
using std::vector;
using std::string;
using std::to_string;

namespace extract {
int GetBytesFromFile(ifstream *is, int offset, int len, char *bytes) {
    is->seekg(offset, ios::beg);  /*从流的开始位置跳过offset个字节*/
    is->read(bytes, len);
    return is->gcount();
}

int GetVld(char *bytes) {
    char vld[1];
    ArrayCopy(vld, bytes, 0, 4, 1);
    return DecimalStrToInt(GetByteToHexOrder(vld, GetArrayLen(vld)), 16);
}

bool IsVldInclude4(int *ints, int len, int &value) {
    vector<int> res = Calculate(ints, len, value);
    vector<int>::const_iterator iter;
    for (iter = res.begin(); iter != res.end(); ++iter) {
        if (*iter == 4) {
            return true;
        }
    }
    return false;
}

};  //namespace extract
