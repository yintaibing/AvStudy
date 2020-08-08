# 设置Cmake最小版本
cmake_minimum_required(VERSION 3.4.1)

#头文件目录
include_directories(
        ${CMAKE_SOURCE_DIR}/src/main/cpp/include
        ${CMAKE_SOURCE_DIR}/libs/include
)

# 添加自己编译的库
add_library( # 设置library名称
        jni_test

        # 设置library模式
        # SHARED模式会编译so文件，STATIC模式不会编译
        SHARED
        #STATIC

        # 设置原生代码路径
        src/main/cpp/JniTest.cpp
)

# 添加ffmpeg的库
add_library(
        avcodec
        SHARED
        IMPORTED
)
set_target_properties(
        avcodec
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libavcodec.so
)
add_library(
        avfilter
        SHARED
        IMPORTED
)
set_target_properties(
        avfilter
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libavfilter.so
)
add_library(
        avformat
        SHARED
        IMPORTED
)
set_target_properties(
        avformat
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libavformat.so
)
add_library(
        avutil
        SHARED
        IMPORTED
)
set_target_properties(
        avutil
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libavutil.so
)
add_library(
        swresample
        SHARED
        IMPORTED
)
set_target_properties(
        swresample
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libswresample.so
)
add_library(
        swscale
        SHARED
        IMPORTED
)
set_target_properties(
        swscale
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libswscale.so
)
add_library(
        fdk-aac
        SHARED
        IMPORTED
)
set_target_properties(
        fdk-aac
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libfdk-aac.so
)

# log
find_library(
        log-lib
        log
)

# 关联library
target_link_libraries(
        jni_test
        avcodec
        avfilter
        avformat
        avutil
        swresample
        swscale
        fdk-aac
        ${log-lib}
)