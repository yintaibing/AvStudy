package me.yintaibing.avstudy.video;

import android.view.Surface;

public class Mp4Decoder {
    public native void decodeMp4(String filePath, Surface surface);
}
