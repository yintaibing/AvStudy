package me.yintaibing.avstudy.audio;

public class AudioInfo {
    int sampleRate;
    int sampleFormat;
    long channels;

    public long getBitRate() {
        return sampleRate * sampleFormat * channels;
    }
}
