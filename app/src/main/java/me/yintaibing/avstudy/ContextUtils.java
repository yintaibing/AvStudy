package me.yintaibing.avstudy;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.provider.MediaStore;
import android.provider.Settings;
import android.text.TextUtils;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.OutputStream;

import androidx.annotation.NonNull;
import androidx.core.content.ContextCompat;
import androidx.core.content.FileProvider;

public class ContextUtils {
    public static boolean startUrlIntent(Context context, String action, String url, boolean withNewTaskFlag) {
        try {
            if (TextUtils.isEmpty(action)) {
                action = Intent.ACTION_VIEW;
            }
            Intent i = new Intent(action);
            i.setData(Uri.parse(url));
            if (withNewTaskFlag) {
                i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            }
            context.startActivity(i);
        } catch (Exception e) {
            e.printStackTrace();
            Log.e("startUrlIntent", "失败,url=" + url);
        }
        return true;
    }

    public static boolean isAfterBuildVersion(int versionCode) {
        return Build.VERSION.SDK_INT >= versionCode;
    }

    public static boolean hasPermissions(Context context, String[] permissions) {
        if (!ContextUtils.isAfterBuildVersion(Build.VERSION_CODES.M)) {
            return true;
        }
        for (String p : permissions) {
            if (ContextCompat.checkSelfPermission(context, p) != PackageManager.PERMISSION_GRANTED) {
                return false;
            }
        }
        return true;
    }

    public static boolean checkGrantPermissionResults(@NonNull int[] grantResults) {
        if (grantResults.length > 0) {
            for (int r : grantResults) {
                if (r != PackageManager.PERMISSION_GRANTED) {
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    public static void installApk(Context context, String filePath) {
        File file = new File(filePath);
        Intent intent = new Intent(Intent.ACTION_VIEW);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        Uri uri = null;
        if (isAfterBuildVersion(Build.VERSION_CODES.N)) {
            intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
            uri = FileProvider.getUriForFile(context, context.getPackageName() + ".fileprovider", file);
        } else {
            uri = Uri.fromFile(file);
        }
        intent.setDataAndType(uri, "application/vnd.android.package-archive");
        context.startActivity(intent);
    }

    public static File saveImage(Context context, Bitmap bitmap, boolean saveToAlbum) throws Exception {
        if (isAfterBuildVersion(Build.VERSION_CODES.Q) &&
                saveToAlbum &&
                Environment.MEDIA_MOUNTED.equals(Environment.getExternalStorageState())) {
            // https://www.jianshu.com/p/ee8a95d9525d
            File dir = Environment.getExternalStorageDirectory();
            String brand = Build.BRAND;
            if (brand.equalsIgnoreCase("xiaomi") || brand.equalsIgnoreCase("huawei")) {
                dir = new File(dir, "DCIM/Camera");
            } else {
                dir = new File(dir, "DCIM");
            }
            String fileName = getRandomImageFileName();
            ContentValues values = new ContentValues();
            values.put(MediaStore.Images.Media.DISPLAY_NAME, fileName);
            values.put(MediaStore.Images.Media.RELATIVE_PATH, "DCIM/");
            values.put(MediaStore.Images.Media.MIME_TYPE, "image/JPEG");
            ContentResolver cr = context.getContentResolver();
            Uri uri = cr.insert(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, values);
            if (uri != null) {
                OutputStream out = cr.openOutputStream(uri);
                if (out != null && bitmap.compress(Bitmap.CompressFormat.JPEG, 100, out)) {
                    out.flush();
                    out.close();
                    return new File(dir, fileName);
                }
            }
            return null;
        } else {
            return saveImageSimple(context, bitmap, saveToAlbum);
        }
    }

    private static File saveImageSimple(Context context, Bitmap bitmap, boolean saveToAlbum) throws Exception {
        File dir = context.getFilesDir();
        if (Environment.MEDIA_MOUNTED.equals(Environment.getExternalStorageState()) &&
                context.getExternalFilesDir(null) != null) {
            dir = context.getExternalFilesDir(null);
        }
        dir = new File(dir, "Pictures");
        if (!dir.exists()) {
            dir.mkdirs();
        }
        File f = new File(dir, getRandomImageFileName());
        if (f.exists()) {
            f.delete();
        }
        FileOutputStream out;
        out = new FileOutputStream(f);
        if (bitmap.compress(Bitmap.CompressFormat.JPEG, 100, out)) {
            out.flush();
            out.close();
            if (saveToAlbum) {
                MediaStore.Images.Media.insertImage(
                        context.getContentResolver(), f.getAbsolutePath(), f.getName(), "");
                Intent i = new Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE, Uri.fromFile(f));
                context.sendBroadcast(i);
            }
            return f;
        }
        return null;
    }

    private static String getRandomImageFileName() {
        return System.currentTimeMillis() + ".jpeg";
    }

    public static void gotoNotificationSettings(Context context) {
        String packageName = context.getPackageName();
        try {
            ApplicationInfo applicationInfo = context.getApplicationInfo();
            Intent intent = new Intent(Settings.ACTION_APP_NOTIFICATION_SETTINGS);
            // API>=26
            intent.putExtra(Settings.EXTRA_APP_PACKAGE, packageName);
            intent.putExtra(Settings.EXTRA_CHANNEL_ID, applicationInfo.uid);
            // 21<=API<=25
            intent.putExtra("app_package", packageName);
            intent.putExtra("app_uid", applicationInfo.uid);
            context.startActivity(intent);
        } catch (Exception e) {
            e.printStackTrace();
            // 下面这种方案是直接跳转到当前应用的设置界面
            Intent intent = new Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
            intent.setData(Uri.parse("package:" + context.getPackageName()));
            context.startActivity(intent);
        }
    }
}
