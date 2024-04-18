# CMake generated Testfile for 
# Source directory: /home/shenming77/桌面/Y_ThreadPool
# Build directory: /home/shenming77/桌面/Y_ThreadPool/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(thread_pool_test "/home/shenming77/桌面/Y_ThreadPool/build/thread_gtest")
set_tests_properties(thread_pool_test PROPERTIES  _BACKTRACE_TRIPLES "/home/shenming77/桌面/Y_ThreadPool/CMakeLists.txt;40;add_test;/home/shenming77/桌面/Y_ThreadPool/CMakeLists.txt;0;")
add_test(ValgrindTest1 "valgrind" "--leak-check=full" "--error-exitcode=1" "/home/shenming77/桌面/Y_ThreadPool/build/y_thread")
set_tests_properties(ValgrindTest1 PROPERTIES  _BACKTRACE_TRIPLES "/home/shenming77/桌面/Y_ThreadPool/CMakeLists.txt;43;add_test;/home/shenming77/桌面/Y_ThreadPool/CMakeLists.txt;0;")
add_test(ValgrindTest2 "valgrind" "--leak-check=full" "--error-exitcode=1" "/home/shenming77/桌面/Y_ThreadPool/build/thread_test")
set_tests_properties(ValgrindTest2 PROPERTIES  _BACKTRACE_TRIPLES "/home/shenming77/桌面/Y_ThreadPool/CMakeLists.txt;46;add_test;/home/shenming77/桌面/Y_ThreadPool/CMakeLists.txt;0;")
