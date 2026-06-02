#
# AUX 示例程序
#

# examples 使用 "aux_msg/aux_msg.h" 路径，需要项目根目录的父目录在 include path 中
set(EXAMPLES_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/..")

# 发布/订阅示例
add_executable(example_pubsub pubsub.c)
target_include_directories(example_pubsub PRIVATE ${EXAMPLES_INCLUDE_DIR})
target_link_libraries(example_pubsub PRIVATE aux_msg)

# 请求/回复示例
add_executable(example_reqrep reqrep.c)
target_include_directories(example_reqrep PRIVATE ${EXAMPLES_INCLUDE_DIR})
target_link_libraries(example_reqrep PRIVATE aux_msg)

# 异步示例
add_executable(example_async async.c)
target_include_directories(example_async PRIVATE ${EXAMPLES_INCLUDE_DIR})
target_link_libraries(example_async PRIVATE aux_msg)
