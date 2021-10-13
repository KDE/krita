/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_PAINTOP_PRESET_UPDATE_PROXY_H
#define __KIS_PAINTOP_PRESET_UPDATE_PROXY_H

#include <QScopedPointer>
#include <QObject>


/**
 * @brief The KisPaintOpPresetUpdateProxy class
 */
class KisPaintOpPresetUpdateProxy : public QObject
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
    void sigSettingsChangedUncompressed();
    void sigUniformPropertiesChanged();

private Q_SLOTS:
    void slotDeliverSettingsChanged();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_PAINTOP_PRESET_UPDATE_PROXY_H */
