/*
 *  SPDX-FileCopyrightText: 2007 Adrian Page <adrian@pagenet.plus.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_CONFIG_NOTIFIER_H_
#define KIS_CONFIG_NOTIFIER_H_

#include <QObject>
#include <QScopedPointer>

#include "kritaglobal_export.h"

/**
 * An object that emits a signal to inform interested parties that the
 * configuration settings have changed.
 */
class KRITAGLOBAL_EXPORT KisConfigNotifier : public QObject
{
    Q_OBJECT
public:
    KisConfigNotifier();
    ~KisConfigNotifier() override;

    /**
     * @return the KisConfigNotifier singleton
     */
    static KisConfigNotifier *instance();

    /**
     * Notify that the configuration has changed. This will cause the
     * configChanged() signal to be emitted.
     */
    void notifyConfigChanged(void);

    void notifyDropFramesModeChanged();
    void notifyPixelGridModeChanged();

Q_SIGNALS:
    /**
     * This signal is emitted whenever notifyConfigChanged() is called.
     */
    void configChanged(void);
    void dropFramesModeChanged();
    void pixelGridModeChanged();
private:
    KisConfigNotifier(const KisConfigNotifier&);
    KisConfigNotifier operator=(const KisConfigNotifier&);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KIS_CONFIG_NOTIFIER_H_
