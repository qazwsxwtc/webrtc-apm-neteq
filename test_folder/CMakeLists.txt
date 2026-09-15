cmake_minimum_required(VERSION 3.20)
project(test_folder)
add_library(lib_a STATIC a.cc)
add_library(lib_b STATIC b.cc)
add_executable(exe_1 main.cc)
set_target_properties(lib_a PROPERTIES FOLDER "webrtc/base")
set_target_properties(lib_b PROPERTIES FOLDER "webrtc/codecs")
set_target_properties(exe_1 PROPERTIES FOLDER "example")

