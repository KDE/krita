 /*
  * This file is part of the KDE project
  * SPDX-FileCopyrightText: 2019 Sharaf Zaman <sharafzaz121@gmail.com>
  *
  * SPDX-License-Identifier: GPL-2.0-or-later
  */

package org.krita.android;

class JNIWrappers {
    public static native void saveState();

    /**
     * Exit the app from full screen.
     *
     * @return true if the app was successfully exited from fullscreen, else
     * false.
     */
    public static native boolean exitFullScreen();

    public static native boolean hasMainWindowLoaded();
    public static native void donationSuccessful();
    public static native void openFileFromIntent(String uri);
}

