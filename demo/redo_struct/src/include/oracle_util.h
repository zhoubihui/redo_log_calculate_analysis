
/**
 * @Auther: zhoubihui
 * @Date: 2018/8/29 11:10
 * @Description: 特定用于Oracle解析的公共函数声明
 */

#ifndef ORACLE_ANALYSIS_ORACLEUTIL_H
#define ORACLE_ANALYSIS_ORACLEUTIL_H

#include <fstream>
#include <string>

namespace extract {

/**
* 读取一定长度的数据
* @param is !< in: 文件输入流
* @param offset !< in: 位置
* @param len 长度 !< in: 长度
* @param bytes !< out: 保存本次读取的数据
* @return 返回本次实际读取的字节数
*/
extern int GetBytesFromFile(std::ifstream *is, int offset, int len, char *bytes);

/**
* 从record中取得VLD参数
* @param bytes !< in:
* @return
*/
extern int GetVld(char *bytes);

/**
 * 判断value的叠加值中是否包含4，包含则返回true
 * @param ints !< in:
 * @param len !< in: ints的长度
 * @param value !< in:
 * @return
*/
extern bool IsVldInclude4(int *ints, int len, int &value);

};  //namespace extract

#endif //ORACLE_ANALYSIS_ORACLEUTIL_H

