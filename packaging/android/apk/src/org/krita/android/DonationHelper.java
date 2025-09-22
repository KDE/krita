/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2022 Sharaf Zaman <shzam@sdf.org>
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
import com.android.billingclient.api.BillingFlowParams.ProductDetailsParams;
import com.android.billingclient.api.BillingResult;
import com.android.billingclient.api.PendingPurchasesParams;
import com.android.billingclient.api.ProductDetails;
import com.android.billingclient.api.ProductDetailsResponseListener;
import com.android.billingclient.api.Purchase;
import com.android.billingclient.api.PurchasesResponseListener;
import com.android.billingclient.api.PurchasesUpdatedListener;
import com.android.billingclient.api.QueryProductDetailsParams;
import com.android.billingclient.api.QueryProductDetailsResult;
import com.android.billingclient.api.QueryPurchasesParams;
import com.android.billingclient.api.UnfetchedProduct;

import org.krita.R;
import org.qtproject.qt5.android.QtNative;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

// Note that for testing this stuff, your Google account needs to be added to
// the "License Testers" in the Play Console and you need to test with a
// "krita.org" package, not "krita.org.debug". To change the package name, edit
// build.gradle and comment out the line `applicationIdSuffix ".debug"`.
@SuppressWarnings("unused")
public class DonationHelper implements PurchasesUpdatedListener, BillingClientStateListener, ProductDetailsResponseListener {

    private static final String TAG = "krita.DonationHelper";

    private static final String PRODUCT_ID_LIFETIME_SUPPORTER = "thankyoukiki";
    private static final List<String> ALL_PRODUCT_IDS = List.of(
            PRODUCT_ID_LIFETIME_SUPPORTER);

    // Keep this in sync with the enum clas State in KisAndroidDonations.h!
    private static final int STATE_UNKNOWN = 0;
    private static final int STATE_CHECKING = 1;
    private static final int STATE_UNAVAILABLE = 2;
    private static final int STATE_NOSUPPORT = 3;
    private static final int STATE_LIFETIMESUPPORTER = 4;

    private final BillingClient mBillingClient;
    private List<ProductDetails> mProductDetailsList;
    private Set<String> mOwnedProductIds = new HashSet<String>();
    private boolean mProductsQueryInProgress = false;
    private boolean mSync = false;

    private static DonationHelper sInstance;

    private DonationHelper() {
        mBillingClient = BillingClient.newBuilder(QtNative.getContext())
            .setListener(this)
            .enableAutoServiceReconnection()
            .enablePendingPurchases(
                PendingPurchasesParams.newBuilder()
                    .enableOneTimeProducts()
                    .build())
            .build();
        mBillingClient.startConnection(this);
    }

    public static synchronized DonationHelper getInstance() {
        if (sInstance == null) {
            Log.d(TAG, "Creating instance");
            sInstance = new DonationHelper();
        }
        return sInstance;
    }

