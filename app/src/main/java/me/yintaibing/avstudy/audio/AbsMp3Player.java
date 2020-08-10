package me.yintaibing.avstudy.audio;

import android.text.TextUtils;

import androidx.annotation.CallSuper;

public abstract class AbsMp3Player {
    private static final String TAG = "AbsMp3Player";

    private String audioFilePath;

    public AbsMp3Player() {
    }

    public String getAudioFilePath() {
        return audioFilePath;
    }

    public void setAudioFilePath(String audioFilePath) {
        this.audioFilePath = audioFilePath;
    }

    public void tryPlay() {
        if (!TextUtils.isEmpty(audioFilePath)) {
            doPlay();
        }
    }

    protected abstract void doPlay();

    @CallSuper
    public void destroy() {
    }
}
