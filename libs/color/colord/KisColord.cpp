/*
 *  SPDX-FileCopyrightText: 2012 Daniel Nicoletti <dantti12@gmail.com>
 *  SPDX-FileCopyrightText: 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisColord.h"

#include <klocalizedstring.h>
#include <kis_debug.h>


#include "CdInterface.h"
#include "CdProfileInterface.h"
#include "CdDeviceInterface.h"


struct Profile {
    QString kind;
    QString filename;
    QString title;
    qulonglong created;
    QString colorspace;
};

struct Device {

    ~Device() {
        qDeleteAll(profiles);
        profiles.clear();
    }

    QString id;
    QString kind;
    QString model;
    QString vendor;
    QString colorspace;

    QList<Profile*> profiles;

};

KisColord::KisColord(QObject *parent)
    : QObject(parent)
{
    //dbgKrita << "Creating KisColorD";

    m_cdInterface = new CdInterface(QLatin1String("org.freedesktop.ColorManager"),
                                    QLatin1String("/org/freedesktop/ColorManager"),
                                    QDBusConnection::systemBus(),
                                    this);

    // listen to colord for device events
    connect(m_cdInterface, SIGNAL(DeviceAdded(QDBusObjectPath)),
            this, SLOT(deviceAdded(QDBusObjectPath)));
    connect(m_cdInterface, SIGNAL(DeviceRemoved(QDBusObjectPath)),
            this, SLOT(deviceRemoved(QDBusObjectPath)));
    connect(m_cdInterface, SIGNAL(DeviceChanged(QDBusObjectPath)),
            this, SLOT(deviceChanged(QDBusObjectPath)));

    // Ask for devices
    QDBusPendingReply<QList<QDBusObjectPath> > async = m_cdInterface->GetDevices();
    QDBusPendingCallWatcher *displayWatcher = new QDBusPendingCallWatcher(async, this);
    connect(displayWatcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(gotDevices(QDBusPendingCallWatcher*)));


    // Make sure we know is colord is running
    QDBusServiceWatcher *watcher = new QDBusServiceWatcher("org.freedesktop.ColorManager",
                                                           QDBusConnection::systemBus(),
                                                           QDBusServiceWatcher::WatchForOwnerChange,
                                                           this);

    connect(watcher, SIGNAL(serviceOwnerChanged(QString,QString,QString)),
            this, SLOT(serviceOwnerChanged(QString,QString,QString)));
}

KisColord::~KisColord()
{
    qDeleteAll(m_devices);
    m_devices.clear();
}

QStringList KisColord::devices(const QString &type) const
{
    QStringList res;
    Q_FOREACH (Device *dev, m_devices.values()) {
        if (type == dev->kind) {
            res << dev->id;
        }
    }
    return res;
}

const QString KisColord::deviceName(const QString &id) const
{
    QString res;
    Q_FOREACH (Device *dev, m_devices.values()) {
        if (dev->id == id) {
            res = dev->model + ", " + dev->vendor;
        }
    }
    return res;
}

QByteArray KisColord::deviceProfile(const QString &id, int p)
{
    QByteArray ba;
    Device *dev = 0;
    Profile *profile = 0;
    Q_FOREACH (Device *d, m_devices.values()) {
        if (d->id == id) {
            dev = d;
            break;
        }
    }

    if (dev) {
        if (dev->profiles.size() > 0) {
            if (dev->profiles.size() < p) {
                profile = dev->profiles[p];
            }
            else {
                profile = dev->profiles[0];
            }
        }

        if (profile) {
            //dbgKrita << "profile filename" << profile->filename;
            QFile f(profile->filename);
            if (f.open(QFile::ReadOnly)) {
                ba = f.readAll();
            }
            else {
                dbgKrita << "Could not load profile" << profile->title << profile->filename;
            }
        }
    }

    return ba;
}



void KisColord::serviceOwnerChanged(const QString &serviceName, const QString &oldOwner, const QString &newOwner)
{
    Q_UNUSED(serviceName);
    if (newOwner.isEmpty() || oldOwner != newOwner) {
        // colord has quit or restarted
        qDeleteAll(m_devices);
        m_devices.clear();
    }
    emit changed();
}

void KisColord::gotDevices(QDBusPendingCallWatcher *call)
{
    //dbgKrita << "Got devices!!!";

    QDBusPendingReply<QList<QDBusObjectPath> > reply = *call;
    if (reply.isError()) {
        dbgKrita << "Unexpected message" << reply.error().message();
    } else {
        QList<QDBusObjectPath> devices = reply.argumentAt<0>();
        Q_FOREACH (const QDBusObjectPath &device, devices) {
            deviceAdded(device, false);
        }
        emit changed();
    }
    //dbgKrita << "gotDevices" << m_devices.count();
    call->deleteLater();
}

void KisColord::deviceChanged(const QDBusObjectPath &objectPath)
{
    CdDeviceInterface device(QLatin1String("org.freedesktop.ColorManager"),
                             objectPath.path(),
                             QDBusConnection::systemBus());
    if (!device.isValid()) {
        return;
    }

    if (!m_devices.contains(objectPath)) {
        //dbgKrita << "deviceChanged for an unknown device" << objectPath.path();
        deviceAdded(objectPath, false);
        return;
    }

    QList<QDBusObjectPath> profiles = device.profiles();

    Device *dev = m_devices[objectPath];
    qDeleteAll(dev->profiles);
    dev->profiles.clear();

    addProfilesToDevice(dev, profiles);

    //dbgKrita << "deviceChanged" << dev->id << "with" << profiles.size() << "profiles";

    emit changed(dev->id);
}

void KisColord::deviceAdded(const QDBusObjectPath &objectPath, bool emitChanged)
{
    if (m_devices.contains(objectPath)) {
        //dbgKrita << "Device is already on the list" << objectPath.path();
        return;
    }

    CdDeviceInterface device(QLatin1String("org.freedesktop.ColorManager"),
                             objectPath.path(),
                             QDBusConnection::systemBus());
    if (!device.isValid()) {
        dbgKrita << "Got an invalid device" << objectPath.path();
        return;
    }

    Device *dev = new Device;

    dev->id = device.deviceId();
    dev->kind = device.kind();
    dev->model = device.model();
    dev->vendor = device.vendor();
    dev->colorspace = device.colorspace();

    m_devices[objectPath] = dev;

    QList<QDBusObjectPath> profiles = device.profiles();
    addProfilesToDevice(dev, profiles);

//    dbgKrita << "deviceAdded" << dev->id
//             << dev->kind
//             << dev->model
//             << dev->vendor
//             << "with" << profiles.size() << "profiles";

    if (emitChanged) {
        emit changed();
    }
}

void KisColord::deviceRemoved(const QDBusObjectPath &objectPath)
{
    if (m_devices.contains(objectPath)) {
        delete m_devices.take(objectPath);
    }
    emit changed();
}

void KisColord::addProfilesToDevice(Device *dev, QList<QDBusObjectPath> profiles) const
{

    Q_FOREACH (const QDBusObjectPath &profileObjectPath, profiles) {

        CdProfileInterface profile(QLatin1String("org.freedesktop.ColorManager"),
                                   profileObjectPath.path(),
                                   QDBusConnection::systemBus());
        if (!profile.isValid()) {
            return;
        }


        Profile *p = new Profile;

        p->kind = profile.kind();
        p->filename = profile.filename();
        p->title = profile.title();
        p->created = profile.created();
        p->colorspace = profile.colorspace();

        dev->profiles << p;
    }
}

