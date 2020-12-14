#include <jni.h>
#include <string>
#include "native-gw.h"

namespace gw {
    // 定义指针变量 buf 及 size 指针变量
    void *g_buf = NULL;
    int g_buf_size = 0;
    bool g_drawable = false;
    int g_thickness = 3;
    float g_min_score = 0.6;
    int input_tensor_buf_size = 270000;

    TfLiteModel *ptr_Model = NULL;
    TfLiteInterpreterOptions *ptr_Options = NULL;
    TfLiteInterpreter *ptr_Interpreter = NULL;
    TfLiteTensor *ptr_Input_Tensor = NULL;
    const TfLiteTensor *g_output_tensor[3] = {NULL};

    //TODO 用于处理图片大小及调用TFliteTensorCopyFromBuffer（相当于队列把图片传入到缓冲区）
    int Mat2LiteTensor(cv::Mat image,TfLiteTensor *tensor,int widht,int height){

        //处理图片
        cv::Mat fix_img;
        cv::cvtColor(image,fix_img,cv::COLOR_RGBA2BGR);
        cv::resize(fix_img,fix_img,cv::Size(widht,height));
        fix_img.convertTo(fix_img,CV_8UC3);

        //调用TFliteTensorCopyFromBuffer
        if (input_tensor_buf_size != (fix_img.dataend - fix_img.datastart)){
            return -1;
        }
        TfLiteStatus input = TfLiteTensorCopyFromBuffer(tensor, fix_img.data,
                                                         input_tensor_buf_size);
        if (kTfLiteOk != input){
            return 0;
        }
        return 0;
    }

    jobject toDetectResults(JNIEnv *env, float od_boxes[10][4], float od_labels[10],
                            float od_scores[10], int width, int height){
        //获取ArrayList类应用
        jclass list_jcls = env->FindClass("java/util/ArrayList");
        //获取ArrayList构造函数id
        jmethodID list_init = env->GetMethodID(list_jcls, "<init>", "()V");
        //创建一个ArrayList对象
        jobject list_obj = env->NewObject(list_jcls, list_init, "");

        //获取ArrayList对象的add()的methodID
        jmethodID list_add = env->GetMethodID(list_jcls, "add", "(Ljava/lang/Object;)Z");

        //获取自定义的bean
        jclass jcls = env->FindClass("com/example/tenserflowdemo/DetectResult");
        //定义bean类变量属性
        jfieldID j_left = env->GetFieldID(jcls,"Left","I");
        jfieldID j_top = env->GetFieldID(jcls, "Top", "I");
        jfieldID j_right = env->GetFieldID(jcls, "Right", "I");
        jfieldID j_bottom = env->GetFieldID(jcls, "Bottom", "I");
        jfieldID j_label = env->GetFieldID(jcls, "Label", "I");
        jfieldID j_score = env->GetFieldID(jcls, "Score", "F");

        for (int i = 0; i < 10; i++) {

            float score = od_scores[i];
            if (score < gw::g_min_score) {
                continue;
            }

            float label = od_labels[i];
            float ymin = od_boxes[i][0];
            float xmin = od_boxes[i][1];
            float ymax = od_boxes[i][2];
            float xmax = od_boxes[i][3];

            float left = xmin * width;
            if (left < 0) {
                left = 0;
            }
            float top = ymin * height;
            if (top < 0) {
                top = 0;
            }
            float right = xmax * width;
            if (right > width) {
                right = width;
            }
            float bottom = ymax * height;
            if (bottom > height) {
                bottom = height;
            }

            jobject jobj = env->AllocObject(jcls);
            env->SetIntField(jobj, j_left, left);
            env->SetIntField(jobj, j_top, top);
            env->SetIntField(jobj, j_right, right);
            env->SetIntField(jobj, j_bottom, bottom);
            env->SetIntField(jobj, j_label, label);
            env->SetFloatField(jobj, j_score, score);
            env->CallBooleanMethod(list_obj, list_add, jobj);
        }
        return list_obj;
    }
}

