//
// Created by Jfree on 12/11/20.
//

#ifndef TENSERFLOWDEMO_NATIVE_GW_H
#define TENSERFLOWDEMO_NATIVE_GW_H

#endif //TENSERFLOWDEMO_NATIVE_GW_H

//Android
#include <jni.h>
#include <android/asset_manager_jni.h>

//OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/hal/interface.h>

// TfLite
#include <tensorflow/lite/c/c_api.h>

//  Standard
#include <vector>
#include <string>
#include <sstream>
#include <chrono>
#include <thread>

// Private
#include "logcat.h"

#define DEBUG true

#define WIDTH 300
#define HEIGHT 300
#define DEPTH 3



//#endif //MOBILEVIDEOANALYSIS_NATIVE_GW_H