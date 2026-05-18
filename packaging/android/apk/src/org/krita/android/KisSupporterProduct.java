// SPDX-License-Identifier: GPL-3.0-or-later
package org.krita.android;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.drawable.Drawable;
import android.util.Log;

// Keep in sync with KisSupporterProduct.h!
public final class KisSupporterProduct {
    private static final String TAG = "krita.KisSupporterProduct";

    public static final int TYPE_ONE_TIME = 0;
    public static final int TYPE_SUBSCRIPTION = 1;
    public static final int AVAILABILITY_MISSING = 0;
    public static final int AVAILABILITY_AVAILABLE = 1;
    public static final int AVAILABILITY_OWNED = 2;

    private final int mType;
    private final int mAvailability;
    private final String mId;
    private final String mTitle;
    private final String mDescription;
    private final Drawable mIcon;
    private final String mButtonText;
    private final String mOfferToken;

    public KisSupporterProduct(
            int type, int availability, String id, String title, String description, Drawable icon,
            String buttonText, String offerToken) {
        mType = type;
        mAvailability = availability;
        mId = id;
        mTitle = title;
        mDescription = description;
        mIcon = icon;
        mButtonText = buttonText;
        mOfferToken = offerToken;
    }

    public int getType() {
        return mType;
    }

    public int getAvailability() {
        return mAvailability;
    }

    public String getId() {
        return mId;
    }

    public String getTitle() {
        return mTitle;
    }

    public String getDescription() {
        return mDescription;
    }

    public int[] getIconPixels(int width, int height) {
        if (width > 0 && height > 0 && mIcon != null) {
            try {
                Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
                Canvas canvas = new Canvas(bitmap);
                mIcon.setBounds(0, 0, canvas.getWidth(), canvas.getHeight());
                mIcon.draw(canvas);
                int[] pixels = new int[width * height];
                bitmap.getPixels(pixels, 0, width, 0, 0, width, height);
                return pixels;
            } catch (Exception e) {
                Log.w(TAG, "Exception getting icon pixels", e);
            }
        }
        return null;
    }

    public String getButtonText() {
        return mButtonText;
    }

    public String getOfferToken() {
        return mOfferToken;
    }
}
