/**
 * @Auther: zhoubihui
 * @Date: 2018/8/29 11:10
 * @Description: Oracle解析的主类实现
 */

#include <memory>
#include <iostream>  

#include "../include/oracle_extract.h"
#include "../include/oracle_util.h"
#include "../include/base_util.h"
#include "../include/template_method.h"

using std::string;
using std::vector;
using std::make_pair;
using std::map;
using std::pair;

namespace extract {
OracleExtract::OracleExtract(std::ifstream *is) : is_(is) {
}

OracleExtract::~OracleExtract() = default;

int OracleExtract::Extract() {
    char block_size_arr[4];
    char nab_arr[4];
    char rRLen_arr[3];/*实际是4个字节*/

    int current_offset = 512 * 0;
    int record_len = 0;

    int size;   //测试用

    current_offset += 20;
    /*取得块大小*/
    GetBytesFromFile(this->is_, current_offset, 4, block_size_arr);
    int block_size = DecimalStrToInt(GetByteToHexOrder(block_size_arr, 4), 16);

    current_offset = 512 * 1 + 156;
    GetBytesFromFile(this->is_, current_offset, 4, nab_arr);
    int nab = DecimalStrToInt(GetByteToHexOrder(nab_arr, 4), 16);
    int redo_size = block_size * nab;

    current_offset = 512 * 2 + 16;
    char *record_buf = nullptr;
    while (current_offset < redo_size) {
        GetBytesFromFile(this->is_, current_offset, GetArrayLen(rRLen_arr), rRLen_arr);
        record_len = DecimalStrToInt(GetByteToHexOrder(rRLen_arr, GetArrayLen(rRLen_arr)), 16);
        if (record_len == 0) {
            current_offset += 512 - current_offset % 512 + 16;
            continue;
        }
        if (this->RecordIsTrue(current_offset)) {
            record_buf = new char[record_len];
            current_offset = ExtractRecord(record_len, current_offset, record_buf);
            ExtractChange(record_buf, record_len);
            delete [] record_buf;
        } else {
            /*说明是一个虚假的记录*/
            current_offset += 512 - current_offset % 512 + 16;
        }
    }
    return current_offset - 16;
}

bool OracleExtract::RecordIsTrue(int &current_offset) {
    int len = 512 - (current_offset % 512);
    if (len >= 24) {
        return true;
    } else {
        return false;
    }
}

int OracleExtract::ExtractRecord(int &record_len, int current_offset, char *record_buf) {
    int rest_len = 512 - (current_offset % 512);
    if (rest_len > record_len) {
        /*说明记录没有跨块*/
        GetBytesFromFile(this->is_, current_offset, record_len, record_buf);
        current_offset += record_len;
        if (current_offset % 512 == 0) {
            current_offset += 16;
        }
    } else {
        int read_len = 0;
        char *bytes = nullptr;
        while (read_len < record_len) {
            if (rest_len <= (record_len - read_len)) {
                /*还需要往下一个块去读取数据*/
                bytes = new char[rest_len];
                GetBytesFromFile(this->is_, current_offset, rest_len, bytes);
                ArrayCopy(record_buf, bytes, read_len, 0, rest_len);
                delete [] bytes;
                read_len += rest_len;
                current_offset += rest_len;
                current_offset += 16;
            } else {
                rest_len = record_len - read_len;
                bytes = new char[rest_len];
                GetBytesFromFile(this->is_, current_offset, rest_len, bytes);
                ArrayCopy(record_buf, bytes, read_len, 0, rest_len);
                delete [] bytes;
                read_len += rest_len;
                current_offset += rest_len;
            }
            rest_len = 512 - (current_offset % 512);
        }
    }
    return current_offset;
}

void OracleExtract::ExtractChange(char *record_buf, int &record_len) {
    int record_header_len = 0;
    if (record_len == 24) {
        return;
    }
    if (record_len < 68) {
        record_header_len = 24;
    } else {
        int vld = GetVld(record_buf);
        int ints[] = {0, 1, 2, 4, 8, 16, 32};
        if (IsVldInclude4(ints, GetArrayLen(ints), vld)) {
            record_header_len = 68;
        } else {
            record_header_len = 24;
        }
    }
    if ((record_len - record_header_len) == 0) {
        //有一种record，只有24字节
        return;
    }

    int offset = 24 + record_header_len;
    int old_offset = 0;
    char length_list_arr[2];
    char *change_buf;
    char *temp_change_buf = nullptr;
    while (offset < record_len) {
        ArrayCopy(length_list_arr, record_buf, 0, offset, 2);
        int size = 0;
        int length_list = DecimalStrToInt(GetByteToHexOrder(length_list_arr, 2), 16);
        for (int i = 2; i < length_list; i = i + 2) {
            ArrayCopy(length_list_arr, record_buf, 0, (offset + i), 2);
            int len = DecimalStrToInt(GetByteToHexOrder(length_list_arr, 2), 16);
            size += Bq4Byte(len);
        }
        offset += size;
        offset += Bq4Byte(length_list);
        int change_len = offset - old_offset - record_header_len;
        change_buf = new char[change_len];
        ArrayCopy(change_buf, record_buf, 0, old_offset + record_header_len, change_len);
        /*---------------------------------------*/
        /*                                       */
        /*         事务处理                       */
        /*                                       */
        /*---------------------------------------*/
        old_offset += change_len;
        offset += 24;
    }
}

};  //namespace extract
