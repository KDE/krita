/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_PAINTOP_PRESET_UPDATE_PROXY_H
#define __KIS_PAINTOP_PRESET_UPDATE_PROXY_H

#include <QScopedPointer>
#include <QObject>

#include "kritaimage_export.h"


/**
 * @brief The KisPaintOpPresetUpdateProxy class
 */
class KRITAIMAGE_EXPORT KisPaintOpPresetUpdateProxy : public QObject
{
    Q_OBJECT

public:
    KisPaintOpPresetUpdateProxy();
    ~KisPaintOpPresetUpdateProxy() override;

    void notifySettingsChanged();
    void notifyUniformPropertiesChanged();

    /**
     * Blocks all sigSettingsChanged() signals until unpostponeSettingsChanges()
     * is called. Used to perform "atomic" writing operations.
     *
     * @see unpostponeSettingsChanges()
     */
    void postponeSettingsChanges();

    /**
     * Unblocks sigSettingsChanged() and emits one signal if there were at least one
     * dropped signal while the block was held.
     */
    void unpostponeSettingsChanges();

Q_SIGNALS:
    void sigSettingsChanged();

    /**
     * Uncompressed signals are delivered in two stages. Firstly,
     * the early warning version is emitted, then the normal. The
     * early warning version is needed to let critical code, like
     * KisPresetShadowUpdater, to perform necessary actions before
     * other receivers got the notification. Don't use it unless
     * you know what you are doing. Use normal
     * sigSettingsChangedUncompressed() instead.
     */
    void sigSettingsChangedUncompressedEarlyWarning();
    void sigSettingsChangedUncompressed();
    void sigUniformPropertiesChanged();

private Q_SLOTS:
    void slotDeliverSettingsChanged();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_PAINTOP_PRESET_UPDATE_PROXY_H */
