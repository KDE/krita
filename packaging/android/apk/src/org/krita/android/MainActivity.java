 /*
  * This file is part of the KDE project
  * SPDX-FileCopyrightText: 2019 Sharaf Zaman <sharafzaz121@gmail.com>
  *
  * SPDX-License-Identifier: GPL-2.0-or-later
  */

package org.krita.android;

import android.os.Build;
import android.util.Log;
import android.os.Bundle;
import android.content.Intent;
import android.net.Uri;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.WindowManager;

import org.qtproject.qt5.android.QtNative;
import org.qtproject.qt5.android.bindings.QtActivity;

public class MainActivity extends QtActivity {

    private boolean isStartup = true;
    private final String TAG = "MainActivity";

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.QT_ANDROID_DEFAULT_THEME = "DefaultTheme";

        // we have to do this before loading main()
        Intent i = getIntent();
        String uri = getUri(i);
        if (uri != null) {
            // this will be passed as a command line argument to main()
            i.putExtra("applicationArguments", uri);
        }

        super.onCreate(savedInstanceState);
        Log.i(TAG, "LibsLoaded");
        new ConfigsManager().handleAssets(this);

        DonationHelper.getInstance();
    }

    @Override
    protected void onNewIntent (Intent intent) {
        String uri = getUri(intent);
        if (uri != null) {
            JNIWrappers.openFileFromIntent(uri);
        }

        super.onNewIntent(intent);
    }

    private String getUri(Intent intent) {
        if (intent != null) {
            Uri fileUri = intent.getData();
            if (fileUri != null) {
                return fileUri.toString();
            }
        }
        return null;
    }

    @Override
    public void onPause() {
        super.onPause();
        // onPause() _is_ called when the app starts. If the native lib
        // isn't loaded, it crashes.
        if (!isStartup) {
            synchronized(this) {
                Intent intent = new Intent(this, DocumentSaverService.class);
                intent.putExtra(DocumentSaverService.START_SAVING, true);
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                    startForegroundService(intent);
                } else {
                    startService(intent);
                }
            }
        }
        else {
            isStartup = false;
        }
    }

    @Override
    public void onDestroy() {
        Intent intent = new Intent(this, DocumentSaverService.class);
        intent.putExtra(DocumentSaverService.KILL_PROCESS, true);

        // Docs say: this method will not be called if the activity's hosting process
        // is killed. This means, for us that the service has been stopped.
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            startForegroundService(intent);
        } else {
            startService(intent);
        }

        super.onDestroy();
    }

    @Override
    public boolean onKeyUp(final int keyCode, final KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            if (!JNIWrappers.hasMainWindowLoaded()) {
                // back button was pressed during splash screen, letting this
                // propagate leaves native side in an undefined state. So, it's
                // best we finish the activity here.
                finish();
            } else if (JNIWrappers.exitFullScreen()) {
                return true;
            }
        }

        return super.onKeyUp(keyCode, event);
    }

    @Override
    public boolean onGenericMotionEvent(MotionEvent event) {
        // We manually pass these events to the QPA Android because,
        // android doesn't send events of type other than SOURCE_CLASS_POINTER
        // to the view which was just tapped. So, this view will never get to
        // QtSurface, because it doesn't claim focus.
        if (event.isFromSource(InputDevice.SOURCE_TOUCHPAD)) {
            return QtNative.sendGenericMotionEvent(event, event.getDeviceId());
        }
        return super.onGenericMotionEvent(event);
    }
    }
