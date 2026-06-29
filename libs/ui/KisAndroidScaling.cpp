/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "KisAndroidScaling.h"

#include <KisAndroidDonations.h>
#include <KisAndroidUtils.h>
#include <KisApplication.h>
#include <kis_config.h>

#include <QGuiApplication>

#include <qpa/qplatformscreen.h>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QJniObject>
#else
#include <QAndroidJniObject>
using QJniObject = QAndroidJniObject;
#endif

KisAndroidScaling::KisAndroidScaling(KisConfig &cfg, KisApplication *app)
    : QObject(app)
{
    KIS_ASSERT(app);

    if (isHighDpiScalingEnabled()) {
        QScreen *screen = QGuiApplication::primaryScreen();
        if (screen) {
            m_initialPrimaryScreen = screen;
            m_initialPrimaryScreenDevicePixelRatio = screen->devicePixelRatio();
        }
    }

    qreal scale = getValidInitialScale(cfg);
    if (scale >= 1.0) {
        setPrimaryScreenScale(scale);
    } else {
        // Reset the configuration, it wasn't valid or the screen changed.
        cfg.setAndroidScalingLastInitialScale(cfg.androidScalingLastInitialScale(true));
        cfg.setAndroidScalingTargetScale(cfg.androidScalingTargetScale(true));
        cfg.setAndroidScalingAskOnStartup(cfg.androidScalingAskOnStartup(true));
    }

    // Queued connections are used in the following to make sure they run on
    // the main thread. The signals may come from the Android UI thread.
    connect(this,
            &KisAndroidScaling::sigJniSetPrimaryScreenScale,
            this,
            &KisAndroidScaling::slotJniSetPrimaryScreenScale,
            Qt::QueuedConnection);
    connect(this,
            &KisAndroidScaling::sigJniSaveInterfaceScale,
            this,
            &KisAndroidScaling::slotJniSaveInterfaceScale,
            Qt::QueuedConnection);
    connect(this,
            &KisAndroidScaling::sigJniScalingDialogActive,
            this,
            &KisAndroidScaling::slotJniScalingDialogActive,
            Qt::QueuedConnection);

    KisAndroidDonations *androidDonations = app->androidDonations();
    KIS_SAFE_ASSERT_RECOVER_NOOP(androidDonations);
    if (androidDonations) {
        connect(androidDonations,
                &KisAndroidDonations::sigSplashDialogDismissed,
                this,
                &KisAndroidScaling::slotSplashDialogDismissed,
                Qt::QueuedConnection);
    }
}

KisAndroidScaling *KisAndroidScaling::instance()
{
    KisApplication *app = qobject_cast<KisApplication *>(KisApplication::instance());
    if (app) {
        return app->androidScaling();
    } else {
        return nullptr;
    }
}

void KisAndroidScaling::slotSplashDialogDismissed()
{
    maybeShowDialog(true);
}

void KisAndroidScaling::slotJniSetPrimaryScreenScale(qreal scale)
{
    if (m_scalingDialogActive) {
        setPrimaryScreenScale(scale);
    }
}

void KisAndroidScaling::slotJniSaveInterfaceScale(bool askOnStartup)
{
    KisConfig cfg(false);
    if (m_scalingDialogActive && m_initialPrimaryScreenDevicePixelRatio > 0.0) {
        QScreen *screen = m_initialPrimaryScreen.data();
        if (screen) {
            cfg.setAndroidScalingLastInitialScale(m_initialPrimaryScreenDevicePixelRatio);
            cfg.setAndroidScalingTargetScale(screen->devicePixelRatio());
            cfg.setAndroidScalingAskOnStartup(askOnStartup);
        }
    }
}

void KisAndroidScaling::slotJniScalingDialogActive(bool active)
{
    m_scalingDialogActive = active;
}

bool KisAndroidScaling::isHighDpiScalingEnabled()
{
    // Detects whether we have scaling available in this session. Compare
    // with the startup handling of the scaling setting in main.cc. We do
    // *not* check the config value here, it only applies after a restart.
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return !qEnvironmentVariableIsSet("QT_ENABLE_HIGHDPI_SCALING")
        && qgetenv("QT_ENABLE_HIGHDPI_SCALING") != QByteArrayLiteral("0");
#else
    return QCoreApplication::testAttribute(Qt::AA_EnableHighDpiScaling)
        && !QCoreApplication::testAttribute(Qt::AA_DisableHighDpiScaling);
#endif
}

