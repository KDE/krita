// SPDX-License-Identifier: GPL-3.0-or-later
package org.krita.android;

import android.app.Activity;
import android.graphics.drawable.Drawable;
import android.util.Log;

import com.android.billingclient.api.ProductDetails;

import org.krita.R;

import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.function.Predicate;
import java.util.stream.Collectors;

public final class DonationProduct {
    private static final String TAG = "krita.DonationProduct";

    private static List<DonationProduct> allProducts = null;

    private final long mFlag;
    private final String mId;
    private final String mTitle;
    private final String mDescription;
    private final Drawable mIcon;
    private final boolean mSubscription;
    private final boolean mAvailable;

    private DonationProduct(long flag, String id, String title, String description, Drawable icon, boolean subscription, boolean available) {
        mFlag = flag;
        mId = id;
        mTitle = title;
        mDescription = description;
        mIcon = icon;
        mSubscription = subscription;
        mAvailable = available;
    }

    public long getFlag() {
        return mFlag;
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

    public Drawable getIcon() {
        return mIcon;
    }

    public boolean isSubscription() {
        return mSubscription;
    }

    public boolean isAvailable() {
        return mAvailable;
    }

    public static void initAllProducts(Activity activity) {
        if (allProducts == null) {
            allProducts = List.of(
                    new DonationProduct(
                            1L << 1L,
                            "kritasupportersubscription1",
                            activity.getString(R.string.product_subscription_title),
                            activity.getString(R.string.product_subscription_description, 8, 150),
                            activity.getDrawable(R.drawable.product_subscription),
                            true,
                            true),
                    new DonationProduct(
                            1L << 2L,
                            "krita5x3x0supporterbundle",
                            activity.getString(R.string.product_onetime_title),
                            activity.getString(R.string.product_onetime_description, 8, 150),
                            activity.getDrawable(R.drawable.product_onetime),
                            false,
                            true),
                    new DonationProduct(
                            1L,
                            "thankyoukiki",
                            activity.getString(R.string.product_supporter_badge_title),
                            activity.getString(R.string.product_supporter_badge_description),
                            null,
                            false,
                            false));
        }
    }

    public static long getOwnedProductFlags(Set<String> ownedProductIds) {
        return allProducts.stream()
                .filter((DonationProduct dp) -> ownedProductIds.contains(dp.getId()))
                .map(DonationProduct::getFlag)
                .reduce(0L, (Long a, Long b) -> a | b);
    }

    public static List<DonationProduct> getAvailableProducts() {
        return allProducts.stream()
                .filter(DonationProduct::isAvailable)
                .collect(Collectors.toList());
    }

    public static Set<String> getAllProductIdSet() {
        return allProducts.stream()
                .map(DonationProduct::getId)
                .collect(Collectors.toSet());
    }

    public static List<String> getAvailableOneTimeProductIds() {
        return allProducts.stream()
                .filter(DonationProduct::isAvailable)
                .filter(Predicate.not(DonationProduct::isSubscription))
                .map(DonationProduct::getId)
                .collect(Collectors.toList());
    }

    public static List<String> getAvailableSubscriptionProductIds() {
        return allProducts.stream()
                .filter(DonationProduct::isAvailable)
                .filter(DonationProduct::isSubscription)
                .map(DonationProduct::getId)
                .collect(Collectors.toList());
    }

    public static List<KisSupporterProduct> getKisSupporterProducts(
            Activity activity, Map<String, ProductDetails> availableProductDetailsById, Set<String> ownedProductIds) {
        return allProducts.stream()
                .map((DonationProduct p) -> {
                    String buttonText = null;
                    String offerToken = null;

                    int availability = p.getKisSupporterProductAvailability(availableProductDetailsById, ownedProductIds);
                    if (availability == KisSupporterProduct.AVAILABILITY_AVAILABLE) {
                        DonationOffer offer;
                        try {
                            offer = DonationOffer.fromProductDetails(activity, availableProductDetailsById.get(p.getId()));
                        } catch (Exception e) {
                            Log.w(TAG, "Exception retrieving offer for " + p.getId(), e);
                            offer = null;
                        }

                        if (offer == null) {
                            Log.w(TAG, "No offer for available product " + p.getId());
                            availability = KisSupporterProduct.AVAILABILITY_MISSING;
                        } else {
                            buttonText = offer.getButtonText();
                            offerToken = offer.getOfferToken();
                        }
                    }

                    return new KisSupporterProduct(
                            p.getKisSupporterProductType(),
                            availability,
                            p.getId(),
                            p.getTitle(),
                            p.getDescription(),
                            p.getIcon(),
                            buttonText,
                            offerToken);
                })
                .collect(Collectors.toList());
    }

    private int getKisSupporterProductType() {
        if (mSubscription) {
            return KisSupporterProduct.TYPE_SUBSCRIPTION;
        } else {
            return KisSupporterProduct.TYPE_ONE_TIME;
        }
    }

    private int getKisSupporterProductAvailability(Map<String, ProductDetails> availableProductsById, Set<String> ownedProductIds) {
        if (ownedProductIds != null && ownedProductIds.contains(mId)) {
            return KisSupporterProduct.AVAILABILITY_OWNED;
        } else if (mAvailable && availableProductsById.containsKey(mId)) {
            return KisSupporterProduct.AVAILABILITY_AVAILABLE;
        } else {
            return KisSupporterProduct.AVAILABILITY_MISSING;
        }
    }
}
