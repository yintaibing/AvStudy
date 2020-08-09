package me.yintaibing.avstudy.video;

import android.Manifest;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.PixelFormat;
import android.media.MediaPlayer;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.provider.MediaStore;
import android.text.TextUtils;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;

import java.io.File;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import me.yintaibing.avstudy.ContextUtils;
import me.yintaibing.avstudy.R;

public class MediaPlayerActivity extends AppCompatActivity {
    private static final String TAG = "MediaPlayerActivity";

    private static final int REQUEST_CODE_AVSTUDY_DOG = 100;
    private static final int REQUEST_CODE_ALBUM_VIDEO = 101;
    private static final int REQUEST_CODE_SELECT_MP4_FROM_ALBUM = 200;

    private SurfaceView surfaceView;
    private MediaPlayer player;
    private boolean surfaceReady;
    private String videoFile;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_mp4_decode);
        setTitle("MediaPlayer");

        surfaceView = findViewById(R.id.surface_view);
        SurfaceHolder holder = surfaceView.getHolder();
        holder.setFormat(PixelFormat.RGBA_8888);
        holder.addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder surfaceHolder) {
                setMediaPlayerDisplay(surfaceHolder);
                tryPlay();
                Log.e(TAG, "surfaceCreated");
            }

            @Override
            public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int i1, int i2) {
                Log.e(TAG, "surfaceChanged");
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder surfaceHolder) {
                surfaceReady = false;
                Log.e(TAG, "surfaceDestroyed");
            }
        });

        findViewById(R.id.btn_avstudy_dog).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                requestExternalStoragePermissions(REQUEST_CODE_AVSTUDY_DOG);
            }
        });
        findViewById(R.id.btn_album_video).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                requestExternalStoragePermissions(REQUEST_CODE_ALBUM_VIDEO);
            }
        });

        player = new MediaPlayer();
        player.setOnPreparedListener(new MediaPlayer.OnPreparedListener() {
            @Override
            public void onPrepared(MediaPlayer mediaPlayer) {
                player.start();
            }
        });
    }

    private void requestExternalStoragePermissions(int requestCode) {
        ActivityCompat.requestPermissions(this,
                new String[]{Manifest.permission.READ_EXTERNAL_STORAGE,
                        Manifest.permission.WRITE_EXTERNAL_STORAGE}, requestCode);
    }

    private void setMediaPlayerDisplay(SurfaceHolder surfaceHolder) {
        if (!surfaceReady) {
            surfaceReady = true;
            player.setDisplay(surfaceHolder);
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (player != null) {
            player.stop();
            player.release();
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQUEST_CODE_AVSTUDY_DOG || requestCode == REQUEST_CODE_ALBUM_VIDEO) {
            if (ContextUtils.checkGrantPermissionResults(grantResults)) {
                if (requestCode == REQUEST_CODE_AVSTUDY_DOG) {
                    playLocalMp4();
                } else {
                    selectMp4FromAlbum();
                }
            }
        }
    }

    private void selectMp4FromAlbum() {
        Intent i = new Intent(Intent.ACTION_PICK);
        i.setType("video/mp4");
        startActivityForResult(i, REQUEST_CODE_SELECT_MP4_FROM_ALBUM);
    }

    private void playLocalMp4() {
        File f = new File(Environment.getExternalStorageDirectory(), "avstudy_dog.mp4");
        if (f.exists()) {
            videoFile = f.getAbsolutePath();
            tryPlay();
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == REQUEST_CODE_SELECT_MP4_FROM_ALBUM) {
            if (resultCode == RESULT_OK && data != null) {
                Uri selectedVideo = data.getData();
                String[] filePathColumn = {MediaStore.Video.Media.DATA};

                Cursor cursor = getContentResolver().query(selectedVideo,
                        filePathColumn, null, null, null);
                cursor.moveToFirst();

                int columnIndex = cursor.getColumnIndex(filePathColumn[0]);
                videoFile = cursor.getString(columnIndex);
                cursor.close();
                tryPlay();
            }
        }
    }

    private void tryPlay() {
        if (player != null && !TextUtils.isEmpty(videoFile) && surfaceReady) {
            try {
                player.setDataSource(videoFile);
                player.prepareAsync();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }
}
