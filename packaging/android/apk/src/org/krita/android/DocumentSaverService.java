/*
 * SPDX-FileCopyrightText: 2021 Sharaf Zaman <sharafzaz121@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

package org.krita.android;

import static android.os.Process.killProcess;
import static android.os.Process.myPid;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Intent;
import android.os.Build;
import android.os.IBinder;
import android.util.Log;

import org.krita.R;

public class DocumentSaverService extends Service {

    private final String TAG = "DocumentSaverService";
    private final String CHANNEL_ID = "org.krita.android";
    private final int NOTIFICATION_ID = 3;

    public static final String START_SAVING = "START_SAVING";
    public static final String KILL_PROCESS = "KILL_PROCESS";
    private static final String CANCEL_SAVING = "CANCEL_SAVING";

    private boolean mKillProcess = false;
    private Thread mDocSaverThread;

    @Override
    public void onCreate() {
        super.onCreate();
        Log.i(TAG, "Creating");

        Intent intent = new Intent(this, DocumentSaverService.class);
        intent.putExtra(CANCEL_SAVING, true);
        PendingIntent cancelPendingIntent = PendingIntent.getService(this, 0,
                intent, 0);

        createNotificationChannel();

        Notification notification;
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

        startForeground(NOTIFICATION_ID, notification);
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

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.i(TAG, "Starting");

        if (intent.getBooleanExtra(START_SAVING, false)) {
            Log.i(TAG, "Starting Auto Save");
            // let's not block the Android UI thread if we return to the app quickly
            mDocSaverThread = new Thread(new Runnable() {
                @Override
                public void run() {
                    JNIWrappers.saveState();
                    stopSelf();
                }
            });
            mDocSaverThread.start();
        } else if (intent.getBooleanExtra(KILL_PROCESS, false)) {
            mKillProcess = true;
            new Thread(new Runnable() {
                @Override
                public void run() {
                    waitForSaving();
                    // if saving finished a long time ago, we will have to kill the app
                    stopSelf();
                }
            }).start();
        } else if (intent.getBooleanExtra(CANCEL_SAVING, false)) {
            // without this Android will think we crashed
            stopSelf();

            // TODO: Think about atomicity, which Qt doesn't support for Android
            killProcess(myPid());
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

    private void waitForSaving() {
        if (mDocSaverThread != null) {
            try {
                mDocSaverThread.join();
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
