/**
 * @Auther: zhoubihui
 * @Date: 2018/8/29 11:10
 * @Description: Oracle解析的主类声明
 */

#ifndef ORACLE_ANALYSIS_ORACLEEXTRACT_H
#define ORACLE_ANALYSIS_ORACLEEXTRACT_H

#include <fstream>
#include <map>
#include <memory>

namespace extract {
class OracleExtract {
public:
    explicit OracleExtract(std::ifstream *is);

    virtual ~OracleExtract();

    /**
     * Oracle解析的主函数
     * @return
    */
    int Extract();

private:

    /**
     * 根据下标判断当前Record是否是真实的
     * @param current_offset !< in:
     * @return
    */
    bool RecordIsTrue(int &current_offset);


    /**
     * 负责从日志文件中读取正确的record
     * @param record_len !< in: 当前record的长度
     * @param current_offset !< in:
     * @param record_buf !< out:
     * @return 返回current_offset的值
    */
    int ExtractRecord(int &record_len, int current_offset, char *record_buf);


    /**
     * 计算Record的头部已经每个change的大小
     * @param record_buf !< in:
     * @param record_len !< in:
    */
    void ExtractChange(char *record_buf, int &record_len);

    std::ifstream *is_;
};

}; //namespace extract
#endif //ORACLE_ANALYSIS_ORACLEEXTRACT_H
