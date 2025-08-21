/*
 * SPDX-FileCopyrightText: 2022 Sharaf Zaman <shzam@sdf.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

package org.krita.android;

import static android.os.Process.killProcess;
import static android.os.Process.myPid;

import android.app.ForegroundServiceStartNotAllowedException;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Intent;
import android.os.Build;
import android.os.IBinder;
import android.util.Log;

import androidx.annotation.RequiresApi;

import org.krita.R;

public class DocumentSaverService extends Service {

    private final String TAG = "DocumentSaverService";
    private final String CHANNEL_ID = "org.krita.android";
    private final int NOTIFICATION_ID = 3;

    public static final String START_SAVING = "START_SAVING";
    public static final String KILL_PROCESS = "KILL_PROCESS";
    private static final String CANCEL_SAVING = "CANCEL_SAVING";

    private boolean mStartedInForeground = false;
    private boolean mSaveAgain = false;
    private boolean mKillProcess = false;
    private Thread mDocSaverThread = null;

    @Override
    public void onCreate() {
        super.onCreate();
        Log.i(TAG, "[onCreate]");

        createNotificationChannel();
    }

    private Notification getNotification() {
        Notification notification;
        Intent intent = new Intent(this, DocumentSaverService.class);
        intent.putExtra(CANCEL_SAVING, true);
        PendingIntent cancelPendingIntent = PendingIntent.getService(this, 0,
                intent, PendingIntent.FLAG_IMMUTABLE);
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
            notification = new Notification.Builder(this, CHANNEL_ID)
                    .setContentTitle(getString(R.string.save_notification_text))
                    .setSmallIcon(R.drawable.ic_save_notification)
                    .addAction(new Notification.Action.Builder(null,
                            getString(R.string.cancel_save_notification), cancelPendingIntent).build())
                    .setProgress(0, 0, true)
                    .build();
        } else {
            notification = new Notification.Builder(this)
                    .setContentTitle(getString(R.string.save_notification_text))
                    .setSmallIcon(R.drawable.ic_save_notification)
                    .setPriority(Notification.PRIORITY_LOW)
                    .setProgress(0, 0, true)
                    .build();
        }

        return notification;
    }

    private void createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            CharSequence name = "Default";
            int importance = NotificationManager.IMPORTANCE_LOW;
            NotificationChannel channel = new NotificationChannel(CHANNEL_ID, name, importance);

            NotificationManager notificationManager = getSystemService(NotificationManager.class);
            notificationManager.createNotificationChannel(channel);
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.S)
    private void tryStartServiceInForegroundS() {
        try {
            Log.w(TAG, "Starting the service in foreground");
            startForeground(NOTIFICATION_ID, getNotification());
            mStartedInForeground = true;
        } catch (ForegroundServiceStartNotAllowedException e) {
            Log.w(TAG, "Could not run the service in foreground: " + e);
            mStartedInForeground = false;
        }
    }

    private void tryStartServiceInForeground() {
        startForeground(NOTIFICATION_ID, getNotification());
        mStartedInForeground = true;
    }

    private synchronized void resetSaveAgain() {
        Log.i(TAG, "Start saving, foreground " + mStartedInForeground + " again " + mSaveAgain + " kill " + mKillProcess);
        mSaveAgain = false;
    }

    private synchronized boolean shouldSaveAgain() {
        Log.i(TAG, "Saving done, foreground " + mStartedInForeground + " again " + mSaveAgain + " kill " + mKillProcess);
        return mSaveAgain && !mKillProcess;
    }

    private synchronized void finishSaverThread() {
        if (mStartedInForeground) {
            mStartedInForeground = false;
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                stopForeground(STOP_FOREGROUND_REMOVE);
            } else {
                stopForeground(true);
            }
        }
        mDocSaverThread = null;
    }

    private void runSaverThread() {
        do {
            resetSaveAgain();
            try {
                JNIWrappers.saveState();
            } catch (UnsatisfiedLinkError e) {
                // This can happen if the application has been unloaded and only
                // this saver thread is left dangling. Since the application is
                // gone, there's no documents left to save, so just bail out.
                Log.w(TAG, "Unsatisfied link error while saving: " + e);
                break;
            }
        } while (shouldSaveAgain());
        finishSaverThread();
    }

    private synchronized void startSaving() {
        if (mDocSaverThread == null) {
            // Promote this to a so-called foreground service. This will keep
            // the service running even when the application isn't in the
            // foreground. For further foreground/background confusion, my
            // Samsung tablet shows the notification for this service under a
            // (pretty well-hidden) section called "background activities".
            if (mStartedInForeground) {
                Log.w(TAG, "Already started in foreground, should not happen");
            } else {
                Log.i(TAG, "Attempting to promote to foreground service");
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                    tryStartServiceInForegroundS();
                } else {
                    tryStartServiceInForeground();
                }
            }
            // No matter if foregrounding succeeded or not, we spawn a thread to
            // do the saving on. That will take care of un-foregrounding the
            // service and nulling the member variable once it's finished.
            Log.i(TAG, "Starting save thread");
            mDocSaverThread =
                    new Thread(
                            new Runnable() {
                                @Override
                                public void run() {
                                    runSaverThread();
                                }
                            });
            mDocSaverThread.start();
        } else {
            Log.i(TAG, "Save thread already running, setting save again flag");
            mSaveAgain = true;
        }
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.i(TAG, "[onStartCommand]");
        if (intent.getBooleanExtra(START_SAVING, false)) {
            startSaving();

        } else if (intent.getBooleanExtra(KILL_PROCESS, false)) {
            Log.i(TAG, "kill process command received");

            boolean needsThread;
            synchronized (this) {
                mKillProcess = true;
                needsThread = mStartedInForeground;
            }

            if (needsThread) {
                // This can be async, because foreground service are promised to run beyond process death.
                // This has to be async, because we have to handle cancelling action.
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        waitForSaving();
                        // if saving finished a long time ago, we will have to kill the app
                        stopSelf();
                    }
                }).start();
            } else {
                // We wait on Main thread. When we don't do this and use Background service, if the
                // Activity is killed the Service is automatically destroyed without waiting for threads
                // to start/finish.
                waitForSaving();
                stopSelf();
            }

        } else if (intent.getBooleanExtra(CANCEL_SAVING, false)) {
            Log.i(TAG, "cancel saving command received");
            // without this Android will think we crashed
            stopSelf();

            // TODO: Think about atomicity, which Qt doesn't support for Android
            killProcess(myPid());

        } else {
            Log.i(TAG, "nothing to do");
        }

        return START_NOT_STICKY;
    }

    @Override
    public void onDestroy() {
        Log.i(TAG, "Destroying: Kill Process = " + mKillProcess);
        waitForSaving();

        if (mKillProcess) {
            // We cannot let the process sit around when the activity is
            // destroyed. QtActivity depends on lot of statics. We also can't
            // exit (libc) using Qt's methods because they crash.
            killProcess(myPid());
        }
        super.onDestroy();
    }

    private synchronized Thread getThreadToWaitOn() {
        Log.i(TAG, "[waitForSaving]: mStartedInForeground: " + mStartedInForeground);
        return mDocSaverThread;
    }

    private void waitForSaving() {
        Thread thread = getThreadToWaitOn();
        if (thread != null) {
            try {
                thread.join();
            } catch (InterruptedException e) {
                Log.e(TAG, "Saving Interrupted :" + e.getMessage());
            }
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
}
