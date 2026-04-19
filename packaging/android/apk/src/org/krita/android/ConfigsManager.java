 /*
  * This file is part of the KDE project
  * SPDX-FileCopyrightText: 2019 Sharaf Zaman <sharafzaz121@gmail.com>
  *
  * SPDX-License-Identifier: GPL-2.0-or-later
  */

package org.krita.android;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.os.Build;
import android.os.Environment;
import android.view.Gravity;
import android.view.Window;
import android.view.WindowManager;
import android.util.Log;
import android.widget.ProgressBar;

import org.qtproject.qt5.android.QtNative;
import org.qtproject.qt5.android.bindings.QtActivity;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.HashSet;
import java.util.Set;

/**
 * Copies the default configurations from assets to INTERNAL_STORAGE
 * on first run of the app.
 */
class ConfigsManager {

    private final String LOG_TAG = "krita.ConfigsManager";
    private final String LAST_UPDATE_TIME = "ORG_KRITA_LASTUPDATETIME";
    private final Activity mActivity;
    private final String mStorageDir;
    private long mLastUpdateTime = 0L;
    private AlertDialog mAlertDialog = null;
    private boolean mAssetsCopied = false;

    public ConfigsManager(Activity activity) {
        mActivity = activity;
        mStorageDir = getStorageDir();
    }

