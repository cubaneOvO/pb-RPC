add_subdirectory(common)
add_subdirectory(coder)
add_subdirectory(net)
add_subdirectory(rpc)


#aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SERVER_LIST)
#list(REMOVE_ITEM SERVER_LIST ${CMAKE_CURRENT_SOURCE_DIR}/client.cpp)

set(CLIENT_LIST client.cpp)

set(SERVER_LIST Server.cpp)

add_executable(server ${SERVER_LIST})
add_executable(client ${CLIENT_LIST})

target_link_libraries(server rpc_server)

target_link_libraries(client rpc_client)

