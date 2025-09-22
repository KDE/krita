/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "KisAndroidDonations.h"

#include "KisApplication.h"

#include <QtAndroid>

// See DonationHelper.java for notes!

KisAndroidDonations *KisAndroidDonations::instance()
{
    KisApplication *app = qobject_cast<KisApplication *>(KisApplication::instance());
    if (app) {
        return app->androidDonations();
    } else {
        return nullptr;
    }
}

bool KisAndroidDonations::shouldShowDonationLink() const
{
    // Normally, we'd want to show a donation link when the state is NoSupport.
    // However, currently we only have the lifetime badge as a product and we
    // don't want to continue offering that, so the link is disabled for now.
    return false;
}

bool KisAndroidDonations::shouldShowSupporterBadge() const
{
    return m_state == State::LifetimeSupporter;
}

void KisAndroidDonations::slotStartDonationFlow()
{
    QAndroidJniObject::callStaticMethod<void>("org/krita/android/DonationHelper", "startBillingFlow", "()V");
}

void KisAndroidDonations::slotUpdateState(int state)
{
    if (int(m_state) != state) {
        m_state = State(state);
        Q_EMIT sigStateChanged();
    }
}

KisAndroidDonations::KisAndroidDonations(QObject *parent)
    : QObject(parent)
{
    // The state update can come from a different thread, so we'll use a queued connection.
    connect(this, SIGNAL(sigStateUpdateReceived(int)), this, SLOT(slotUpdateState(int)), Qt::QueuedConnection);
}

void KisAndroidDonations::syncState()
{
    QAndroidJniObject::callStaticMethod<void>("org/krita/android/DonationHelper", "syncState", "()V");
}

extern "C" JNIEXPORT void JNICALL Java_org_krita_android_JNIWrappers_donationStateUpdated(JNIEnv * /*env*/,
                                                                                          jobject /*obj*/,
                                                                                          jint state)
{
    KisApplication *app = qobject_cast<KisApplication *>(KisApplication::instance());
    if (app) {
        KisAndroidDonations *androidDonations = app->androidDonations();
        if (androidDonations) {
            Q_EMIT androidDonations->sigStateUpdateReceived(state);
        }
    }
}
