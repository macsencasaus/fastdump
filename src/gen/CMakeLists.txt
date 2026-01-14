add_executable(fastdumpgen fastdump_gen.cpp decision_tree.cpp emit.cpp)

target_include_directories(fastdumpgen PRIVATE ..)
