package me.yintaibing.avstudy.video;

import android.view.Surface;
import android.view.SurfaceView;

public class Mp4FFmpegPlayer extends AbsMp4Player {
    public Mp4FFmpegPlayer(SurfaceView surfaceView) {
        super(surfaceView);
    }

    @Override
    protected void doPlay() {
        new Thread() {
            @Override
            public void run() {
                decodeMp4(getVideoFilePath(), surfaceView.getHolder().getSurface());
            }
        }.start();
    }

    private native void decodeMp4(String filePath, Surface surface);
}
