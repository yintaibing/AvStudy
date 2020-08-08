# 设置Cmake最小版本
cmake_minimum_required(VERSION 3.4.1)

#头文件目录
include_directories(
    src/main/cpp/include
)

# 编译library
add_library( # 设置library名称
    jni_test

    # 设置library模式
    # SHARED模式会编译so文件，STATIC模式不会编译
    SHARED
    #STATIC

    # 设置原生代码路径
    src/main/cpp/JniTest.cpp
)

# 关联library
target_link_libraries(
    jni_test
)