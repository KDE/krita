/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __KIS_PAINTOP_SETTINGS_UPDATE_PROXY_H
#define __KIS_PAINTOP_SETTINGS_UPDATE_PROXY_H

#include <QScopedPointer>
#include <QObject>

/**
 * @brief The KisPaintopSettingsUpdateProxy class
 */
class KisPaintopSettingsUpdateProxy : public QObject
{
    Q_OBJECT

public:
    KisPaintopSettingsUpdateProxy(QObject *parent = 0);
    ~KisPaintopSettingsUpdateProxy() override;

    void setDirty(bool dirty);

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
    void sigUniformPropertiesChanged();

private Q_SLOTS:
    void slotDeliverSettingsChanged();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_PAINTOP_SETTINGS_UPDATE_PROXY_H */
