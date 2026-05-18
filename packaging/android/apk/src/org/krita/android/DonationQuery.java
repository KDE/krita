// SPDX-License-Identifier: GPL-3.0-or-later
package org.krita.android;

import android.util.Log;

import com.android.billingclient.api.BillingClient;
import com.android.billingclient.api.ProductDetails;
import com.android.billingclient.api.Purchase;
import com.android.billingclient.api.QueryProductDetailsParams;
import com.android.billingclient.api.QueryPurchasesParams;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

// Wrapper around the awful interface of the Android billing client. Some undocumented issues:
// * You can't query both one-time and subscription products in one query, it errors.
// * Querying for details about products that the user already owns also errors.
// * Can't query for subscriptions that contain dashes in their IDs, raises invalid ID errors.
// * Can't query for subscriptions that contain underscores in their IDs, just doesn't return them.
//
// Also, this class does all of its queries sequentially. That is intentional, I don't trust the
// billing client to actually be capable of performing two queries at once, considering the numerous
// undocumented failure modes it has. It's fine this way.
public class DonationQuery {

    private static final String TAG = "krita.DonationQuery";

    private final DonationHelper mDonationHelper;
    private final Set<String> mOwnedProductIds = new HashSet<>();
    private final List<ProductDetails> mProductDetails = new ArrayList<>();

    private DonationQuery(DonationHelper donationHelper) {
        mDonationHelper = donationHelper;
    }

    public List<ProductDetails> getProductDetails() {
        return mProductDetails;
    }

    public static void execute(DonationHelper donationHelper) {
        DonationQuery donationQuery = new DonationQuery(donationHelper);
        donationQuery.step1QueryOwnedOneTimeProducts();
    }

    // Part 1: Query for products the user has purchased. We must always do this because inquiring
    // about products that the user already owns raises an (undocumented) error. We also have to do
    // this in two queries, because the API doesn't allow both one-time products and subscriptions.

    private void step1QueryOwnedOneTimeProducts() {
        queryOwnedProducts(BillingClient.ProductType.INAPP, this::step2QueryOwnedSubscriptionProducts);
    }

    private void step2QueryOwnedSubscriptionProducts() {
        queryOwnedProducts(BillingClient.ProductType.SUBS, this::step3QueryOneTimeProductDetails);
    }

    private void queryOwnedProducts(String productType, Runnable next) {
        try {
            QueryPurchasesParams params = QueryPurchasesParams.newBuilder()
                    .setProductType(productType)
                    .build();
            Log.d(TAG, "Querying " + productType + " purchases");
            mDonationHelper.getBillingClient().queryPurchasesAsync(params, (billingResult, purchases) -> {
                Log.d(TAG, "Purchase query " + productType + " result: " + billingResult);
                if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK) {
                    handlePurchases(purchases);
                }
                next.run();
            });
        } catch (Exception e) {
            Log.e(TAG, "Exception querying " + productType + " purchases", e);
            next.run();
        }
    }

    private void handlePurchases(List<Purchase> purchases) {
        mDonationHelper.handlePurchases(purchases);
        for (Purchase purchase : purchases) {
            mOwnedProductIds.addAll(purchase.getProducts());
        }
    }

    // Part 2: Query information about available products, avoiding the forbidden query of
    // already-owned ones. Even though the API seems to allow it, attempting to query both one-time
    // and subscription product raises an (undocumented) error.

    private void step3QueryOneTimeProductDetails() {
        queryProductDetails(
                BillingClient.ProductType.INAPP, DonationProduct.getAvailableOneTimeProductIds(),
                this::step4QuerySubscriptionProductDetails);
    }

    private void step4QuerySubscriptionProductDetails() {
        queryProductDetails(
                BillingClient.ProductType.SUBS, DonationProduct.getAvailableSubscriptionProductIds(),
                this::step5Finish);
    }

    private void queryProductDetails(String productType, List<String> productIds, Runnable next) {
        try {
            List<QueryProductDetailsParams.Product> productList = new ArrayList<>();
            for (String productId : productIds) {
                if (mOwnedProductIds.contains(productId)) {
                    Log.d(TAG, "Not querying for owned " + productType + " product " + productId);
                } else {
                    Log.d(TAG, "Querying for " + productType + " product " + productId);
                    productList.add(QueryProductDetailsParams.Product.newBuilder()
                            .setProductId(productId)
                            .setProductType(productType)
                            .build());
                }
            }

            if (productList.isEmpty()) {
                Log.d(TAG, "No " + productType + "products to query for");
                next.run();
            } else {
                QueryProductDetailsParams params = QueryProductDetailsParams.newBuilder()
                        .setProductList(productList)
                        .build();
                Log.d(TAG, "Querying " + productType + " product details");
                mDonationHelper.getBillingClient().queryProductDetailsAsync(params, (billingResult, queryProductDetailsResult) -> {
                    Log.d(TAG, "Query " + productType + " product details result: " + billingResult);
                    if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK) {
                        List<ProductDetails> productDetails = queryProductDetailsResult.getProductDetailsList();
                        mProductDetails.addAll(productDetails);
                        Log.d(TAG, "Fetched " + productType + " products: " + productDetails);
                        Log.d(TAG, "Unfetched " + productType + " products: " + queryProductDetailsResult.getUnfetchedProductList());
                    }
                    next.run();
                });
            }
        } catch (Exception e) {
            Log.e(TAG, "Exception querying " + productType + " product details", e);
            next.run();
        }
    }

    // Part 3: Report back.

    private void step5Finish() {
        mDonationHelper.handleFinishedQuery(this);
    }
}
