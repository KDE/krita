/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2020 Sharaf Zaman <sharafzaz121@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

package org.krita.android;

import android.util.Log;
import android.widget.Toast;

import com.android.billingclient.api.AcknowledgePurchaseParams;
import com.android.billingclient.api.AcknowledgePurchaseResponseListener;
import com.android.billingclient.api.BillingClient;
import com.android.billingclient.api.BillingClientStateListener;
import com.android.billingclient.api.BillingFlowParams;
import com.android.billingclient.api.BillingResult;
import com.android.billingclient.api.Purchase;
import com.android.billingclient.api.PurchasesUpdatedListener;
import com.android.billingclient.api.SkuDetails;
import com.android.billingclient.api.SkuDetailsParams;
import com.android.billingclient.api.SkuDetailsResponseListener;

import org.krita.R;
import org.qtproject.qt5.android.QtNative;

import java.util.ArrayList;
import java.util.List;

public class DonationHelper implements PurchasesUpdatedListener, BillingClientStateListener, SkuDetailsResponseListener {

    private final String LOG_TAG = "krita.DonationHelper";

    private BillingClient mBillingClient;
    private List<SkuDetails> mSkuDetails;

    private static DonationHelper sInstance;

    private DonationHelper() {

        mBillingClient = BillingClient.newBuilder(QtNative.getContext())
                                      .setListener(this)
                                      .enablePendingPurchases()
                                      .build();
        mBillingClient.startConnection(this);
    }

    public static DonationHelper getInstance() {
        if (sInstance == null) {
            sInstance = new DonationHelper();
        }
        return sInstance;
    }

    @Override
    public void onBillingSetupFinished(BillingResult billingResult) {
        if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK) {
            querySkuDetails();
        }
    }

    private void querySkuDetails() {
        List<String> skus = new ArrayList<>();
        skus.add("thankyoukiki");

        SkuDetailsParams params = SkuDetailsParams.newBuilder()
          .setType(BillingClient.SkuType.INAPP)
          .setSkusList(skus)
          .build();

        mBillingClient.querySkuDetailsAsync(params, this);
    }

    @Override
    public void onSkuDetailsResponse(BillingResult billingResult, List<SkuDetails> list) {
        if (billingResult == null) {
            Log.e(LOG_TAG, "null billingResult");
            return;
        }

        if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK) {
            if (list != null) {
                mSkuDetails = list;
            }
        }
    }

    @Override
    public void onBillingServiceDisconnected() {

    }


    @Override
    public void onPurchasesUpdated(BillingResult billingResult, List<Purchase> purchases) {
        if (billingResult == null) {
            Log.e(LOG_TAG, "null billingResult");
            return;
        }
        switch (billingResult.getResponseCode()) {
            case BillingClient.BillingResponseCode.OK:
                // only one item, for now
                for (Purchase purchase: purchases) {
                    handlePurchase(purchase);
                }
                break;

            case BillingClient.BillingResponseCode.ITEM_ALREADY_OWNED:
                // this shouldn't happen with our current logic!
                Log.w(LOG_TAG, "Item already owned");
                JNIWrappers.donationSuccessful();
                break;

            case BillingClient.BillingResponseCode.DEVELOPER_ERROR:
                Log.e(LOG_TAG, "Dev Error: " + billingResult.getDebugMessage());
                break;

            case BillingClient.BillingResponseCode.USER_CANCELED:
                showToast(R.string.cancelled);
                break;

            default:
                showToast(R.string.something_wrong);
        }
    }

    private void handlePurchase(Purchase purchase) {
        if (purchase.getPurchaseState() == Purchase.PurchaseState.PURCHASED) {
            ackPurchase(purchase);
            JNIWrappers.donationSuccessful();
        }
    }

    private void ackPurchase(Purchase purchase) {
        AcknowledgePurchaseParams params = AcknowledgePurchaseParams
                                                 .newBuilder()
                                                 .setPurchaseToken(purchase.getPurchaseToken())
                                                 .build();
        mBillingClient.acknowledgePurchase(params, new AcknowledgePurchaseResponseListener() {
            @Override
            public void onAcknowledgePurchaseResponse(BillingResult billingResult) {
                Log.d(LOG_TAG, "BillingResult: " + billingResult.getResponseCode());
            }
        });
    }

    private static void showToast(final int resourceId) {
        QtNative.activity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(QtNative.getContext(), resourceId, Toast.LENGTH_LONG).show();
            }
        });
    }

    public static void startBillingFlow() {
        if (!getInstance().mBillingClient.isReady()) {
            getInstance().mBillingClient.startConnection(sInstance);
            showToast(R.string.something_wrong);
            return;
        }

        if (getInstance().mSkuDetails != null) {
            // there's only one for nwo
            for (SkuDetails detail: getInstance().mSkuDetails) {
                BillingFlowParams flowParams = BillingFlowParams.newBuilder()
                  .setSkuDetails(detail)
                  .build();

                getInstance().mBillingClient.launchBillingFlow(QtNative.activity(), flowParams);
            }
        }
    }

    // This method will be called from C++ side, to see if the banner has been purchased.
    // We only have one item right now, so this will do.
    public static boolean isBadgePurchased() {
        Purchase.PurchasesResult purchasesResult =
          getInstance().mBillingClient.queryPurchases(BillingClient.SkuType.INAPP);

        if (purchasesResult.getPurchasesList() != null)
            return !purchasesResult.getPurchasesList().isEmpty();
        else
            return false;
    }

    public static void endConnection() {
        getInstance().mBillingClient.endConnection();
        sInstance = null;
    }
}
