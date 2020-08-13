package me.yintaibing.avstudy.audio;

public class Mp3FFmpegOpenSLPlayer extends AbsMp3Player {
    public Mp3FFmpegOpenSLPlayer() {
        super();
    }

    @Override
    protected void doPlay() {
        new Thread() {
            @Override
            public void run() {
                nativeStart(getAudioFilePath());
            }
        }.start();
    }

    private native void nativeStart(String filePath);

    @Override
    protected void doStop() {
        nativeStop();
    }

    private native void nativeStop();
}
