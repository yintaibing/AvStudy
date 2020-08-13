package me.yintaibing.avstudy.audio;

import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import me.yintaibing.avstudy.MainActivity;
import me.yintaibing.avstudy.R;

public class Mp3DecodeActivity extends AppCompatActivity {
    private static final String TAG = "Mp3DecodeActivity";

    private TextView tvFilePath;
    private TextView tvAudioInfo;

    private final String defaultAudioFilePath = MainActivity.TEST_MP3;
    private String audioFilePath = defaultAudioFilePath;
    private AbsMp3Player curPlayer;
    private Mp3MediaPlayer mp3MediaPlayer;
    private Mp3FFmpegAudioTrackPlayer mp3FFmpegAudioTrackPlayer;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_mp3_decode);
        setTitle("Mp3Decode");

        tvFilePath = findViewById(R.id.tv_file_path);
        tvFilePath.setText(audioFilePath);
        tvAudioInfo = findViewById(R.id.tv_audio_info);

        findViewById(R.id.btn_default_mp3).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                setAudioFilePath(MainActivity.TEST_MP3);
            }
        });
        findViewById(R.id.btn_default_aac).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                setAudioFilePath(MainActivity.TEST_AAC);
            }
        });

        findViewById(R.id.btn_use_media_player).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                play(0);
            }
        });
        findViewById(R.id.btn_use_ffmpeg_audio_track_player).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                play(1);
            }
        });
        findViewById(R.id.btn_use_ffmpeg_open_sl_player).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                play(2);
            }
        });
        findViewById(R.id.btn_stop).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (curPlayer != null) {
                    curPlayer.stop();
                }
            }
        });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (curPlayer != null) {
            curPlayer.stop();
            curPlayer.destroy();
            curPlayer = null;
        }
    }

    private void setAudioFilePath(String audioFilePath) {
        this.audioFilePath = audioFilePath;
        tvFilePath.setText(audioFilePath);
        if (curPlayer != null) {
            curPlayer.setAudioFilePath(audioFilePath);
        }
    }

    private void play(int which) {
        switch (which) {
            case 0:
                curPlayer = new Mp3MediaPlayer();
                break;
            case 1:
                curPlayer = new Mp3FFmpegAudioTrackPlayer();
                break;
            case 2:
                curPlayer = new Mp3FFmpegOpenSLPlayer();
                break;
            default:
                return;
        }
        curPlayer.setCallback(new AbsMp3Player.Callback() {
            @Override
            public void onGotAudioInfo(AudioInfo audioInfo) {
                showAudioInfo(audioInfo);
            }
        });
        curPlayer.setAudioFilePath(audioFilePath);
        curPlayer.tryPlay();
    }

    private void showAudioInfo(AudioInfo audioInfo) {
        if (audioInfo == null) return;
        final String text = "音频信息：" +
                "\n采样率：" + audioInfo.sampleRate +
                "\n位深度：" + audioInfo.sampleFormat +
                "\n声道数：" + audioInfo.channels /*+
                "\n比特率：" + audioInfo.getBitRate()*/;
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                tvAudioInfo.setText(text);
            }
        });
    }
}
