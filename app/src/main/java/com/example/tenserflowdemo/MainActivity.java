package com.example.tenserflowdemo;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraActivity;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.CvType;
import org.opencv.core.Mat;

import android.Manifest;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PixelFormat;
import android.graphics.PorterDuff;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;

import com.example.tenserflowdemo.utils.AssetUtils;
import com.example.tenserflowdemo.utils.WindowSizeUtils;
import com.tbruyelle.rxpermissions2.RxPermissions;

import java.io.IOException;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 *  auther: JFree
 */

public class MainActivity extends CameraActivity implements CameraBridgeViewBase.CvCameraViewListener2 {

    private CameraBridgeViewBase mCVCamera;
    private RxPermissions rxPermission;
    private BaseLoaderCallback mLoaderCallback;
    private ExecutorService newSingleThreadExecutor;
    private SurfaceHolder surfaceHolder;
    private SurfaceView mSurfaceView;
    private Mat rgba;
    private Canvas canvas;
    private boolean isVisible;
    private Paint paintM,paint;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        initView();
    }

    void initView() {
        mCVCamera = findViewById(R.id.cameraView);
        mSurfaceView = findViewById(R.id.sv_view);
        surfaceHolder = mSurfaceView.getHolder();
        mSurfaceView.setZOrderOnTop(true);
        mSurfaceView.getHolder().setFormat(PixelFormat.TRANSPARENT);
        paintM = new Paint();
        paintM.setColor(getResources().getColor(R.color.colorAccent));
        paintM.setStyle(Paint.Style.STROKE);
        paintM.setStrokeWidth(8.0f);

        paint = new Paint();
        paint.setTextSize(60);
        paint.setColor(Color.YELLOW);
        paint.setStyle(Paint.Style.FILL);

        mCVCamera.setKeepScreenOn(true);
        mCVCamera.setCvCameraViewListener(this);
        mCVCamera.setWinSize(WindowSizeUtils.getWindowsSize(this).getWidth(), WindowSizeUtils.getWindowsSize(this).getHeight());
        rxPermission = new RxPermissions(this);
        rxPermission
                .requestEachCombined(Manifest.permission.CAMERA,
                        Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.RECORD_AUDIO)
                .subscribe(
                        permission -> {
                            if (!permission.granted) {
                                return;
                            } else if (permission.shouldShowRequestPermissionRationale) {

                            } else {

                            }
                        }
                );

        mLoaderCallback = new BaseLoaderCallback(this) {
            @Override
            public void onManagerConnected(int status) {
                switch (status) {
                    case LoaderCallbackInterface.SUCCESS:
                        mCVCamera.enableView();
                        break;
                    default:
                        break;
                }
            }
        };
        newSingleThreadExecutor = Executors.newSingleThreadExecutor();
        NativeLoad.TfLiteInit(getAssets(),"detect.tflite",false,3,0.6f);
    }

    @Override
    protected void onResume() {
        if (OpenCVLoader.initDebug()) {
            mLoaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS);
        } else {
            OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_3_0_0, this, mLoaderCallback);
        }
        super.onResume();
        isVisible = false;
    }

    @Override
    protected void onPause() {
        super.onPause();
        isVisible = true;
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mCVCamera != null) {
            mCVCamera.disableView();
        }
        NativeLoad.TfLiteDestory();
        if (newSingleThreadExecutor != null) {
            newSingleThreadExecutor.shutdown();
        }
    }

    @Override
    protected List<? extends CameraBridgeViewBase> getCameraViewList() {
        return Collections.singletonList(mCVCamera);
    }

    @Override
    public void onCameraViewStarted(int width, int height) {
        rgba = new Mat(width, height, CvType.CV_8UC4);
    }

    @Override
    public void onCameraViewStopped() {
        rgba.release();
    }

    @Override
    public Mat onCameraFrame(CameraBridgeViewBase.CvCameraViewFrame inputFrame) {
        rgba = inputFrame.rgba();
        newSingleThreadExecutor.execute(new Runnable() {
            @Override
            public void run() {
                try {
                    show_detect_results(NativeLoad.TfLiteDetect(rgba.getNativeObjAddr()));
                    Thread.sleep(50);
                }  catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        });
        return rgba;
    }

    private void show_detect_results(List<DetectResult> results){
        float widht = WindowSizeUtils.getWindowsSize(this).getWidth() / 1920;
        float height = WindowSizeUtils.getWindowsSize(this).getHeight() / 1080;
        synchronized (this) {
            if (!isVisible) {
                ClearDraw();
                canvas = surfaceHolder.lockCanvas();
                try {
                    for (int i = 0; i < results.size(); i++) {
                        if (canvas != null) {
                            canvas.drawRect((float) (results.get(i).Left * widht), (float) (results.get(i).Top * height),
                                    (float) (results.get(i).Right * widht),
                                    (float) (results.get(i).Bottom * height), paintM);
                            canvas.drawText(AssetUtils.readAssets(getAssets(), (int) results.get(i).Label + 1)+"   分数："+results.get(i).Score, results.get(i).Left * widht, results.get(i).Top * height-10, paint);
                        }
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                } finally {
                    surfaceHolder.unlockCanvasAndPost(canvas);
                }
            }
        }
    }

    /**
     * 清空上次View
     */
    private void ClearDraw() {
        try {
            canvas = surfaceHolder.lockCanvas(null);
            canvas.drawColor(Color.WHITE);
            canvas.drawColor(Color.TRANSPARENT, PorterDuff.Mode.SRC);
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            if (canvas != null) {
                surfaceHolder.unlockCanvasAndPost(canvas);
            }
        }
    }
}
