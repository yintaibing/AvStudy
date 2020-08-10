package me.yintaibing.avstudy.audio;

import android.media.MediaPlayer;
import android.util.Log;

public class Mp3MediaPlayer extends AbsMp3Player {
    private static final String TAG = "Mp3MediaPlayer";

    private MediaPlayer player;

    public Mp3MediaPlayer() {
        super();
    }

    @Override
    protected void doPlay() {
        if (player == null) {
            createMediaPlayer();
        }
        if (player != null) {
            try {
                player.setDataSource(getAudioFilePath());
                player.prepare();// 等待回调MediaPlayer.OnPreparedListener
            } catch (Exception e) {
                e.printStackTrace();
                stopAndRelease();
            }
        }
    }

    @Override
    public void destroy() {
        super.destroy();
        stopAndRelease();
    }

    private void createMediaPlayer() {
        player = new MediaPlayer();
        player.setOnPreparedListener(new MediaPlayer.OnPreparedListener() {
            @Override
            public void onPrepared(MediaPlayer mp) {
                mp.start();
            }
        });
        player.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
            @Override
            public void onCompletion(MediaPlayer mp) {
                stopAndRelease();
            }
        });
        player.setOnErrorListener(new MediaPlayer.OnErrorListener() {
            @Override
            public boolean onError(MediaPlayer mp, int what, int extra) {
                stopAndRelease();
                return false;
            }
        });
    }

    private void stopAndRelease() {
        if (player != null) {
            if (player.isPlaying()) {
                player.stop();
            }
            player.release();
            player = null;
            Log.e(TAG, "stopAndRelease");
        }
    }
}
