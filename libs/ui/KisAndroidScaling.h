/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef __KISANDROIDSCALING_H_
#define __KISANDROIDSCALING_H_

#include <QObject>
#include <QPointer>
#include <QScreen>

#include "kritaui_export.h"

class KisApplication;
class KisConfig;

class KRITAUI_EXPORT KisAndroidScaling : public QObject
{
    Q_OBJECT

public:
    explicit KisAndroidScaling(KisConfig &cfg, KisApplication *app);

    static KisAndroidScaling *instance();

    bool isSupported() const
    {
        return !m_initialPrimaryScreen.isNull();
    }

    void showDialog()
    {
        maybeShowDialog(false);
    }

Q_SIGNALS:
    void sigInterfaceScaleChanged();
    void sigJniSetPrimaryScreenScale(qreal scale);
    void sigJniSaveInterfaceScale(bool askOnStartup);
    void sigJniScalingDialogActive(bool active);

private Q_SLOTS:
    void slotSplashDialogDismissed();
    void slotJniSetPrimaryScreenScale(qreal scale);
    void slotJniSaveInterfaceScale(bool askOnStartup);
    void slotJniScalingDialogActive(bool active);

private:
    static bool isHighDpiScalingEnabled();

    qreal getValidInitialScale(KisConfig &cfg) const;

    void setPrimaryScreenScale(qreal scale);

    void maybeShowDialog(bool startup);

    QPointer<QScreen> m_initialPrimaryScreen;
    qreal m_initialPrimaryScreenDevicePixelRatio = 0.0;
    bool m_scalingDialogActive = false;
};

namespace KisAndroidScalingDialog
{

KRITAUI_EXPORT void init();

KRITAUI_EXPORT void setInitialScale(KisConfig &cfg);

KRITAUI_EXPORT void show();

KRITAUI_EXPORT void maybeShowOnStartup();

KRITAUI_EXPORT bool isSupported();

} // namespace KisAndroidScalingDialog

#endif
