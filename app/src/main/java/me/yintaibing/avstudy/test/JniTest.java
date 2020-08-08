package me.yintaibing.avstudy.test;

public class JniTest {
    private static boolean loadLibrary = false;

    public String stringFromJNI(String inputStr) {
        if (!loadLibrary) {
            System.loadLibrary("jni_test");
            loadLibrary = true;
        }
        return nativeStringFromJNI(inputStr);
    }

    private native String nativeStringFromJNI(String inputStr);
}
