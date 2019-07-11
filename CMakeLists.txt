cmake_minimum_required (VERSION 3.10)

project (started_in_redis)

include_directories(./ ./src ./lib)

add_subdirectory(src)
add_subdirectory(lib)
add_subdirectory(test)
