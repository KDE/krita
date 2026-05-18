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
import com.android.billingclient.api.Purchase;
import com.android.billingclient.api.PurchasesUpdatedListener;

import org.krita.R;
import org.qtproject.qt5.android.QtNative;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;
import java.util.function.Function;
import java.util.stream.Collectors;
import java.util.stream.Stream;

// Note that for testing this stuff, your Google account needs to be added to
// the "License Testers" in the Play Console and you need to test with a
// "krita.org" package, not "krita.org.debug". To change the package name, edit
// build.gradle and comment out the line `applicationIdSuffix ".debug"`.
public class DonationHelper implements PurchasesUpdatedListener, BillingClientStateListener {

    private static final String TAG = "krita.DonationHelper";

    // Keep this in sync with the enum clas State in KisAndroidDonations.h!
    private static final int STATE_UNKNOWN = 0;
    private static final int STATE_CHECKING = 1;
    private static final int STATE_UNAVAILABLE = 2;
    private static final int STATE_NOSUPPORT = 3;
    private static final int STATE_SUPPORTER = 4;

    private final BillingClient mBillingClient;
    private List<ProductDetails> mProductDetailsList;
    private final Set<String> mOwnedProductIds = new HashSet<>();
    private boolean mReady = false;
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

    public BillingClient getBillingClient() {
        return mBillingClient;
    }

    public synchronized boolean isReady() {
        return mReady && !mProductsQueryInProgress;
    }

    public synchronized boolean isAnyProductsOwned() {
        return !mOwnedProductIds.isEmpty();
    }

    public synchronized List<ProductDetails> getProductDetails() {
        if (mProductsQueryInProgress || mProductDetailsList == null || mProductDetailsList.isEmpty()) {
            return List.of();
        } else {
            return new ArrayList<>(mProductDetailsList);
        }
    }

