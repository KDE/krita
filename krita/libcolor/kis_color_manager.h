/*
 *  Copyright (c) 2015 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_COLOR_MANAGER_H
#define KIS_COLOR_MANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QByteArray>

#include "kritacolor_export.h"
/**
 * @brief The KisColorManager class can be used as a cross-platform way to get the
 * display profile associated with a device.
 *
 * TODO: support other devices than monitors
 */
class KRITALIBCOLOR_EXPORT KisColorManager : public QObject
{
    Q_OBJECT

public:

    virtual ~KisColorManager();

    enum DeviceType {
        screen,
        printer,
        camera,
        scanner
    };

    /// Return the user-visible name for the given device
    QString deviceName(const QString &id);

    /// Return a list of device id's for the specified type
    QStringList devices(DeviceType type = screen) const;

    /// Return the icc profile for the given device and index (if a device has more than one profile)
    QByteArray displayProfile(const QString &device, int profile = 0) const;

    static KisColorManager *instance();

Q_SIGNALS:

    void changed(const QString device);

public Q_SLOTS:

private:

    explicit KisColorManager();
    KisColorManager(const KisColorManager&);
    KisColorManager operator=(const KisColorManager&);

    class Private;
    const Private *const d;
};

#endif // KIS_COLOR_MANAGER_H
