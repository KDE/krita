/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef __KISANDROIDDONATIONS_H_
#define __KISANDROIDDONATIONS_H_

#include <QObject>
#include <QVector>

#include "KisSupporterProduct.h"
#include "kritaui_export.h"

// See DonationHelper.java for notes!
class KRITAUI_EXPORT KisAndroidDonations : public QObject
{
    Q_OBJECT
    friend class KisApplication;

public:
    // Keep this in sync with the STATE_ constants in DonationHelper.java!
    enum class State {
        // Initial or error state. Assume in-app purchases are unavailable.
        Unknown = 0,
        // The billing code is still checking whether any products are available
        // and if the user owns any of them.
        Checking = 1,
        // In-app purchases are unavailable. This may be because of an
        // unexpected error, but may also be an expected condition, e.g. because
        // the user is running a debug or nightly build, which have a different
        // package name and therefore no products assigned to them.
        Unavailable = 2,
        // User does not own any support products, but they could purchase one.
        NoSupport = 3,
        // User owns at least some kind of supporter product.
        Supporter = 4,
    };

    static KisAndroidDonations *instance();

    State state() const { return m_state; }

    bool shouldShowSupporterBadge() const;

    bool isShowDonationManagementDialogPending() const
    {
        return m_showDonationManagementDialogPending;
    }

    void setShowDonationManagementDialogPending(bool pending);

    QVector<KisSupporterProduct> getCurrentProducts() const;

    void startBillingFlowFor(const QString &productId, const QString &offerToken);

    static void setLoaded(bool loaded);
    static void setLoadingText(const QString &text);
    static void showDonationDialog(bool splash);

public Q_SLOTS:
    void slotStartDonationFlow();
    void slotManageSubscriptions();
    void slotManageSubscription(const QString &productId);

private Q_SLOTS:
    void slotUpdateState(int state, long long ownedProductFlags);

Q_SIGNALS:
    void sigStateUpdateReceived(int state, long long ownedProductFlags);
    void sigStateChanged();
    void sigShowDonationManagementDialogRequested();
    void sigSplashDialogDismissed();

private:
    explicit KisAndroidDonations(QObject *parent = nullptr);

    bool isProductOwned(const QString &productId) const;

    void syncState();

    State m_state{State::Unknown};
    long long m_ownedProductFlags{0L};
    bool m_showDonationManagementDialogPending{false};
};

#endif // __KISANDROIDDONATIONS_H
