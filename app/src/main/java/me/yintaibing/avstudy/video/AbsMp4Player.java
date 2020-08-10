package me.yintaibing.avstudy.video;

import android.text.TextUtils;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import androidx.annotation.CallSuper;

public abstract class AbsMp4Player implements SurfaceHolder.Callback {
    private static final String TAG = "AbsMp4Player";

    protected SurfaceView surfaceView;
    protected boolean surfaceReady;
    private String videoFilePath;

    public AbsMp4Player(SurfaceView surfaceView) {
        this.surfaceView = surfaceView;
//        SurfaceHolder holder = surfaceView.getHolder();
//        holder.setFormat(PixelFormat.RGBA_8888);
//        holder.addCallback(this);
    }

    public String getVideoFilePath() {
        return videoFilePath;
    }

    public void setVideoFilePath(String videoFilePath) {
        this.videoFilePath = videoFilePath;
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        surfaceReady = true;
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        surfaceReady = false;
    }

    public void tryPlay() {
        if (!TextUtils.isEmpty(videoFilePath) && surfaceReady) {
            doPlay();
        }
    }

    protected abstract void doPlay();

    @CallSuper
    public void destroy() {
//        SurfaceHolder holder = surfaceView.getHolder();
//        holder.removeCallback(this);
    }
}
