cmake_minimum_required(VERSION 3.12)
project(OracleAnalysis)

set(CMAKE_CXX_STANDARD 14)
add_definitions(-std=c++11)
#dubug 模式
#注意 需要在CMakeLists.txt 里加 
#set (CMAKE_CXX_FLAGS “${CMAKE_CXX_FLAGS} -g”) 
#开启debug 不然断点调试是无效的 
#如需去除dubug 只要去掉此行了
set (CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g")

aux_source_directory(. DIR_SRCS)
add_subdirectory(src) #这个目录下如果没有CMakeLists.txt文件就会报错

include_directories($ENV{ORACLE_HOME}/sdk/include)
link_directories($ENV{ORACLE_HOME}/lib)

add_executable(OracleAnalysis ${DIR_SRCS})

target_link_libraries(OracleAnalysis extract)#链接对应的库