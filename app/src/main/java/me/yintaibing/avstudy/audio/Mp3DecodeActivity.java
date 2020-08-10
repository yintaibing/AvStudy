package me.yintaibing.avstudy.audio;

import android.os.Bundle;
import android.os.Environment;
import android.view.SurfaceHolder;
import android.view.View;
import android.widget.TextView;

import java.io.File;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import me.yintaibing.avstudy.R;

public class Mp3DecodeActivity extends AppCompatActivity {
    private static final String TAG = "Mp3DecodeActivity";

    private static final String DEFAULT_FILE_PATH =
            Environment.getExternalStorageDirectory() + File.separator + "avstudy_dream_it_possible.mp3";

    private TextView tvFilePath;

    private String audioFilePath = DEFAULT_FILE_PATH;
    private AbsMp3Player curPlayer;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_mp3_decode);
        setTitle("Mp3Decode");

        tvFilePath = findViewById(R.id.tv_file_path);
        tvFilePath.setText(audioFilePath);

        findViewById(R.id.btn_use_media_player).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                play(false);
            }
        });
        findViewById(R.id.btn_use_ffmpeg_player).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                play(true);
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

    private void play(boolean useFFmpeg) {
        if (curPlayer == null) {
            curPlayer = new Mp3MediaPlayer();
            curPlayer.setAudioFilePath(audioFilePath);
        }
        curPlayer.tryPlay();
    }
}
