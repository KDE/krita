// SPDX-License-Identifier: GPL-3.0-or-later
package org.krita.android;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Bitmap;
import android.graphics.drawable.Drawable;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.ViewAnimator;

import com.android.billingclient.api.ProductDetails;

import org.krita.R;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

public class DonationDialog {

    private static final String TAG = "krita.DonationDialog";
    private static final String PREF_LAST_TIMESTAMP = "donationdialogautotimestamp";
    private static final String PREF_LAST_SHOWN = "donationdialogautoshown";

    private static final long DAY_IN_SECONDS = 24L * 60L * 60L;
    // How many days to wait between automatically showing the support options on the splash screen.
    private static final long AUTO_SHOW_DELAY = 5L * DAY_IN_SECONDS;

    private static final int PAGE_SPLASH = 0;
    private static final int PAGE_PRODUCT_LIST = 1;
    private static final int PAGE_PURCHASE_CONFIRMATION = 2;

    private final Activity mActivity;
    private final View mView;
    private final AlertDialog mAlertDialog;
    private final boolean mSplash;
    private boolean mLoading;
    private boolean mAutoClose;
    private boolean mHaveSplashImage = false;
    private int mPage;
    private List<ProductDetails> mProductDetails = null;
    private boolean mAnyProductsOwned = false;
    private boolean mAutoShowSet = false;
    private boolean mShouldAutoShow = false;
    private boolean mFirstStartup = false;

    public DonationDialog(Activity activity, boolean splash) {
        mActivity = activity;
        mSplash = splash;
        mLoading = splash;
        mAutoClose = splash;
        mPage = splash ? PAGE_SPLASH : PAGE_PRODUCT_LIST;
        mView = activity.getLayoutInflater().inflate(R.layout.donation_dialog_layout, null);

        mAlertDialog = new AlertDialog.Builder(activity)
                .setView(mView)
                .create();

        getBackButton().setOnClickListener(v -> showSplashPage());
        getHeaderButton().setOnClickListener(v -> showProductListPage());
        getDismissButton().setOnClickListener(v -> mAlertDialog.dismiss());

        updateState();
    }

    public boolean isSplash() {
        return mSplash;
    }

    public AlertDialog getAlertDialog() {
        return mAlertDialog;
    }

    public Button getBackButton() {
        return mView.findViewById(R.id.backButton);
    }

    public Button getHeaderButton() {
        return mView.findViewById(R.id.headerButton);
    }

    public ImageView getSplashImage() {
        return mView.findViewById(R.id.splashImage);
    }

    public TextView getSplashText() {
        return mView.findViewById(R.id.splashText);
    }

    public ViewAnimator getBottomViews() {
        return mView.findViewById(R.id.bottomViews);
    }

    public Button getDismissButton() {
        return mView.findViewById(R.id.dismissButton);
    }

    public TextView getLoadingText() {
        return mView.findViewById(R.id.loadingText);
    }

    public void setLoading(boolean loading) {
        if (loading != mLoading) {
            mLoading = loading;
            updateState();
            if (!loading && mAutoClose) {
                mAlertDialog.dismiss();
            }
        }
    }

    public void showSplashPage() {
        if (mLoading && mSplash) {
            mAutoClose = true;
            mPage = PAGE_SPLASH;
            updateState();
        }
    }

    public void showProductListPage() {
        mAutoClose = false;
        mPage = PAGE_PRODUCT_LIST;
        updateState();
    }

    public void showPurchaseConfirmationPage() {
        mAutoClose = false;
        mPage = PAGE_PURCHASE_CONFIRMATION;
        updateState();
    }

    public void setLoadingText(String text) {
        getLoadingText().setText(text);
    }

    public void setSplashContents(Bitmap bitmap, String text) {
        mHaveSplashImage = bitmap != null;
        getSplashImage().setImageBitmap(bitmap);
        getSplashText().setText(text);
        updateState();
    }

    public void setProductDetails(List<ProductDetails> productDetails, boolean anyProductsOwned) {
        if (!Objects.equals(productDetails, mProductDetails) || anyProductsOwned != mAnyProductsOwned) {
            mProductDetails = productDetails;
            mAnyProductsOwned = anyProductsOwned;
            updateProducts();
            if (checkAutoShowProducts()) {
                mPage = PAGE_PRODUCT_LIST;
                mAutoClose = false;
            }
            updateState();
        }
    }

