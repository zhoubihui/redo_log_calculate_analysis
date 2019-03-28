aux_source_directory(./source DIR_SUB_SRCS)

include_directories($ENV{ORACLE_HOME}/sdk/include)
include_directories(./include)
link_directories($ENV{ORACLE_HOME}/lib)

add_library(extract ${DIR_SUB_SRCS})

target_link_libraries(extract -locci -lclntsh)#链接对应的库
