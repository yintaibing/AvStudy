package me.yintaibing.avstudy;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import me.yintaibing.avstudy.test.TestActivity;
import me.yintaibing.avstudy.video.MediaPlayerActivity;
import me.yintaibing.avstudy.video.Mp4DecodeActivity;

public class MainActivity extends AppCompatActivity {
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        findViewById(R.id.btn_jni_test).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                startActivity(new Intent(MainActivity.this, TestActivity.class));
            }
        });
        findViewById(R.id.btn_media_player).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                startActivity(new Intent(MainActivity.this, MediaPlayerActivity.class));
            }
        });
        findViewById(R.id.btn_mp4_decode).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                startActivity(new Intent(MainActivity.this, Mp4DecodeActivity.class));
            }
        });

        LibraryLoader.loadLibs();
    }
}
