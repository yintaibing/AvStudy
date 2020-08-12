package me.yintaibing.avstudy;

import android.Manifest;
import android.content.Intent;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.view.View;
import android.widget.Toast;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import me.yintaibing.avstudy.audio.Mp3DecodeActivity;
import me.yintaibing.avstudy.test.TestActivity;
import me.yintaibing.avstudy.video.Mp4DecodeActivity;

public class MainActivity extends AppCompatActivity {
    private static final int REQUEST_CODE_STORAGE = 10;
    private static final String ASSET_MP3 = "dream_it_possible.mp3";
    private static final String ASSET_MP4 = "test.mp4";

    public static volatile String TEST_MP3;
    public static volatile String TEST_MP4;

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
        findViewById(R.id.btn_mp3_decode).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                startActivity(new Intent(MainActivity.this, Mp3DecodeActivity.class));
            }
        });
        findViewById(R.id.btn_mp4_decode).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                startActivity(new Intent(MainActivity.this, Mp4DecodeActivity.class));
            }
        });

        requestExternalStoragePermissions(REQUEST_CODE_STORAGE);

        LibraryLoader.loadLibs();
    }

    private void requestExternalStoragePermissions(int requestCode) {
        ActivityCompat.requestPermissions(this,
                new String[]{Manifest.permission.READ_EXTERNAL_STORAGE,
                        Manifest.permission.WRITE_EXTERNAL_STORAGE}, requestCode);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions,
                                           @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQUEST_CODE_STORAGE) {
            if (ContextUtils.checkGrantPermissionResults(grantResults)) {
                copyTestFilesToStorage();
            }
        }
    }

    private void copyTestFilesToStorage() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                TEST_MP3 = copyTestFileToStorage(ASSET_MP3);
                TEST_MP4 = copyTestFileToStorage(ASSET_MP4);
                getWindow().getDecorView().post(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(MainActivity.this, "ready", Toast.LENGTH_SHORT).show();
                    }
                });
            }
        }).start();
    }

    private String copyTestFileToStorage(String fileName) {
        File cacheDir = getCacheDir();
        File targetFile = new File(cacheDir, fileName);
        if (!targetFile.exists()) {
            try {
                FileOutputStream fos = new FileOutputStream(targetFile);

                AssetManager am = getAssets();
                InputStream in = am.open(fileName);
                byte[] buf = new byte[4096];
                int len;
                while ((len = in.read(buf)) != -1) {
                    fos.write(buf, 0, len);
                }
                in.close();
                fos.flush();
                fos.close();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        return targetFile.getAbsolutePath();
    }
}
