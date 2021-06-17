#---------------------------------------------------------------------------
#CMake minimum required version
cmake_minimum_required (VERSION 2.8)
#---------------------------------------------------------------------------
#look up dir's file, and add to var
#aux_source_directory(<dir> <variable>)
aux_source_directory(. DIR_SRCS)
add_subdirectory(command)
add_subdirectory(tools)
#---------------------------------------------------------------------------
#add header file
include_directories(../thirdpart)
#---------------------------------------------------------------------------
#generate static library
set(LIBNAME lsmdb)
add_library(${LIBNAME} ${DIR_SRCS})
#---------------------------------------------------------------------------
#install target 
install(TARGETS ${LIBNAME} DESTINATION ${LIBRARY_OUTPUT_PATH_LIB})

set(HEADERS
)
install(FILES ${HEADERS} DESTINATION ${LIBRARY_OUTPUT_PATH_INC})
#---------------------------------------------------------------------------
