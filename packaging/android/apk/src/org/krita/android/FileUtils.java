/*
 * SPDX-FileCopyrightText: 2007-2008 OpenIntents.org
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * This file was modified by Sharaf Zaman <sharafzaz121@gmail.com> from
 * following original file: https://github.com/iPaulPro/aFileChooser/blob/master/aFileChooser/src/com/ipaulpro/afilechooser/utils/FileUtils.java
 */

package org.krita.android;

import android.content.Context;
import android.net.Uri;
import android.os.Environment;
import android.provider.DocumentsContract;


@SuppressWarnings("unused")
public class FileUtils {
	private FileUtils() {}

	public static boolean isExternalStorageDocument(Uri uri) {
		return "com.android.externalstorage.documents".equals(uri.getAuthority());
	}

	public static String getPath(final Context context, final Uri uri) {
		if (DocumentsContract.isDocumentUri(context, uri) && isExternalStorageDocument(uri)) {
				final String docId = DocumentsContract.getDocumentId(uri);
				final String[] split = docId.split(":");
				final String type = split[0];

				if ("primary".equalsIgnoreCase(type)) {
					return Environment.getExternalStorageDirectory() + "/" + split[1];
				}
			}
		return uri.toString();
    }
}
