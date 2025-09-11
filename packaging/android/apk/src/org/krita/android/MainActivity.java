/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2019 Sharaf Zaman <sharafzaz121@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

package org.krita.android;

import android.app.ForegroundServiceStartNotAllowedException;
import android.app.ServiceStartNotAllowedException;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.ViewConfiguration;

import androidx.annotation.RequiresApi;

import org.qtproject.qt5.android.QtNative;
import org.qtproject.qt5.android.QtInputEventDispatcher;
import org.qtproject.qt5.android.bindings.QtActivity;

import org.libsdl.app.SDLAudioManager;

public class MainActivity extends QtActivity {

    private boolean haveLibsLoaded = false;
    private boolean serviceStarted = false;
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

        SDLAudioManager.initialize();
        SDLAudioManager.setContext(this);
        SDLAudioManager.nativeSetupJNI();

        super.onCreate(savedInstanceState);
        Log.i(TAG, "TouchSlop: " + ViewConfiguration.get(this).getScaledTouchSlop());
        Log.i(TAG, "LibsLoaded");
        haveLibsLoaded = true;
        new ConfigsManager().handleAssets(this);

        DonationHelper.getInstance();
    }

    @Override
    public void onStart() {
        super.onStart();

        // unlike onCreate where we did this before, this method is called several times throughout the
        // lifecycle of our app, but we intend to run this method only once (and in "Foreground").
        if (!serviceStarted) {
            // Keep the service started so in an unfortunate case where we're not allowed to start a
            // foreground service, we can try to continue without it.
            Intent docSaverServiceIntent = new Intent(this, DocumentSaverService.class);
            startService(docSaverServiceIntent);
            serviceStarted = true;
        }
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
        if (haveLibsLoaded) {
            synchronized(this) {
                startServiceGeneric(DocumentSaverService.START_SAVING);
            }
        }
    }

    void startServiceGeneric(final String action) {
        Intent intent = new Intent(this, DocumentSaverService.class);
        intent.putExtra(action, true);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            startForegroundServiceS(intent);
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            startForegroundService(intent);
        } else {
            startService(intent);
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.S)
    void startForegroundServiceS(Intent intent) {
        try {
            try {
                startForegroundService(intent);
            } catch (ForegroundServiceStartNotAllowedException e) {
                Log.w(TAG, "ForegroundServiceStartNotAllowedException: " + e);

                // The service is already running, so maybe try saving without trying to put it in
                // foreground. According to docs we should have a couple of minutes of runtime.
                startService(intent);
            }
        } catch (ServiceStartNotAllowedException e) {
            // We may not be allowed to start a background service either,
            // probably because onPause is called on an already-paused
            // application that is beyond the "couple of minutes" cutoff.
            Log.w(TAG, "ServiceStartNotAllowedException: " + e);
        }
    }

    @Override
    public void onDestroy() {
        // Docs say: this method will not be called if the activity's hosting process
        // is killed. This means, for us that the service has been stopped.

        Log.i(TAG, "[onDestroy]");
        startServiceGeneric(DocumentSaverService.KILL_PROCESS);

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
            return QtNative.getInputEventDispatcher().sendGenericMotionEvent(event, event.getDeviceId());
        }
        return super.onGenericMotionEvent(event);
    }

    public void onUserInteraction() {
    }
}
