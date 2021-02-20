/*
 *  SPDX-FileCopyrightText: 2012 Daniel Nicoletti <dantti12@gmail.com>
 *  SPDX-FileCopyrightText: 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_COLORD_H
#define KIS_COLORD_H

#include "dbus-types.h"

#include <QDBusObjectPath>
#include <QDBusServiceWatcher>
#include <QDBusPendingCallWatcher>

#include <QMap>
#include <QString>

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
    ~KisColord() override;

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
