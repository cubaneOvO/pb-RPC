add_custom_target(gen_pb_data
    ALL
    COMMAND mkdir -p ${PROJECT_SOURCE_DIR}/src/proto
    COMMAND protoc --proto_path=${PROJECT_SOURCE_DIR}/proto --cpp_out=${PROJECT_SOURCE_DIR}/src/proto order.proto
    COMMENT "Generating pb data"
)