    @Override
    public void onBillingSetupFinished(BillingResult billingResult) {
        Log.d(TAG, "Billing setup finished: " + billingResult);
        if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK) {
            refreshPurchases();
        } else {
            Log.d(TAG, "Billing setup not successful, not refreshing purchases");
            sendDonationStateUpdate();
        }
    }

    private void refreshPurchases() {
        try {
            QueryPurchasesParams params = QueryPurchasesParams.newBuilder()
                .setProductType(BillingClient.ProductType.INAPP)
                .build();
            Log.d(TAG, "Querying purchases");
            mBillingClient.queryPurchasesAsync(params,
                    new PurchasesResponseListener() {
                        @Override
                        public void onQueryPurchasesResponse(BillingResult billingResult, List<Purchase> purchases) {
                            Log.d(TAG, "Badge check result: " + billingResult);
                            if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK) {
                                mOwnedProductIds.clear();
                                if (handlePurchases(purchases)) {
                                    Log.d(TAG, "Badge purchased or pending");
                                } else {
                                    Log.w(TAG, "No badge purchased");
                                }
                            }
                            sendDonationStateUpdate();
                            refreshProductDetails();
                        }
                    });
        } catch (Exception e) {
            Log.e(TAG, "Exception refreshing purchases", e);
            sendDonationStateUpdate();
        }
    }

    private void refreshProductDetails() {
        mProductsQueryInProgress = true;
        try {
            List<QueryProductDetailsParams.Product> productList = new ArrayList<>();
            // Querying for only owned products just produces an error, so filter those.
            for (String productId : ALL_PRODUCT_IDS) {
                if(mOwnedProductIds.contains(productId)) {
                    Log.d(TAG, "Not querying for owned product " + productId);
                } else {
                    Log.d(TAG, "Querying for product " + productId);
                    productList.add(QueryProductDetailsParams.Product.newBuilder()
                        .setProductId(productId)
                        .setProductType(BillingClient.ProductType.INAPP)
                        .build());
                }
            }

            if (productList.isEmpty()) {
                Log.d(TAG, "No products to query for");
                mProductsQueryInProgress = false;
            } else {
                QueryProductDetailsParams params = QueryProductDetailsParams.newBuilder()
                    .setProductList(productList)
                    .build();
                Log.d(TAG, "Query product details");
                mBillingClient.queryProductDetailsAsync(params, this);
            }
        } catch (Exception e) {
            Log.e(TAG, "Exception refreshing product details", e);
            mProductsQueryInProgress = false;
        }
    }

    @Override
    public void onProductDetailsResponse(BillingResult billingResult, QueryProductDetailsResult queryProductDetailsResult) {
        Log.d(TAG, "onProductDetailsResponse: " + billingResult);
        mProductsQueryInProgress = false;
        if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK) {
            mProductDetailsList = queryProductDetailsResult.getProductDetailsList();
            Log.d(TAG, "Fetched: " + mProductDetailsList);
            List<UnfetchedProduct> unfetchedProductList = queryProductDetailsResult.getUnfetchedProductList();
            Log.d(TAG, "Unfetched: " + unfetchedProductList);
        }
        sendDonationStateUpdate();
    }

    @Override
    public void onBillingServiceDisconnected() {
        Log.d(TAG, "onBillingServiceDisconnected");
    }

    @Override
    public void onPurchasesUpdated(BillingResult billingResult, List<Purchase> purchases) {
        Log.d(TAG, "onPurchasesUpdated: " + billingResult);
        switch (billingResult.getResponseCode()) {
            case BillingClient.BillingResponseCode.OK:
                handlePurchases(purchases);
                break;
            case BillingClient.BillingResponseCode.ITEM_ALREADY_OWNED:
                refreshPurchases();
                break;
            case BillingClient.BillingResponseCode.USER_CANCELED:
                showToast(R.string.cancelled);
                break;
            default:
                showToast(R.string.something_wrong);
                break;
        }
    }

    private boolean handlePurchases(List<Purchase> purchases) {
        boolean handled = false;
        if (purchases == null || purchases.isEmpty()) {
            Log.d(TAG, "No purchases to handle");
        } else {
            Log.d(TAG, "Handling purchases " + purchases);
            for (Purchase purchase: purchases) {
                try {
                    if (handlePurchase(purchase)) {
                        handled = true;
                    }
                } catch (Exception e) {
                    Log.e(TAG, "Error handling purchase " + purchase, e);
                }
            }
        }
        return handled;
    }

    private boolean handlePurchase(Purchase purchase) {
        Set<String> productIds = new HashSet<String>(ALL_PRODUCT_IDS);
        productIds.retainAll(purchase.getProducts());
        if(productIds.isEmpty()) {
            Log.w(TAG, "No products identified in " + purchase);
            return false;
        }

        switch(purchase.getPurchaseState()) {
            case Purchase.PurchaseState.PURCHASED:
                Log.d(TAG, "Purchased " + productIds + " with " + purchase);
                addOwnedProductIds(productIds);
                acknowledgePurchase(purchase);
                return true;
            case Purchase.PurchaseState.PENDING:
                // Pending means the payment is still processing and could
                // conceivably be declined in the future. We'll just treat this
                // as a successful purchase, it's not worth the extra code.
                Log.d(TAG, "Pending " + productIds + " with " + purchase);
                addOwnedProductIds(productIds);
                return true;
            default:
                // Ostensibly there's only an UNSPECIFIED_STATE left.
                Log.d(TAG, "Unhandled " + productIds + " with " + purchase);
                return false;
        }
    }

    private void addOwnedProductIds(Set<String> productIds) {
        mOwnedProductIds.addAll(productIds);
        sendDonationStateUpdate();
    }

    private void acknowledgePurchase(Purchase purchase) {
        if (purchase.isAcknowledged()) {
            Log.d(TAG, "Purchase already acknowledged");
            /////////////////////////////////////////////////////////////////////
            // Only uncomment for development, do not let this get released!
            // This will "consume" the product, meaning the user will not have
            // it anymore and it will become available for purchase again.
            /////////////////////////////////////////////////////////////////////
            // Log.w(TAG, "Developer consuming purchase " + purchaseToken);
            // mBillingClient.consumeAsync(
            //     com.android.billingclient.api.ConsumeParams.newBuilder()
            //         .setPurchaseToken(purchase.getPurchaseToken())
            //         .build(),
            //     new com.android.billingclient.api.ConsumeResponseListener() {
            //         @Override
            //         public void onConsumeResponse(BillingResult billingResult, String purchaseToken) {
            //             Log.d(TAG, "Developer consumed '" + purchaseToken + "':" + billingResult);
            //             refreshPurchases();
            //         }
            //     });
        } else {
            try {
                AcknowledgePurchaseParams params = AcknowledgePurchaseParams
                    .newBuilder()
                    .setPurchaseToken(purchase.getPurchaseToken())
                    .build();
                Log.d(TAG, "Acknowledging purchase");
                mBillingClient.acknowledgePurchase(params, new AcknowledgePurchaseResponseListener() {
                    @Override
                    public void onAcknowledgePurchaseResponse(BillingResult billingResult) {
                        Log.d(TAG, "Acknowledge purchase result: " + billingResult);
                    }
                });
            } catch (Exception e) {
                Log.e(TAG, "Exception acknowledging purchase", e);
            }
        }
    }

    // Called from native code by KisAndroidDonations.
    public static void startBillingFlow() {
        Log.d(TAG, "startBillingFlow");
        try {
            getInstance().startBillingFlowInternal();
        } catch (Exception e) {
            Log.e(TAG, "Exception starting billing flow", e);
            showToast(R.string.something_wrong);
        }
    }

    private void startBillingFlowInternal() {
        if (anyProductsAvailable()) {
            List<ProductDetailsParams> productDetailsParamsList = new ArrayList<>();
            for (ProductDetails details : mProductDetailsList) {
                productDetailsParamsList.add(ProductDetailsParams.newBuilder()
                        .setProductDetails(details)
                        .build());
            }

            BillingFlowParams params = BillingFlowParams.newBuilder()
                    .setProductDetailsParamsList(productDetailsParamsList)
                    .build();

            Log.d(TAG, "Launch billing flow");
            mBillingClient.launchBillingFlow(QtNative.activity(), params);
        } else {
            Log.e(TAG, "No products available to launch billing flow with");
            showToast(R.string.something_wrong);
        }
    }

    boolean anyProductsAvailable() {
        if (mProductDetailsList != null) {
            for (ProductDetails productDetails : mProductDetailsList) {
                if (ALL_PRODUCT_IDS.contains(productDetails.getProductId())) {
                    return true;
                }
            }
        }
        return false;
    }

    private static void showToast(final int resourceId) {
        try{
            QtNative.activity().runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    try {
                        Toast.makeText(QtNative.getContext(), resourceId, Toast.LENGTH_LONG).show();
                    } catch (Exception|UnsatisfiedLinkError e) {
                        Log.e(TAG, "Exception showing toast with resource id " + resourceId, e);
                    }
                }
            });
        } catch (Exception|UnsatisfiedLinkError e) {
            Log.e(TAG, "Exception dispatching toast with resource id " + resourceId, e);
        }
    }

    // Called from native code by KisAndroidDonations.
    public static void syncState() {
        Log.d(TAG, "syncState");
        getInstance().syncStateInternal();
    }

    private void syncStateInternal() {
        enableSync();
        sendDonationStateUpdate();
    }

    private synchronized void enableSync() {
        if (mSync) {
            Log.d(TAG, "Already synchonized");
        } else {
            Log.d(TAG, "Enabling synchronization");
            mSync = true;
        }
    }

    private synchronized void sendDonationStateUpdate() {
        int state = getState();
        if (mSync) {
            Log.d(TAG, "Sending donation state " + state);
            try {
                JNIWrappers.donationStateUpdated(state);
            } catch (Exception|UnsatisfiedLinkError e) {
                Log.e(TAG, "Exception sending donation state update with state " + state, e);
            }
        } else {
            Log.d(TAG, "Not synchronized, not sending donation state " + state);
        }
    }

    private int getState() {
        if (mOwnedProductIds.contains(PRODUCT_ID_LIFETIME_SUPPORTER)) {
            return STATE_LIFETIMESUPPORTER;
        } else if (anyProductsAvailable()) {
            return STATE_NOSUPPORT;
        } else if (mProductsQueryInProgress) {
            return STATE_CHECKING;
        } else {
            return STATE_UNAVAILABLE;
        }
    }
}
