/*
 *  SPDX-FileCopyrightText: 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
class KRITACOLOR_EXPORT KisColorManager : public QObject
{
    Q_OBJECT

public:
    explicit KisColorManager();
    ~KisColorManager() override;

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

    KisColorManager(const KisColorManager&);
    KisColorManager operator=(const KisColorManager&);

    class Private;
    const Private *const d;
};

#endif // KIS_COLOR_MANAGER_H
