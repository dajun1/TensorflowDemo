package com.example.tenserflowdemo;

import android.content.res.AssetManager;

import java.util.List;

public class NativeLoad {
    static {
        System.loadLibrary("native-gw");
    }

    public static native void TfLiteInit(AssetManager assetManager,String tflite_file_name,boolean drawable,int thinckess,float filter_score);
    public static native List<DetectResult> TfLiteDetect(long addr);
    public static native void TfLiteDestory();
}
