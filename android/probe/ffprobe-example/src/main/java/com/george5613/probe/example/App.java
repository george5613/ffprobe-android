package com.george5613.probe.example;

import android.app.Application;

import com.george5613.probe.logic.ProbeLogic;

import java.io.File;

public class App extends Application {

    private static final String VIDEO_FILE_NAME = "video.mp4";
    private static App mInstance;

    public static App getInstance() {
        return mInstance;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        mInstance = this;
        copyRawVideo();
        ProbeLogic.init(getApplicationContext());
    }

    public File getDestFile() {
        return new File(FileUtils.getFilesDirectory(this).getAbsolutePath() + File.separator + VIDEO_FILE_NAME);
    }

    private void copyRawVideo() {
        File dest = getDestFile();
        if(!dest.exists())
            FileUtils.copyBinaryFromAssetsToData(this, VIDEO_FILE_NAME, VIDEO_FILE_NAME);
    }
}
