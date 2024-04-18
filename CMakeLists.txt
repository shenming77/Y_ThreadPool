message("* * * * * * * * * *  Y_ThreadPool * * * * * * * * * * * * * * *")

cmake_minimum_required(VERSION 3.1.0)

project(CGraph LANGUAGES CXX)

# 默认使用C++14版本，推荐使用C++17版本。不支持C++11以下版本
set(CMAKE_CXX_STANDARD 17)


find_package(Threads REQUIRED)


include_directories (
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

link_directories(
)


file(GLOB_RECURSE LibSource src/*.cpp)
add_library(utils ${LibSource})
target_link_libraries(utils Threads::Threads)
# add_dependencies(utils)

add_executable(y_thread app/main.cpp)
target_link_libraries(y_thread utils)

add_executable(thread_gtest test/threadpool_gtest.cpp)
target_link_libraries(thread_gtest gtest utils)

add_executable(thread_test test/threadpool_test.cpp)
target_link_libraries(thread_test  utils)


enable_testing()
add_test(NAME thread_pool_test COMMAND thread_gtest)

# 使用 Valgrind 的测试
add_test(NAME ValgrindTest1 
         COMMAND valgrind --leak-check=full --error-exitcode=1 $<TARGET_FILE:y_thread>)
         
add_test(NAME ValgrindTest2 
         COMMAND valgrind --leak-check=full --error-exitcode=1 $<TARGET_FILE:thread_test>)
