/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2022 Sharaf Zaman <shzam@sdf.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

package org.krita.android;

import android.util.Log;
import android.widget.Toast;

import androidx.annotation.NonNull;

import com.android.billingclient.api.AcknowledgePurchaseParams;
import com.android.billingclient.api.AcknowledgePurchaseResponseListener;
import com.android.billingclient.api.BillingClient;
import com.android.billingclient.api.BillingClientStateListener;
import com.android.billingclient.api.BillingFlowParams;
import com.android.billingclient.api.BillingFlowParams.ProductDetailsParams;
import com.android.billingclient.api.BillingResult;
import com.android.billingclient.api.ProductDetails;
import com.android.billingclient.api.ProductDetailsResponseListener;
import com.android.billingclient.api.Purchase;
import com.android.billingclient.api.PurchasesResponseListener;
import com.android.billingclient.api.PurchasesUpdatedListener;
import com.android.billingclient.api.QueryProductDetailsParams;
import com.android.billingclient.api.QueryProductDetailsParams.Product;
import com.android.billingclient.api.QueryPurchasesParams;

import org.krita.R;
import org.qtproject.qt5.android.QtNative;

import java.util.ArrayList;
import java.util.List;

@SuppressWarnings("unused")
public class DonationHelper implements PurchasesUpdatedListener, BillingClientStateListener, ProductDetailsResponseListener {

    private final String LOG_TAG = "krita.DonationHelper";

    private final BillingClient mBillingClient;
    private List<ProductDetails> mProductDetailsList;

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
            queryProductDetails();
        }
    }

    private void queryProductDetails() {
        List<QueryProductDetailsParams.Product> productList = new ArrayList<>();
        productList.add(Product.newBuilder()
                .setProductId("thankyoukiki")
                .setProductType(BillingClient.ProductType.INAPP)
                .build());

        QueryProductDetailsParams params = QueryProductDetailsParams.newBuilder()
                .setProductList(productList)
                .build();

        mBillingClient.queryProductDetailsAsync(params, this);
    }

    @Override
    public void onProductDetailsResponse(@NonNull BillingResult billingResult, @NonNull List<ProductDetails> list) {
        if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK) {
            mProductDetailsList = list;
        }
    }

    @Override
    public void onBillingServiceDisconnected() {

    }

    @Override
    public void onPurchasesUpdated(@NonNull BillingResult billingResult, List<Purchase> purchases) {
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
            public void onAcknowledgePurchaseResponse(@NonNull BillingResult billingResult) {
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

        if (getInstance().mProductDetailsList != null) {
            List<ProductDetailsParams> productDetailsParamsList = new ArrayList<>();

            // there's only one for now
            for (ProductDetails details: getInstance().mProductDetailsList) {
                productDetailsParamsList.add(ProductDetailsParams.newBuilder()
                        .setProductDetails(details)
                        .build());
            }
            BillingFlowParams flowParams = BillingFlowParams.newBuilder()
                    .setProductDetailsParamsList(productDetailsParamsList)
                    .build();

            getInstance().mBillingClient.launchBillingFlow(QtNative.activity(), flowParams);
        }
    }

    // This method will be called from C++ side, to see if the banner has been purchased.
    // We only have one item right now, so this will do.
    public static void checkBadgePurchased() {
        getInstance().mBillingClient.queryPurchasesAsync(QueryPurchasesParams.newBuilder()
                        .setProductType(BillingClient.ProductType.INAPP)
                        .build(),
                new PurchasesResponseListener() {
                    @Override
                    public void onQueryPurchasesResponse(@NonNull BillingResult billingResult, @NonNull List<Purchase> list) {
                        if (!list.isEmpty()) {
                            // tell C++ that banner is purchased
                            Log.d("DonationHelper", "Badge purchased");
                            JNIWrappers.donationSuccessful();
                        }
                    }
                });
    }

    public static void endConnection() {
        getInstance().mBillingClient.endConnection();
        sInstance = null;
    }

}
