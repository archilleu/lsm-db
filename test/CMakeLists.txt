#---------------------------------------------------------------------------
#CMake minimum required version
cmake_minimum_required (VERSION 2.8)
#---------------------------------------------------------------------------
#look up dir's file, and add to var
aux_source_directory(. DIR_TESTS)
#---------------------------------------------------------------------------
#generate static library
#add_library(libname ${<dir>})
#---------------------------------------------------------------------------
#add include path
include_directories(../../thirdpart)
#---------------------------------------------------------------------------
#add lib
find_library(LIBBASE base HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../thirdpart/base/lib)
if(NOT LIBBASE)
    message(FATAL_ERROR "base library not found, please copy from base project")
endif()
#---------------------------------------------------------------------------
#generate test
set(TEST_lsm_db test_lsm_db)
add_executable(${TEST_lsm_db} test_lsm_db.cc)

set(TEST_ss_table_metainfo test_ss_table_metainfo)
add_executable(${TEST_ss_table_metainfo} test_ss_table_metainfo.cc)
#---------------------------------------------------------------------------
#add link lib
set(LIBNAME lsmdb)
set(LIBTOOLS tools)

target_link_libraries(${TEST_lsm_db} ${LIBNAME} ${LIBTOOLS} ${LIBBASE} pthread)

target_link_libraries(${TEST_ss_table_metainfo} ${LIBNAME} ${LIBTOOLS} ${LIBBASE} pthread)
#---------------------------------------------------------------------------
#add test
add_test(${TEST_lsm_db} ${EXECUTABLE_OUTPUT_PATH}/${TEST_lsm_db})

add_test(${TEST_ss_table_metainfo}
    ${EXECUTABLE_OUTPUT_PATH}/${TEST_ss_table_metainfo})
#---------------------------------------------------------------------------
