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
                decodeMp3(getAudioFilePath());
            }
        }.start();
    }

    private native void decodeMp3(String filePath);

    private native void stop();
}