    @Override
    public void onBillingSetupFinished(BillingResult billingResult) {
        Log.d(TAG, "Billing setup finished: " + billingResult);
        if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK) {
            refreshPurchases();
        } else {
            Log.d(TAG, "Billing setup not successful, not refreshing purchases");
            mReady = true;
            sendDonationStateUpdate();
        }
    }

    private void refreshPurchases() {
        mReady = false;
        mProductsQueryInProgress = true;
        DonationQuery.execute(this);
    }

    public void handleFinishedQuery(DonationQuery query) {
        mProductDetailsList = query.getProductDetails();
        mProductsQueryInProgress = false;
        mReady = true;
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
                MainActivity.showPurchaseConfirmation();
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

    public void handlePurchases(List<Purchase> purchases) {
        if (purchases == null || purchases.isEmpty()) {
            Log.d(TAG, "No purchases to handle");
        } else {
            Log.d(TAG, "Handling purchases " + purchases);
            for (Purchase purchase : purchases) {
                try {
                    handlePurchase(purchase);
                } catch (Exception e) {
                    Log.e(TAG, "Error handling purchase " + purchase, e);
                }
            }
        }
    }

    private void handlePurchase(Purchase purchase) {
        Set<String> productIds = DonationProduct.getAllProductIdSet();
        productIds.retainAll(purchase.getProducts());
        if (productIds.isEmpty()) {
            Log.w(TAG, "No products identified in " + purchase);
            return;
        }

        switch (purchase.getPurchaseState()) {
            case Purchase.PurchaseState.PURCHASED:
                Log.d(TAG, "Purchased " + productIds + " with " + purchase);
                addOwnedProductIds(productIds);
                acknowledgePurchase(purchase);
                break;
            case Purchase.PurchaseState.PENDING:
                // Pending means the payment is still processing and could
                // conceivably be declined in the future. We'll just treat this
                // as a successful purchase, it's not worth the extra code.
                Log.d(TAG, "Pending " + productIds + " with " + purchase);
                addOwnedProductIds(productIds);
                break;
            default:
                // Ostensibly there's only an UNSPECIFIED_STATE left.
                Log.d(TAG, "Unhandled " + productIds + " with " + purchase);
                break;
        }
    }

    private void addOwnedProductIds(Set<String> productIds) {
        mOwnedProductIds.addAll(productIds);
        if (mProductDetailsList != null) {
            mProductDetailsList.removeIf(productDetails -> mOwnedProductIds.contains(productDetails.getProductId()));
        }
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
            // Log.w(TAG, "Developer consuming purchase " + purchase.getPurchaseToken());
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

    @SuppressWarnings("unused")
    public static boolean isProductOwned(String productId) {
        DonationHelper donationHelper = getInstance();
        synchronized (donationHelper) {
            return donationHelper.mOwnedProductIds.contains(productId);
        }
    }

    @SuppressWarnings("unused")
    public static void startBillingFlowForProductId(String productId, String offerToken) {
        DonationHelper donationHelper = getInstance();
        ProductDetails productDetails;
        synchronized (donationHelper) {
            productDetails = Optional.ofNullable(donationHelper.mProductDetailsList)
                    .map(List::stream)
                    .orElseGet(Stream::empty)
                    .filter((ProductDetails pd) -> Objects.equals(pd.getProductId(), productId))
                    .findFirst()
                    .orElse(null);
        }
        donationHelper.startBillingFlowWith(productDetails, offerToken);
    }

    public void startBillingFlowWith(ProductDetails productDetails, String offerToken) {
        if (anyProductsAvailable() && productDetails != null) {
            ProductDetailsParams.Builder builder = ProductDetailsParams.newBuilder()
                    .setProductDetails(productDetails);
            if (offerToken != null && !offerToken.isEmpty()) {
                builder.setOfferToken(offerToken);
            }

            BillingFlowParams params = BillingFlowParams.newBuilder()
                    .setProductDetailsParamsList(List.of(builder.build()))
                    .build();

            Log.d(TAG, "Launch billing flow");
            mBillingClient.launchBillingFlow(QtNative.activity(), params);
        } else {
            Log.e(TAG, "No products available to launch billing flow with");
            showToast(R.string.something_wrong);
        }
    }

    public boolean anyProductsAvailable() {
        if (mProductDetailsList != null) {
            Set<String> allProductIds = DonationProduct.getAllProductIdSet();
            for (ProductDetails productDetails : mProductDetailsList) {
                if (allProductIds.contains(productDetails.getProductId())) {
                    return true;
                }
            }
        }
        return false;
    }

    private static void showToast(final int resourceId) {
        try {
            QtNative.activity().runOnUiThread(() -> {
                try {
                    Toast.makeText(QtNative.getContext(), resourceId, Toast.LENGTH_LONG).show();
                } catch (Exception | UnsatisfiedLinkError e) {
                    Log.e(TAG, "Exception showing toast with resource id " + resourceId, e);
                }
            });
        } catch (Exception | UnsatisfiedLinkError e) {
            Log.e(TAG, "Exception dispatching toast with resource id " + resourceId, e);
        }
    }

    @SuppressWarnings("unused")
    public static List<KisSupporterProduct> getCurrentProducts() {
        return getInstance().getCurrentProductsInternal();
    }

    private synchronized List<KisSupporterProduct> getCurrentProductsInternal() {
        if (isReady()) {
            return DonationProduct.getKisSupporterProducts(
                    QtNative.activity(), getAvailableProductsById(), mOwnedProductIds);
        } else {
            return List.of();
        }
    }

    private Map<String, ProductDetails> getAvailableProductsById() {
        if (mProductDetailsList == null) {
            return Map.of();
        } else {
            return mProductDetailsList.stream()
                    .collect(Collectors.toMap(ProductDetails::getProductId, Function.identity()));
        }
    }

    @SuppressWarnings("unused")
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
            long flags = DonationProduct.getOwnedProductFlags(mOwnedProductIds);
            Log.d(TAG, "Sending donation state " + state + " flags " + flags);
            try {
                JNIWrappers.donationStateUpdated(state, flags);
            } catch (Exception | UnsatisfiedLinkError e) {
                Log.e(TAG, "Exception sending donation state update with state " + state + " flags " + flags, e);
            }
        } else {
            Log.d(TAG, "Not synchronized, not sending donation state " + state);
        }
        MainActivity.updateDonationDialogProducts();
    }

    private int getState() {
        if (isAnyProductsOwned()) {
            return STATE_SUPPORTER;
        } else if (anyProductsAvailable()) {
            return STATE_NOSUPPORT;
        } else if (mProductsQueryInProgress) {
            return STATE_CHECKING;
        } else {
            return STATE_UNAVAILABLE;
        }
    }
}
