aux_source_directory(. SRC_LIST)

add_executable(Client ${SRC_LIST})
target_link_libraries(Client pthread)
