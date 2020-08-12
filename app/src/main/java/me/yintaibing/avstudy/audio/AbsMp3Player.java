package me.yintaibing.avstudy.audio;

import android.text.TextUtils;

import androidx.annotation.CallSuper;

public abstract class AbsMp3Player {
    private static final String TAG = "AbsMp3Player";

    private String audioFilePath;
    private Callback callback;

    public AbsMp3Player() {
    }

    public String getAudioFilePath() {
        return audioFilePath;
    }

    public void setAudioFilePath(String audioFilePath) {
        this.audioFilePath = audioFilePath;
    }

    public void setCallback(Callback callback) {
        this.callback = callback;
    }

    public void onGotAudioInfo(int sampleRate,
                               int sampleFormat,
                               Long channels) {
        if (callback != null) {
            AudioInfo audioInfo = new AudioInfo();
            audioInfo.sampleRate = sampleRate;
            audioInfo.sampleFormat = sampleFormat;
            audioInfo.channels = channels;
            callback.onGotAudioInfo(audioInfo);
        }
    }

    public void tryPlay() {
        if (!TextUtils.isEmpty(audioFilePath)) {
            doPlay();
        }
    }

    protected abstract void doPlay();

    @CallSuper
    public void destroy() {
        callback = null;
    }

    interface Callback {
        void onGotAudioInfo(AudioInfo audioInfo);
    }
}