    private void updateLastUpdateTime() {
        SharedPreferences sharedPref = mActivity.getPreferences(Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = sharedPref.edit();
        editor.putLong(LAST_UPDATE_TIME, mLastUpdateTime);
        editor.apply();
    }

    private boolean isAppUpdated() {
        return mActivity.getPreferences(Context.MODE_PRIVATE)
                        .getLong(LAST_UPDATE_TIME, 0) != mLastUpdateTime;
    }

    public void handleAssets() {
        try {
            PackageInfo info = mActivity.getPackageManager().getPackageInfo(mActivity.getPackageName(), 0);
            mLastUpdateTime = info.lastUpdateTime;
        } catch (PackageManager.NameNotFoundException e) {
            Log.e(LOG_TAG, "handleAssets(): packageName not found: ", e);
        }
        if (!isAppUpdated()) {
            return;
        }

        // Copying assets can take quite a while, so we show a spinner dialog
        // during it. The functions relating to that are all synchronized,
        // which means they take a mutex so that we don't end up with a race.
        setAssetsCopied(false);
        QtNative.activity().runOnUiThread(() -> {
            try {
                showDialog();
            } catch (Exception e) {
                Log.e(LOG_TAG, "Failed to show progress dialog", e);
            }
        });

        Log.i(LOG_TAG, "storage dir: " + mStorageDir);
        try {
            copyAssets();
        } catch (Exception e) {
            Log.e(LOG_TAG, "Failed to copy assets", e);
        }

        try {
            updateLastUpdateTime();
        } catch (Exception e) {
            Log.e(LOG_TAG, "Failed to update last update time", e);
        }

        setAssetsCopied(true);
        QtNative.activity().runOnUiThread(() -> {
            try {
                dismissDialog();
            } catch (Exception e) {
                Log.e(LOG_TAG, "Failed to dismiss progress dialog", e);
            }
        });
    }

    private synchronized void setAssetsCopied(boolean assetsCopied) {
        // Don't remove this function, it exists because it's synchronized! We
        // have to take a lock to avoid races with the UI thread.
        mAssetsCopied = assetsCopied;
    }

    private synchronized void showDialog() {
        // We might already be done copying the assets. This function is
        // synchronized!
        if (!mAssetsCopied) {
            ProgressBar progressBar = new ProgressBar(mActivity);
            progressBar.setPadding(100, 100, 100, 100);

            mAlertDialog = new AlertDialog.Builder(mActivity, AlertDialog.THEME_DEVICE_DEFAULT_DARK)
                .setCancelable(false)
                .setView(progressBar)
                .create();

            // This dialog usually appears and disappears very quickly, so
            // having animations looks pretty wonky. Turn those off.
            Window window = mAlertDialog.getWindow();
            if (window != null) {
                window.setWindowAnimations(0);
            }

            mAlertDialog.show();

            // By default, the alert dialog is full-width. This clamps the
            // dialog bounds to the spinner so that it looks sensible.
            window = mAlertDialog.getWindow();
            if (window != null) {
                window.setLayout(WindowManager.LayoutParams.WRAP_CONTENT, WindowManager.LayoutParams.WRAP_CONTENT);
                window.setGravity(Gravity.CENTER);
            }
        }
    }

    private synchronized void dismissDialog() {
        // We may not have a dialog if we copied stuff fast enough. Again, note
        // this function is synchronized!
        if (mAlertDialog != null) {
            mAlertDialog.dismiss();
            mAlertDialog = null;
        }
    }

    private void copyAssets() {
        AssetManager assetManager = mActivity.getAssets();
        Set<String> handledPaths = new HashSet<String>();
        recurse(assetManager, handledPaths, "");
    }

    private void recurse(AssetManager assetManager, Set<String> handledPaths, String path) {
        try {
            String[] assets = assetManager.list(path);
            if (assets == null) {
                return;
            }
            // no assets inside, so a file
            if (assets.length == 0) {
                copyFile(path);
                handledPaths.add(path);
            } else {
                for (String asset : assets) {
                    if (asset.length() > 0) {
                        String childPath = toPath(path, asset);
                        if (childPath != null) {
                            recurse(assetManager, handledPaths, childPath);
                        } else {
                        }
                    }
                }
            }
        } catch (Exception e) {
            Log.e(LOG_TAG, "Error copying assets '" + path + "'", e);
        }
    }

    /**
     * Asset manager API doesn't like trailing `/` at the end and
     * some other things so we need this for proper path
     * @param args file names or directories
     * @return sanitized path
     */
    private String toPath(String... args) {
        try {
            StringBuilder result = new StringBuilder();
            for (String item: args) {
                if (!item.isEmpty()) {
                    if (result.length() != 0 && result.charAt(result.length() - 1) != '/') {
                        result.append('/');
                    }
                    result.append(item);
                    if (item.charAt(item.length() - 1) != '/') {
                        result.append('/');
                    }
                }
            }

            // if last character is '/', then delete it
            if (result.length() != 0 && result.charAt(result.length() - 1) == '/') {
                result.deleteCharAt(result.length() - 1);
            }

            return result.toString();
        } catch (Exception e) {
            Log.e(LOG_TAG, "Error building path", e);
            return null;
        }
    }

    private void copyFile(String name) throws IOException {
        InputStream in = null;
        OutputStream out = null;
        try {
            in = mActivity.getAssets().open(name);

            String fileSavePath = toPath(mStorageDir, "/share/", name);

            // use the same directory structure
            File base = new File(basePath(fileSavePath));
            if (!base.exists()) {
                base.mkdirs();
            }

            out = new FileOutputStream(fileSavePath);
            byte[] buffer = new byte[4 * 1024];
            int read;
            while ((read = in.read(buffer)) != -1) {
                out.write(buffer, 0, read);
            }
        } catch (Exception e) {
            Log.w(LOG_TAG, "Could not copy file '" + name + "'", e);
        } finally {
            if (in != null) {
                in.close();
            }
            if (out != null) {
                out.close();
            }
        }
    }

    /**
     * Returns base path for example, in /data/foo it would
     * return /data
     * @param path to be sanitized
     * @return base path
     */
    private String basePath(String path) {
        for (int i = path.length() - 1; i >= 0; --i) {
            if (path.charAt(i) == '/') {
                return path.substring(0, i);
            }
        }
        return "";
    }

    private String getStorageDir() {
        return mActivity.getFilesDir().getAbsolutePath();
    }
}

