package com.example.tenserflowdemo.utils;

import android.app.Activity;
import android.view.Display;

public class WindowSizeUtils {

    static WindowSizeBean windowSizeBean;
    public static WindowSizeBean getWindowsSize(Activity context){
        if (windowSizeBean == null){
            windowSizeBean = new WindowSizeBean();
        }
        Display display = context.getWindowManager().getDefaultDisplay();
        windowSizeBean.setHeight(display.getHeight());
        windowSizeBean.setWidth(display.getWidth());
        return windowSizeBean;
    }
}
