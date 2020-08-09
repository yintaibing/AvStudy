package me.yintaibing.avstudy;

public class LibraryLoader {
    private static final String MY_LIB_NAME = "my_native_lib";
    private static final String[] LIBS = {
            "fdk-aac",
            "avutil",
            "swresample",
            "avcodec",
            "avfilter",
            "swscale",
            "avformat",
            MY_LIB_NAME
    };

    public static void loadLibs() {
        for (String lib : LIBS) {
            System.loadLibrary(lib);
        }
    }
}
