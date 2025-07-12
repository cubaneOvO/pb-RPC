aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} CODER_LIST)

add_library(coder ${CODER_LIST})

target_link_libraries(coder PUBLIC common)