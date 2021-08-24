 /*
  * This file is part of the KDE project
  * SPDX-FileCopyrightText: 2019 Sharaf Zaman <sharafzaz121@gmail.com>
  *
  * SPDX-License-Identifier: GPL-2.0-or-later
  */

package org.krita.android;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.os.Build;
import android.os.Environment;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * Copies the default configurations from assets to INTERNAL_STORAGE
 * on first run of the app.
 */
class ConfigsManager {

	private final String LOG_TAG = "krita.ConfigsManager";
	private final String LAST_UPDATE_TIME = "ORG_KRITA_LASTUPDATETIME";
	private long mLastUpdateTime;
	private Activity mActivity;

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

	void handleAssets(Activity activity) {
		mActivity = activity;
		try {
			PackageInfo info = mActivity.getPackageManager().getPackageInfo(mActivity.getPackageName(), 0);
			mLastUpdateTime = info.lastUpdateTime;
		} catch (PackageManager.NameNotFoundException e) {
			Log.e(LOG_TAG, "handleAssets(): packageName not found: ", e);
		}
		if (!isAppUpdated()) {
			return;
		}

		Log.d(LOG_TAG, getStorageDir());
		copyAssets();
		updateLastUpdateTime();
	}

	private void copyAssets() {
		recurse("");
	}

	/**
	 * Recurse into directories
	 * Do not start path with `/` or end with `/`
	 * @param path relative path to asset
	 */
	private void recurse(String path) {
		AssetManager assetManager = mActivity.getAssets();
		String[] assets;
		try {
			assets = assetManager.list(path);
			if (assets == null) {
				return;
			}
			// no assets inside, so a file
			if (assets.length == 0) {
				copyFile(path);
			}
			else {
				for (String asset : assets) {
					if (asset.length() > 0) {
						recurse(toPath(path, asset));
					}
				}
			}
		} catch (IOException ex) {
			Log.e(LOG_TAG, "Could not access assets stream", ex);
		}
	}

	/**
	 * Asset manager API doesn't like trailing `/` at the end and
	 * some other things so we need this for proper path
	 * @param args file names or directories
	 * @return sanitized path
	 */
	private String toPath(String... args) {
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
	}

	private void copyFile(String name) throws IOException {
		InputStream in = null;
		OutputStream out = null;
		try {
			in = mActivity.getAssets().open(name);

			String fileSavePath = toPath(getStorageDir(), "/share/", name);

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
		}
		catch (IOException e) {
			Log.w(LOG_TAG, "Could not copy file: " + name, e);
		}
		finally {
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
		if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)) {
			return mActivity.getExternalFilesDir(null).getAbsolutePath();
		} else {
			return mActivity.getFilesDir().getAbsolutePath();
		}
	}
}

