package com.example.tenserflowdemo.utils;

import android.content.res.AssetManager;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.Vector;

public class AssetUtils {

    private static Vector<String> labels = new Vector<String>();

    public static String readAssets(AssetManager assetManager, int dex) throws IOException {
        InputStream labelsInput = assetManager.open("labelmap.txt");
        BufferedReader br = new BufferedReader(new InputStreamReader(labelsInput));
        String line;
        while ((line = br.readLine()) != null) {
            labels.add(line);
        }
        br.close();
        return labels.get(dex);
    }
}
