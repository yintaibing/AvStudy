package me.yintaibing.avstudy.video;

import android.content.Intent;
import android.database.Cursor;
import android.graphics.PixelFormat;
import android.net.Uri;
import android.os.Bundle;
import android.provider.MediaStore;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.TextView;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import me.yintaibing.avstudy.MainActivity;
import me.yintaibing.avstudy.R;

public class Mp4DecodeActivity extends AppCompatActivity {
    private static final String TAG = "Mp4DecodeActivity";
    private static final int REQUEST_CODE_SELECT_MP4_FROM_ALBUM = 200;

    private TextView tvFilePath;
    private SurfaceView surfaceView;
    private SurfaceHolder.Callback surfaceHolderCallback;

    private final String defaultVideoFilePath = MainActivity.TEST_MP4;
    private String videoFilePath = defaultVideoFilePath;
    private AbsMp4Player curPlayer;
    private Mp4MediaPlayer mp4MediaPlayer;
    private Mp4FFmpegPlayer mp4FFmpegPlayer;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_mp4_decode);
        setTitle("Mp4Decode");

        tvFilePath = findViewById(R.id.tv_file_path);
        tvFilePath.setText(videoFilePath);

        findViewById(R.id.btn_default_file).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                setVideoFilePath(defaultVideoFilePath);
            }
        });
        findViewById(R.id.btn_album_file).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                selectMp4FromAlbum();
            }
        });

        surfaceView = findViewById(R.id.surface_view);
        SurfaceHolder holder = surfaceView.getHolder();
        holder.setFormat(PixelFormat.RGBA_8888);
        holder.addCallback(surfaceHolderCallback = new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder surfaceHolder) {
                Log.e(TAG, "surfaceCreated");
                if (curPlayer != null) {
                    curPlayer.surfaceCreated(surfaceHolder);
                }
            }

            @Override
            public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int i1, int i2) {
                Log.e(TAG, "surfaceChanged");
                if (curPlayer != null) {
                    curPlayer.surfaceChanged(surfaceHolder, i, i1, i2);
                }
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder surfaceHolder) {
                Log.e(TAG, "surfaceDestroyed");
                if (curPlayer != null) {
                    curPlayer.surfaceDestroyed(surfaceHolder);
                }
            }
        });

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
        SurfaceHolder holder = surfaceView.getHolder();
        holder.removeCallback(surfaceHolderCallback);
        surfaceHolderCallback = null;
    }

    private void setVideoFilePath(String videoFilePath) {
        this.videoFilePath = videoFilePath;
        tvFilePath.setText(videoFilePath);
        if (curPlayer != null) {
            curPlayer.setVideoFilePath(videoFilePath);
        }
    }

    private void selectMp4FromAlbum() {
        Intent i = new Intent(Intent.ACTION_PICK);
        i.setType("video/mp4");
        startActivityForResult(i, REQUEST_CODE_SELECT_MP4_FROM_ALBUM);
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
                String filePath = cursor.getString(columnIndex);
                cursor.close();

                setVideoFilePath(filePath);
            }
        }
    }

    private void play(boolean useFFmpeg) {
        boolean justInit = false;
        if (useFFmpeg) {
            if (mp4FFmpegPlayer == null) {
                justInit = true;
                mp4FFmpegPlayer = new Mp4FFmpegPlayer(surfaceView);
            }
            curPlayer = mp4FFmpegPlayer;
        } else {
            if (mp4MediaPlayer == null) {
                justInit = true;
                mp4MediaPlayer = new Mp4MediaPlayer(surfaceView);
            }
            curPlayer = mp4MediaPlayer;
        }
        if (justInit) {
            curPlayer.surfaceCreated(surfaceView.getHolder());
            curPlayer.setVideoFilePath(videoFilePath);
        }
        curPlayer.tryPlay();
    }
}
