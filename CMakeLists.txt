#---------------------------------------------------------------------------
#CMake minimum required version
cmake_minimum_required (VERSION 2.8)
#---------------------------------------------------------------------------
#project info
project(lsm-db)
#---------------------------------------------------------------------------
#enable test
enable_testing()
#---------------------------------------------------------------------------
#set dubug or release
if(NOT CMAKE_BUILD_TYPE)
    #set(CMAKE_BUILD_TYPE "Release")
    set(CMAKE_BUILD_TYPE "Debug")
endif()
#---------------------------------------------------------------------------
#compile flags
set(CXX_FLAGS
-DCHECK_PTHREAD_RETURN_VALUE
-D_FILE_OFFSET_BITS=64
-Wall
-Wextra
-Werror
-Wconversion
-Wno-unused-parameter
-Wold-style-cast
-Wpointer-arith
-Wshadow
-Wwrite-strings
-Woverloaded-virtual
-march=native
-std=c++11
)
if(CMAKE_BUILD_BITS EQUAL 32)
  list(APPEND CXX_FLAGS "-m32")
endif()
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -D_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -finline-limit=1000 -DNDEBUG")
#---------------------------------------------------------------------------
#install path
set(CMAKE_INSTALL_PREFIX .)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH_LIB ${PROJECT_BINARY_DIR}/lib)
set(LIBRARY_OUTPUT_PATH_INC ${PROJECT_BINARY_DIR}/include)
#---------------------------------------------------------------------------
#output message
string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
message(STATUS "CXX_FLAGS = " ${CMAKE_CXX_FLAGS} " CMAKE_BUILD_TYPE:" ${CMAKE_CXX_FLAGS_}${BUILD_TYPE})
message(STATUS "INSTALL PATH = " ${LIBRARY_OUTPUT_PATH_LIB} " and "${LIBRARY_OUTPUT_PATH_LIB} " and " ${LIBRARY_OUTPUT_PATH_INC})
#---------------------------------------------------------------------------
#add subdirectory
#add_subdirectory(<dir>)
add_subdirectory(src)
add_subdirectory(test)
#---------------------------------------------------------------------------
#add test
set(TEST_lsm_db test_lsm_db)
add_test(test_run ${EXECUTABLE_OUTPUT_PATH}/${TEST_lsm_db})
#---------------------------------------------------------------------------
