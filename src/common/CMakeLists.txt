find_package(Protobuf REQUIRED)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} COMMON_LIST)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../proto PROTO)

add_library(common ${COMMON_LIST} ${PROTO})

target_link_libraries(common PUBLIC spdlog::spdlog)

target_link_libraries(common PUBLIC yaml-cpp::yaml-cpp)

target_link_libraries(common PUBLIC protobuf::libprotobuf)

target_link_libraries(common PUBLIC etcd-cpp-api)