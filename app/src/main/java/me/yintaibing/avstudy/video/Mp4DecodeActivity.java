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
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Toast;

import java.io.File;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import me.yintaibing.avstudy.ContextUtils;
import me.yintaibing.avstudy.R;

public class Mp4DecodeActivity extends AppCompatActivity {
    private static final int REQUEST_CODE_EXTERNAL_STORAGE = 100;
    private static final int REQUEST_CODE_SELECT_MP4_FROM_ALBUM = 200;

    private SurfaceView surfaceView;
    private Mp4Decoder mp4Decoder;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_mp4_decode);
        setTitle("Mp4Decode");

        surfaceView = findViewById(R.id.surface_view);
        SurfaceHolder holder = surfaceView.getHolder();
        holder.setFormat(PixelFormat.RGBA_8888);

        surfaceView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                ActivityCompat.requestPermissions(Mp4DecodeActivity.this,
                        new String[]{Manifest.permission.READ_EXTERNAL_STORAGE,
                                Manifest.permission.WRITE_EXTERNAL_STORAGE},
                        REQUEST_CODE_EXTERNAL_STORAGE);
            }
        });

        mp4Decoder = new Mp4Decoder();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQUEST_CODE_EXTERNAL_STORAGE) {
            if (ContextUtils.checkGrantPermissionResults(grantResults)) {
//                selectMp4FromAlbum();
                playLocalMp4();
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
            callDecodeMp4(f.getAbsolutePath());
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
                String filePath = cursor.getString(columnIndex);
                cursor.close();
                callDecodeMp4(filePath);
            }
        }
    }

    private void callDecodeMp4(String filePath) {
        Toast.makeText(this, filePath, Toast.LENGTH_LONG).show();
        if (mp4Decoder != null) {
            mp4Decoder.decodeMp4(filePath, surfaceView.getHolder().getSurface());
        }
    }
}
