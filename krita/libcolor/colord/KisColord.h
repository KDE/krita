/*
 *  Copyright (C) 2012 by Daniel Nicoletti <dantti12@gmail.com>
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

#ifndef KIS_COLORD_H
#define KIS_COLORD_H

#include "dbus-types.h"

#include <QDBusObjectPath>
#include <QDBusServiceWatcher>
#include <QDBusPendingCallWatcher>

#include <QMap>
#include <QString>

#include <kdatetime.h>

#include <QMetaType>


#define CD_PROFILE_METADATA_DATA_SOURCE_EDID	 "edid"
#define CD_PROFILE_METADATA_DATA_SOURCE_CALIB	 "calib"
#define CD_PROFILE_METADATA_DATA_SOURCE_STANDARD "standard"
#define CD_PROFILE_METADATA_DATA_SOURCE_TEST     "test"

class CdInterface;
struct Device;


#include "kritacolord_export.h"

class KRITACOLORD_EXPORT KisColord : public QObject
{
    Q_OBJECT
public:
    KisColord(QObject *parent = 0);
    ~KisColord();

    QStringList devices(const QString &type) const;
    const QString deviceName(const QString &id) const;
    QByteArray deviceProfile(const QString &id, int profile);

Q_SIGNALS:
    void changed();
    void changed(const QString& device);


private Q_SLOTS:

    void serviceOwnerChanged(const QString &serviceName, const QString &oldOwner, const QString &newOwner);

    void gotDevices(QDBusPendingCallWatcher *call);
    void deviceChanged(const QDBusObjectPath &objectPath);
    void deviceAdded(const QDBusObjectPath &objectPath, bool emitChanged = true);
    void deviceRemoved(const QDBusObjectPath &objectPath);

private:

    void addProfilesToDevice(Device *dev, QList<QDBusObjectPath> profiles) const;

    QMap<QDBusObjectPath, Device*> m_devices;

    CdInterface *m_cdInterface;
};

#endif // COLORD_KCM_H
