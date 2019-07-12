project(testapp)

include_directories (../lib)

aux_source_directory(. TEST_SRC)

add_executable(testapp ${TEST_SRC})

target_link_libraries(testapp cunit datastructure)
