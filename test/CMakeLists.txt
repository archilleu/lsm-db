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

set(TEST_kv_store test_kv_store)
add_executable(${TEST_kv_store} test_kv_store.cc)

set(TEST_ss_table test_ss_table)
add_executable(${TEST_ss_table} test_ss_table.cc)

set(TEST_command_convert test_command_convert)
add_executable(${TEST_command_convert} test_command_convert.cc)
#---------------------------------------------------------------------------
#add link lib
set(LIBNAME lsmdb)
set(LIBTOOLS tools)

target_link_libraries(${TEST_lsm_db} ${LIBNAME} ${LIBTOOLS} ${LIBBASE} pthread)

target_link_libraries(${TEST_ss_table_metainfo} ${LIBNAME} ${LIBTOOLS} ${LIBBASE} pthread)

target_link_libraries(${TEST_kv_store} ${LIBNAME} ${LIBTOOLS} ${LIBBASE} pthread)

target_link_libraries(${TEST_ss_table} ${LIBNAME} ${LIBTOOLS} ${LIBBASE} pthread)

target_link_libraries(${TEST_command_convert} ${LIBNAME} ${LIBTOOLS} ${LIBBASE} pthread)
#---------------------------------------------------------------------------
#add test
add_test(${TEST_lsm_db} ${EXECUTABLE_OUTPUT_PATH}/${TEST_lsm_db})

add_test(${TEST_ss_table_metainfo}
    ${EXECUTABLE_OUTPUT_PATH}/${TEST_ss_table_metainfo})

add_test(${TEST_kv_store} ${EXECUTABLE_OUTPUT_PATH}/${TEST_kv_store})

add_test(${TEST_ss_table} ${EXECUTABLE_OUTPUT_PATH}/${TEST_ss_table})

add_test(${TEST_ss_table} ${EXECUTABLE_OUTPUT_PATH}/${TEST_command_convert})
#---------------------------------------------------------------------------
