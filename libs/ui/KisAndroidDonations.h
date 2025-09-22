/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef __KISANDROIDDONATIONS_H_
#define __KISANDROIDDONATIONS_H_

#include <QObject>

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
        // User purchased the (legacy) lifetime supporter badge.
        LifetimeSupporter = 4,
    };

    static KisAndroidDonations *instance();

    bool shouldShowDonationLink() const;
    bool shouldShowSupporterBadge() const;

public Q_SLOTS:
    void slotStartDonationFlow();

private Q_SLOTS:
    void slotUpdateState(int state);

Q_SIGNALS:
    void sigStateUpdateReceived(int state);
    void sigStateChanged();

private:
    explicit KisAndroidDonations(QObject *parent = nullptr);

    void syncState();

    State m_state{State::Unknown};
};

#endif // __KISANDROIDDONATIONS_H