    private boolean checkAutoShowProducts() {
        if (mPage != PAGE_SPLASH) {
            Log.d(TAG, "checkAutoShowProducts: not on splash screen");
            return false;
        }

        if (mAnyProductsOwned) {
            Log.d(TAG, "checkAutoShowProducts: already owns a product");
            return false;
        }

        if (mProductDetails == null) {
            Log.d(TAG, "checkAutoShowProducts: no product details");
            return false;
        }

        if (mAutoShowSet) {
            Log.d(TAG, "checkAutoShowProducts: already set to " + mShouldAutoShow);
            return mShouldAutoShow;
        }

        SharedPreferences prefs = mActivity.getPreferences(Context.MODE_PRIVATE);
        long lastAutoShowTimestamp = prefs.getLong(PREF_LAST_TIMESTAMP, -1L);
        boolean lastShown = prefs.getBoolean(PREF_LAST_SHOWN, false);
        mFirstStartup = lastAutoShowTimestamp == -1;
        long currentTimestamp = System.currentTimeMillis() / 1000L;

        // Automatically show the supporter page on the splash screen if enough time has elapsed
        // since the last time it has shown. However, we never automatically show it on first
        // startup and also never show it two times in a row.
        mShouldAutoShow = !mFirstStartup && !lastShown && lastAutoShowTimestamp + AUTO_SHOW_DELAY <= currentTimestamp;
        mAutoShowSet = true;

        Log.d(TAG, "checkAutoShowProducts lastAutoShowTimestamp=" + lastAutoShowTimestamp
                + " lastShown=" + lastShown + " firstStartup=" + mFirstStartup
                + " currentTimestamp=" + currentTimestamp + " shouldAutoShow=" + mShouldAutoShow);

        SharedPreferences.Editor editor = prefs.edit();
        if (mFirstStartup) {
            editor.putLong(PREF_LAST_TIMESTAMP, 0L);
        } else if (mShouldAutoShow) {
            editor.putLong(PREF_LAST_TIMESTAMP, currentTimestamp);
        }
        editor.putBoolean(PREF_LAST_SHOWN, mShouldAutoShow);
        editor.apply();

        return mShouldAutoShow;
    }

    private void updateProducts() {
        List<DonationProductView> products = new ArrayList<>();
        if (mProductDetails != null) {
            for (DonationProduct donationProduct : DonationProduct.getAvailableProducts()) {
                addProduct(products, donationProduct);
            }
        }

        // If no products are available (e.g. because the user doesn't have a phone with Google
        // services on it or this is a Krita Next build), we show donation options instead.
        if (products.isEmpty()) {
            Log.d(TAG, "No products, adding fallback");
            products.addAll(DonationProductView.getFallbackProducts(mActivity));
        }

        LinearLayout productList = mView.findViewById(R.id.productList);
        productList.removeAllViews();
        for (int i = 0, count = products.size(); i < count; ++i) {
            if (i != 0) {
                productList.addView(mActivity.getLayoutInflater().inflate(R.layout.divider_layout, null));
            }
            productList.addView(products.get(i).getView());
        }
    }

    private void addProduct(List<DonationProductView> products, DonationProduct donationProduct) {
        String productId = donationProduct.getId();
        DonationOffer offer = searchOfferById(productId);
        if (offer == null) {
            Log.d(TAG, "Did not find offer for product " + productId);
        } else {
            Log.d(TAG, "Adding offer " + productId + ": " + offer);
            boolean left = products.size() % 2 == 0;
            String description = String.format(
                    "<h2>%s</h2>%s", donationProduct.getTitle(), donationProduct.getDescription());
            String buttonText = offer.getButtonText();
            View.OnClickListener onClick = (View v) -> {
                DonationHelper donationHelper = DonationHelper.getInstance();
                donationHelper.startBillingFlowWith(offer.getProductDetails(), offer.getOfferToken());
            };
            Drawable icon = donationProduct.getIcon();
            products.add(new DonationProductView(
                    mActivity, left ? icon : null, left ? null : icon, description,
                    buttonText, onClick));
        }
    }

    private DonationOffer searchOfferById(String productId) {
        return mProductDetails.stream()
                .filter(pd -> productId.equals(pd.getProductId()))
                .findAny()
                .map(productDetails -> DonationOffer.fromProductDetails(mActivity, productDetails))
                .orElse(null);
    }

    private void updateState() {
        // Adjust which elements are visible depending on whether this
        // is the splash screen or summoned by the user. If this is
        // the splash screen, it may show or hide some elements
        // depending on if the application is still starting up or not.
        Button headerButton = getHeaderButton();
        boolean showHeaderButton = mPage == PAGE_SPLASH && mProductDetails != null
                && !mAnyProductsOwned && !mFirstStartup;
        headerButton.setEnabled(showHeaderButton);
        setViewGone(headerButton, !showHeaderButton);

        getDismissButton().setEnabled(!mLoading);
        mAlertDialog.setCancelable(!mLoading);

        setViewGone(mView.findViewById(R.id.headerTextLayout), mPage == PAGE_SPLASH);
        setViewGone(mView.findViewById(R.id.backButton), !mLoading || !mSplash);
        setViewGone(mView.findViewById(R.id.topDivider), mPage == PAGE_SPLASH);
        setViewGone(mView.findViewById(R.id.splashLayout), mPage != PAGE_SPLASH || !mHaveSplashImage);
        setViewGone(mView.findViewById(R.id.productScroll), mPage != PAGE_PRODUCT_LIST);
        setViewGone(mView.findViewById(R.id.supporterText), mPage != PAGE_PURCHASE_CONFIRMATION);

        ViewAnimator bottomViews = getBottomViews();
        int bottomDisplay = bottomViews.getDisplayedChild();
        if (mLoading && bottomDisplay != 1) {
            bottomViews.setDisplayedChild(1);
        } else if (!mLoading && bottomDisplay != 0) {
            bottomViews.setDisplayedChild(0);
        }
    }

    private static void setViewGone(View view, boolean gone) {
        if (view != null) {
            view.setVisibility(gone ? View.GONE : View.VISIBLE);
        }
    }
}