//初始化函数
extern "C"
JNIEXPORT void JNICALL
Java_com_example_tenserflowdemo_NativeLoad_TfLiteInit(JNIEnv *env, jclass clazz,
                                                      jobject asset_manager,
                                                      jstring tflite_file_name, jboolean drawable,
                                                      jint thinckess, jfloat filter_score) {

    AAssetManager *mAssetManager = AAssetManager_fromJava(env, asset_manager);
    const char *tflite_file_name_ptr = env->GetStringUTFChars(tflite_file_name, 0);
    AAsset *pAsset = AAssetManager_open(mAssetManager, tflite_file_name_ptr, AASSET_MODE_UNKNOWN);
    off_t buf_size = AAsset_getLength(pAsset);
    const void *buf = AAsset_getBuffer(pAsset);

    gw::g_buf_size = buf_size;
    gw::g_buf = malloc(buf_size + 1);
    memset(gw::g_buf, 0, buf_size + 1);
    memcpy(gw::g_buf, buf, buf_size);

    AAsset_close(pAsset);
    env->ReleaseStringUTFChars(tflite_file_name, tflite_file_name_ptr);

    gw::ptr_Model = TfLiteModelCreate(gw::g_buf, gw::g_buf_size);
    gw::ptr_Options = TfLiteInterpreterOptionsCreate();
    TfLiteInterpreterOptionsSetNumThreads(gw::ptr_Options,2);
    gw::ptr_Interpreter = TfLiteInterpreterCreate(gw::ptr_Model, gw::ptr_Options);

    //5.Allocate tensors and populate the input tensor data.
    TfLiteStatus allocate = TfLiteInterpreterAllocateTensors(gw::ptr_Interpreter);

    gw::ptr_Input_Tensor = TfLiteInterpreterGetInputTensor(gw::ptr_Interpreter, 0);

    for(int i = 0;i<3;i++){
        gw::g_output_tensor[i] = TfLiteInterpreterGetOutputTensor(gw::ptr_Interpreter,i);
    }

    gw::g_drawable = drawable;
    gw::g_thickness = thinckess;
    gw::g_min_score = filter_score;
}
//识别函数
extern "C"
JNIEXPORT jobject JNICALL
Java_com_example_tenserflowdemo_NativeLoad_TfLiteDetect(JNIEnv *env, jclass clazz, jlong addr) {
    // TODO: implement TfLiteDetect()
    cv::Mat &raw_img = *(cv::Mat*)addr;
    gw::Mat2LiteTensor(raw_img,gw::ptr_Input_Tensor,300,300);
    TfLiteStatus invoke = TfLiteInterpreterInvoke(gw::ptr_Interpreter);
    LOGV("native-gw TfLiteDetect", "TfLiteInterpreterInvoke %d", invoke);

    float od_boxes[10][4] ={0};
    float od_labels[10] = {0};
    float od_scores[10] = {10};
    TfLiteStatus ret_box = TfLiteTensorCopyToBuffer(gw::g_output_tensor[0], od_boxes,
                                                    40 * sizeof(float));
    TfLiteStatus ret_class = TfLiteTensorCopyToBuffer(gw::g_output_tensor[1], od_labels,
                                                      10 * sizeof(float));
    TfLiteStatus ret_score = TfLiteTensorCopyToBuffer(gw::g_output_tensor[2], od_scores,
                                                      10 * sizeof(float));

    int width = raw_img.cols;
    int height = raw_img.rows;
    jobject result = gw::toDetectResults(env,od_boxes,od_labels,od_scores,width,height);
    return result;
}
//销毁函数
extern "C"
JNIEXPORT void JNICALL
Java_com_example_tenserflowdemo_NativeLoad_TfLiteDestory(JNIEnv *env, jclass clazz) {
    // TODO: implement TfLiteDestory()
    if (NULL != gw::g_buf) {
        free(gw::g_buf);
        gw::g_buf = NULL;
    }
    if (NULL != gw::ptr_Interpreter) {
        TfLiteInterpreterDelete(gw::ptr_Interpreter);
        gw::ptr_Interpreter = NULL;
    }
    if (NULL != gw::ptr_Options) {
        TfLiteInterpreterOptionsDelete(gw::ptr_Options);
        gw::ptr_Options = NULL;
    }
    if (NULL != gw::ptr_Model) {
        TfLiteModelDelete(gw::ptr_Model);
        gw::ptr_Model = NULL;
    }
}