project(libdatastructure)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../so/)

aux_source_directory(. DIR_LIB_DATA_STRUCTURE)

add_library(datastructure SHARED ${DIR_LIB_DATA_STRUCTURE})
