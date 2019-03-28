/**
 * @Auther: zhoubihui
 * @Date: 2018/8/30 17:53
 * @Description: 专门用于存放函数模板和类模板
 */

#ifndef ORACLEANALYSIS_TEMPLATE_H
#define ORACLEANALYSIS_TEMPLATE_H

#include <vector>
#include <sstream>
#include <string>
#include <memory>

namespace extract {
/**
* 取得数组长度<br>
* 待改进:字符数组末尾多一个字符
* @tparam T
* @param arr !< in: 必须是引用类型才行
* @return
*/
template <typename T>
int GetArrayLen(T& arr) {
    return (sizeof(arr) / sizeof(arr[0]));
}


/**
 * 利用穷举递归法计算由哪几个元素叠加
 * @tparam T
 * @param element_index !< out: 符合条件的元素下标
 * @param vec !< in: 全部元素的集合
 * @param current_sum !< out: 当前的元素和
 * @param current_index !< in: 当前处理的元素的下标
 * @param value !< in:
*/
template <typename T>
void Recursively(std::vector<int> &element_index,std::vector<T> &vec, T current_sum, int current_index, T &value) {
    if(current_index == 0) {
        return;
    }
    T next_element = vec[current_index - 1];  /*下一个元素的值*/
    if(current_sum + next_element > value) {
        Recursively(element_index, vec, current_sum, current_index - 1, value);
    } else if(current_sum + next_element < value) {
        element_index.push_back(current_index - 1);
        current_sum += next_element;
        Recursively(element_index, vec, current_sum, current_index - 1, value);
    } else {
        element_index.push_back(current_index - 1);
        return;
    }
}


/**
 * 计算value的值由arr数组中哪些元素叠加
 * @tparam T
 * @param arr !< in:
 * @param len !< in:
 * @param value !< in:
 * @return
 */
template <typename T>
std::vector<T> SearchElement(T *arr, int &len, T &value) {
    std::vector<T> vec;
    vec.reserve(len);
    std::vector<int> element_index;
    element_index.reserve(3);
    bool findone = false;
    for (int i = 0; i < len ; ++i) {
        if(arr[i] < value) {
            vec.push_back(arr[i]); /*排除大于value的元素*/
        }
    }

    bool flag = true;
    do {
        /*如果最大值加最小值大于sum，则去掉最大值*/
        T min = vec[0];
        T max = vec[vec.size() - 1];
        if(max + min > value) {
            vec.erase(vec.end() - 1);
        } else if(min + max == value) {
            findone = true;
            element_index.push_back((int)vec.size() - 1);
            element_index.push_back(0);
            break;
        } else {
            flag = false;
        }
    } while(flag);

    if(findone) {

    } else {
        for (int i = 0; i < vec.size(); ++i) {
            element_index.clear();
            element_index.push_back((int)vec.size() - 1 - i);
            T current_sum = vec[vec.size() - 1 - i];
            Recursively(element_index, vec, current_sum, ((int)vec.size() - 1 - i), value);
            if(element_index.size() > 1) {
                break;
            }

        }
    }

    std::vector<T> res;
    res.reserve(3);  //预先分配空间
    for(int i = 0; i < element_index.size(); i++) {
        res.push_back(vec[element_index[i]]);
    }
    return res;
}


/**
* 计算value的值由arr数组中哪些元素叠加
* @tparam T
* @param arr !< in:
* @param len !< in: arr的长度
* @param value !< in:
* @return
*/
template <typename T>
std::vector<T> Calculate(T *arr, int &len, T &value) {
    int index = 0;
    for (; index < len; index++) {
        if(arr[index] == value) {
            break;
        }
    }
    if(index != len) {
        std::vector<T> res;
        res.reserve(1);  //预定义大小
        res.push_back(arr[index]);
        return res;
    } else {
        return SearchElement(arr, len, value);
    }
}

};  //namespace extract
#endif //ORACLEANALYSIS_TEMPLATE_H
