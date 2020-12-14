cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_VERBOSE_MAKEFILE on)

set(OPENCV_BASE ${CMAKE_SOURCE_DIR}/src/main/cpp/native)
set(OPENCV_SHARED ${OPENCV_BASE}/libs) # OpenCV动态库
set(OPENCV_HEADER ${OPENCV_BASE}/jni/include) # OpenCV头文件


set(TFLITE_BASE ${CMAKE_SOURCE_DIR}/src/main/cpp/third_party/tensorflow_lite)
set(TFLITE_HEADER ${TFLITE_BASE}/headers)
set(TFLITE_SHARED ${TFLITE_BASE}/libs)

message("OPENCV_HEADER : " ${OPENCV_HEADER})
message("OPENCV_SHARED : " ${OPENCV_SHARED})
message("TFLITE_BASE :" ${TFLITE_BASE})

include_directories(${OPENCV_HEADER} ${TFLITE_HEADER})
#link_directories(${TFLITE_SHARED}/${ANDROID_ABI})

add_library(tensorflowlite_jni
        SHARED
        IMPORTED)
set_target_properties(tensorflowlite_jni PROPERTIES
        IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libtensorflowlite_jni.so)

#  配置依赖库
add_library(libopencv_java4
        SHARED
        IMPORTED)

set_target_properties(libopencv_java4 PROPERTIES
        IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libopencv_java4.so)

add_library(native-gw
        SHARED
        src/main/cpp/native-gw.cpp)

target_link_libraries(native-gw
        libopencv_java4
        tensorflowlite_jni
        android
        log)
