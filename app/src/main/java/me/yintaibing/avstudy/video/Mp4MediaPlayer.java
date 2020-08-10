package me.yintaibing.avstudy.video;

import android.media.MediaPlayer;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class Mp4MediaPlayer extends AbsMp4Player {
    private static final String TAG = "Mp4MediaPlayer";

    private MediaPlayer player;
    private SurfaceHolder surfaceHolder;

    public Mp4MediaPlayer(SurfaceView surfaceView) {
        super(surfaceView);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        super.surfaceCreated(holder);
        this.surfaceHolder = holder;
    }

    @Override
    protected void doPlay() {
        if (player == null) {
            createMediaPlayer();
        }
        if (player != null && surfaceHolder != null) {
            try {
                player.setDisplay(surfaceHolder);
                player.setDataSource(getVideoFilePath());
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
        surfaceHolder = null;
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
