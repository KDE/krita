// SPDX-License-Identifier: GPL-3.0-or-later
package org.krita.android;

import android.app.Activity;
import android.app.AlertDialog;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.SeekBar;
import android.widget.TextView;

import org.krita.R;

public final class ScalingDialog {

    private static final String TAG = "krita.ScalingDialog";
    private static int SCALING_STEP_SIZE_PERCENT = 5;

    private final Activity mActivity;
    private final int mDefaultProgress;
    private final View mView;
    private final AlertDialog mAlertDialog;

    public ScalingDialog(Activity activity, double currentScale, double defaultScale, boolean showOnStartup, boolean canShowOnStartup) {
        mActivity = activity;
        mDefaultProgress = scaleToProgress(defaultScale);
        mView = activity.getLayoutInflater().inflate(R.layout.scaling_dialog_layout, null);
        mAlertDialog = new AlertDialog.Builder(activity).setView(mView).setCancelable(false).create();

        SeekBar seekBar = mView.findViewById(R.id.seekBar);
        double maximumScale = Math.max(4.0, Math.ceil(defaultScale * 2.0));
        seekBar.setMax((int) Math.round((maximumScale - 1.0) * 100.0 / SCALING_STEP_SIZE_PERCENT));
        seekBar.setProgress(scaleToProgress(currentScale));
        seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                updatePercentText(progress);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                // Nothing.
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                int progress = seekBar.getProgress();
                double scale;
                if (progress == mDefaultProgress) {
                    scale = defaultScale;
                } else {
                    scale = progressToScale(progress);
                }
                JNIWrappers.setPrimaryScreenScale(scale);
            }
        });

        CheckBox rememberCheckBox = mView.findViewById(R.id.rememberCheckBox);
        rememberCheckBox.setChecked(showOnStartup);
        if (!canShowOnStartup) {
            rememberCheckBox.setVisibility(View.GONE);
        }

        Button okButton = mView.findViewById(R.id.okButton);
        okButton.setOnClickListener(v -> {
            mAlertDialog.dismiss();
            JNIWrappers.savePrimaryScreenScale(rememberCheckBox.isChecked());
        });
        updatePercentText(seekBar.getProgress());

        mAlertDialog.setOnShowListener(dialogInterface -> JNIWrappers.onScalingDialogShown());
        mAlertDialog.setOnDismissListener(dialogInterface -> JNIWrappers.onScalingDialogDismissed());
    }

    public void show() {
        mAlertDialog.show();
        Window window = mAlertDialog.getWindow();
        if (window != null) {
            window.clearFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
        }
    }

    private void updatePercentText(int progress) {
        int resId;
        if (progress == mDefaultProgress) {
            resId = R.string.interface_scale_default;
        } else {
            resId = R.string.interface_scale;
        }
        TextView percentText = mView.findViewById(R.id.percentText);
        percentText.setText(mActivity.getString(resId, progressToPercent(progress)));
    }

    private static int scaleToProgress(double scale) {
        return (int) Math.round((scale - 1.0) * 100.0 / SCALING_STEP_SIZE_PERCENT);
    }

    private static int progressToPercent(int progress) {
        return 100 + (progress * SCALING_STEP_SIZE_PERCENT);
    }

    private static double progressToScale(int progress) {
        return ((double) progressToPercent(progress)) / 100.0;
    }
}
