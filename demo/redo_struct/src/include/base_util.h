/**
 * @Auther: zhoubihui
 * @Date: 2018/8/29 11:10
 * @Description: 基本公共函数的声明
 */

#ifndef ORACLE_ANALYSIS_BASEUTIL_H
#define ORACLE_ANALYSIS_BASEUTIL_H

#include <cstring>
#include <string>

namespace extract {

/**
 * 将字节数组转成十六进制的字符串表示
 * @param bytes !< in:数组
 * @param len !<in:数组长度
 * @return
 */
extern std::string GetByteToHex(const char *bytes, int len);


/**
 * 将字节数组的元素调换顺序后,转成十六进制的字符串表示
 * @param bytes !< in:
 * @param len !< in:
 * @return
 */
extern std::string GetByteToHexOrder(const char *bytes, int len);

/**
 * 将十六进制的字符串转成int类型
 * @param str !< in:十六进制字符串
 * @param base !< in:进制
 * @return 返回int类型
 */
extern int DecimalStrToInt(std::string str, int base);

/**
* 将十六进制的字符串转成long int类型
* @param str !< in:十六进制字符串
* @param base !< in:进制
* @return 以long int类型返回
*/
extern long int DecimalStrToLongInt(std::string str, int base);


/**
* 将len做4 字节补齐
* @param len !< in:需要补齐的长度
* @return
*/
extern int Bq4Byte(int len);


/**
 * 数组拷贝
 * @param dest !< out: 目标数组的地址
 * @param src !< in: 源数组
 * @param dest_start !< in: 目标数组的开始复制位置
 * @param src_start !< in: 从源数组的哪个位置开始复制
 * @param n 本次复制的字节数
*/
inline void ArrayCopy(
        char *dest,
        const char *src,
        const int dest_start,
        const int src_start,
        const int n) {
    memcpy(dest + dest_start, src + src_start, n);
}

}; //namespace extract
#endif //ORACLE_ANALYSIS_BASEUTIL_H
