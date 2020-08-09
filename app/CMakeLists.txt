# 设置Cmake最小版本
cmake_minimum_required(VERSION 3.4.1)

#set(CMAKE_VERBOSE_MAKEFILE on)

set(ffmpeg_so_dir ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI})
set(my_cpp_dir ${CMAKE_SOURCE_DIR}/src/main/cpp)

#头文件目录
include_directories(
        ${CMAKE_SOURCE_DIR}/src/main/cpp/include
)

# 添加ffmpeg的库
add_library(
        avutil
        SHARED
        IMPORTED
)
set_target_properties(
        avutil
        PROPERTIES IMPORTED_LOCATION
        ${ffmpeg_so_dir}/libavutil.so
)
add_library(
        swresample
        SHARED
        IMPORTED
)
set_target_properties(
        swresample
        PROPERTIES IMPORTED_LOCATION
        ${ffmpeg_so_dir}/libswresample.so
)
add_library(
        avcodec
        SHARED
        IMPORTED
)
set_target_properties(
        avcodec
        PROPERTIES IMPORTED_LOCATION
        ${ffmpeg_so_dir}/libavcodec.so
)
add_library(
        avfilter
        SHARED
        IMPORTED
)
set_target_properties(
        avfilter
        PROPERTIES IMPORTED_LOCATION
        ${ffmpeg_so_dir}/libavfilter.so
)
add_library(
        swscale
        SHARED
        IMPORTED
)
set_target_properties(
        swscale
        PROPERTIES IMPORTED_LOCATION
        ${ffmpeg_so_dir}/libswscale.so
)
add_library(
        avformat
        SHARED
        IMPORTED
)
set_target_properties(
        avformat
        PROPERTIES IMPORTED_LOCATION
        ${ffmpeg_so_dir}/libavformat.so
)
add_library(
        fdk-aac
        SHARED
        IMPORTED
)
set_target_properties(
        fdk-aac
        PROPERTIES IMPORTED_LOCATION
        ${ffmpeg_so_dir}/libfdk-aac.so
)

# 添加自己编译的库。注意只能add_library一次（多个cpp），
add_library( # 设置library名称
        my_native_lib

        # 设置library模式
        # SHARED模式会编译so文件，STATIC模式不会编译
        SHARED
        #STATIC

        # 设置原生代码路径
        ${my_cpp_dir}/JniTest.cpp
        ${my_cpp_dir}/Mp4Decoder.cpp
        )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

# log
#find_library(
#        log-lib
#        log
#)

# 关联library
target_link_libraries(
        my_native_lib
        fdk-aac
        avutil
        swresample
        avcodec
        avfilter
        swscale
        avformat
#        ${log-lib}
        log
        android
)