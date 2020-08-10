package me.yintaibing.avstudy.audio;

import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.widget.TextView;

import java.io.File;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import me.yintaibing.avstudy.R;

public class Mp3DecodeActivity extends AppCompatActivity {
    private static final String TAG = "Mp3DecodeActivity";

    private static final String DEFAULT_FILE_PATH = Environment.getExternalStorageDirectory() + File.separator
//            + "avstudy_dream_it_possible.mp3";
    + "MIUI/sound_recorder/avstudy_audio.mp3";

    private TextView tvFilePath;

    private String audioFilePath = DEFAULT_FILE_PATH;
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

        findViewById(R.id.btn_default_file).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                setAudioFilePath(DEFAULT_FILE_PATH);
            }
        });
        findViewById(R.id.btn_album_file).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
//                selectMp4FromAlbum();
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
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (curPlayer != null) {
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
        curPlayer.setAudioFilePath(audioFilePath);
        curPlayer.tryPlay();
    }
}
