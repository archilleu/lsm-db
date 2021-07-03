#---------------------------------------------------------------------------
#CMake minimum required version
cmake_minimum_required (VERSION 2.8)
#---------------------------------------------------------------------------
#look up dir's file, and add to var
#aux_source_directory(<dir> <variable>)
aux_source_directory(. DIR_SRCS_COMMAND)
#---------------------------------------------------------------------------
#add include path
#include_directories(../../thirdpart)
#---------------------------------------------------------------------------
#generate static library
#add_library(libname ${<dir>})
#set(LIBCOMMAND command)
#add_library(${LIBCOMMAND} ${DIR_SRCS_COMMAND})
#---------------------------------------------------------------------------
