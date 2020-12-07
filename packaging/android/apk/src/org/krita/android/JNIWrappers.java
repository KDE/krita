 /*
  * This file is part of the KDE project
  * Copyright (C) 2019 Sharaf Zaman <sharafzaz121@gmail.com>
  *
  * SPDX-License-Identifier: GPL-2.0-or-later
  */

package org.krita.android;

class JNIWrappers {
	public static native void saveState();
	public static native void exitFullScreen();
	public static native void donationSuccessful();
    public static native void openFileFromIntent(String uri);
}

