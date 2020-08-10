package me.yintaibing.avstudy.audio;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;

public class Mp3FFmpegAudioTrackPlayer extends AbsMp3Player {
    private AudioTrack audioTrack;

    public Mp3FFmpegAudioTrackPlayer() {
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

    //C调用，创建AudioTrack
    public void createAudioTrack(int sampleRateInHz, int nb_channals) {
        int channaleConfig;
        if (nb_channals == 1) {
            channaleConfig = AudioFormat.CHANNEL_OUT_MONO;
        } else if (nb_channals == 2) {
            channaleConfig = AudioFormat.CHANNEL_OUT_STEREO;
        } else {
            channaleConfig = AudioFormat.CHANNEL_OUT_MONO;
        }

        int bufferSize = AudioTrack.getMinBufferSize(sampleRateInHz,
                channaleConfig,AudioFormat.ENCODING_PCM_16BIT);
        audioTrack = new AudioTrack(AudioManager.STREAM_MUSIC,
                sampleRateInHz,channaleConfig,
                AudioFormat.ENCODING_PCM_16BIT,bufferSize,AudioTrack.MODE_STREAM);

        audioTrack.play();
    }

    //C调用 传入音频数据
    public synchronized void playTrack(byte[] buffer, int lenth) {
        if (audioTrack != null && audioTrack.getPlayState() == AudioTrack.PLAYSTATE_PLAYING) {
            audioTrack.write(buffer, 0, lenth);
        }
    }
}