qreal KisAndroidScaling::getValidInitialScale(KisConfig &cfg) const
{
    if (m_initialPrimaryScreen.isNull()) {
        return 0.0;
    }

    // If we don't have a previous screen scale or the default screen scale
    // changed, the configuration is invalid and we fall back to the default.
    qreal lastInitialScale = cfg.androidScalingLastInitialScale();
    if (lastInitialScale < 1.0 || !qFuzzyCompare(lastInitialScale, m_initialPrimaryScreenDevicePixelRatio)) {
        return 0.0;
    }

    return cfg.androidScalingTargetScale();
}

void KisAndroidScaling::setPrimaryScreenScale(qreal scale)
{
    if (m_initialPrimaryScreenDevicePixelRatio > 0.0) {
        QScreen *screen = m_initialPrimaryScreen.data();
        if (screen) {
            QPlatformScreen *platformScreen = screen->handle();
            if (platformScreen) {
                qreal actualScale = qMax(1.0, scale);
                qreal densityAdjustment = actualScale / m_initialPrimaryScreenDevicePixelRatio;
                if (!qFuzzyCompare(densityAdjustment, platformScreen->densityAdjustment())) {
                    platformScreen->setDensityAdjustment(densityAdjustment);
                    Q_EMIT sigInterfaceScaleChanged();
                }
            }
        }
    }
}

void KisAndroidScaling::maybeShowDialog(bool startup)
{
    // We want to see the dialog if scaling is supported, the dialog isn't
    // already up and either the user manually summoned it or they want to
    // see the dialog on startup. The user can only decide to remember the
    // scaling setting after they decided on a target scale at least once
    // before. This is to make sure that they actually used the application
    // at that scale and can make an informed decision about not wanting to
    // see the dialog again instead of despairing at choosing a wrong scale
    // and being unable to find the dialog again.
    if (m_scalingDialogActive) {
        return;
    }

    QScreen *screen = m_initialPrimaryScreen.data();
    if (!screen) {
        return;
    }

    KisConfig cfg(true);
    bool haveTargetScale = cfg.androidScalingTargetScale() >= 1.0;
    bool askOnStartup = cfg.androidScalingAskOnStartup();
    if (!startup || askOnStartup) {
        QJniObject activity = QJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                 "activity",
                                                                 "()Landroid/app/Activity;");
        KisAndroidUtils::clearJniException(QStringLiteral("getting activity in KisAndroidScalingDialog"));

        if (activity.isValid()) {
            activity.callMethod<void>("showScalingDialog",
                                      "(DDZZ)V",
                                      jdouble(m_initialPrimaryScreen->devicePixelRatio()),
                                      jdouble(m_initialPrimaryScreenDevicePixelRatio),
                                      jboolean(askOnStartup),
                                      jboolean(!startup || haveTargetScale));
            KisAndroidUtils::clearJniException(QStringLiteral("showing scaling dialog in KisAndroidScalingDialog"));
        }
    }
}

extern "C" JNIEXPORT void JNICALL Java_org_krita_android_JNIWrappers_setPrimaryScreenScale(JNIEnv *env,
                                                                                           jobject obj,
                                                                                           jdouble scale)
{
    Q_UNUSED(env);
    Q_UNUSED(obj);
    KisAndroidScaling *androidScaling = KisAndroidScaling::instance();
    if (androidScaling) {
        Q_EMIT androidScaling->sigJniSetPrimaryScreenScale(scale);
    }
}

extern "C" JNIEXPORT void JNICALL Java_org_krita_android_JNIWrappers_savePrimaryScreenScale(JNIEnv *env,
                                                                                            jobject obj,
                                                                                            jboolean askOnStartup)
{
    Q_UNUSED(env);
    Q_UNUSED(obj);
    KisAndroidScaling *androidScaling = KisAndroidScaling::instance();
    if (androidScaling) {
        Q_EMIT androidScaling->sigJniSaveInterfaceScale(askOnStartup);
    }
}

extern "C" JNIEXPORT void JNICALL Java_org_krita_android_JNIWrappers_onScalingDialogShown(JNIEnv *env, jobject obj)
{
    Q_UNUSED(env);
    Q_UNUSED(obj);
    KisAndroidScaling *androidScaling = KisAndroidScaling::instance();
    if (androidScaling) {
        Q_EMIT androidScaling->sigJniScalingDialogActive(true);
    }
}

extern "C" JNIEXPORT void JNICALL Java_org_krita_android_JNIWrappers_onScalingDialogDismissed(JNIEnv *env, jobject obj)
{
    Q_UNUSED(env);
    Q_UNUSED(obj);
    KisAndroidScaling *androidScaling = KisAndroidScaling::instance();
    if (androidScaling) {
        Q_EMIT androidScaling->sigJniScalingDialogActive(false);
    }
}
