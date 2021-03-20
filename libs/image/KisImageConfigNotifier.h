/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISIMAGECONFIGNOTIFIER_H
#define KISIMAGECONFIGNOTIFIER_H

#include <QObject>
#include "kritaimage_export.h"

class KRITAIMAGE_EXPORT KisImageConfigNotifier : public QObject
{
    Q_OBJECT
public:
    explicit KisImageConfigNotifier();
    ~KisImageConfigNotifier() override;

    static KisImageConfigNotifier* instance();

    /**
     * Notify that the configuration has changed. This will cause the
     * configChanged() signal to be emitted.
     */
    void notifyConfigChanged(void);

Q_SIGNALS:
    /**
     * This signal is emitted whenever notifyConfigChanged() is called.
     */
    void configChanged(void);

private:
    Q_DISABLE_COPY(KisImageConfigNotifier)

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISIMAGECONFIGNOTIFIER_H
