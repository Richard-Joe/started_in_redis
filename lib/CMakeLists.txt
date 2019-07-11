cmake_minimum_required(VERSION 3.10)

project(libdatastructure)

aux_source_directory(. DIR_LIB_DATA_STRUCTURE)

add_library(datastructure SHARED ${DIR_LIB_DATA_STRUCTURE})
