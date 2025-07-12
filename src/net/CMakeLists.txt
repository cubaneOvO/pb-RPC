aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SERVER_LIST)

list(REMOVE_ITEM SERVER_LIST ${CMAKE_CURRENT_SOURCE_DIR}/TcpClient.cpp)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} CLIENT_LIST)

list(REMOVE_ITEM CLIENT_LIST ${CMAKE_CURRENT_SOURCE_DIR}/TcpServer.cpp)

add_library(rpc_server ${SERVER_LIST})

add_library(rpc_client ${CLIENT_LIST})

target_link_libraries(rpc_server PUBLIC common PUBLIC coder PUBLIC rpctool)

target_link_libraries(rpc_client PUBLIC common PUBLIC coder PUBLIC rpctool)

