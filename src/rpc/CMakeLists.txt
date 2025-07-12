aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} RPC_LIST)

add_library(rpctool ${RPC_LIST})

target_link_libraries(rpctool PUBLIC common PUBLIC coder PUBLIC rpc_server PUBLIC rpc_client)

