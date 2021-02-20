/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOINPUTDEVICEHANDLER_H
#define KOINPUTDEVICEHANDLER_H

#include "kritaflake_export.h"
#include <QObject>

/**
 * Base class for all custom input devices.
 */
class KRITAFLAKE_EXPORT KoInputDeviceHandler : public QObject
{
    Q_OBJECT
public:
    KoInputDeviceHandler(QObject *parent, const QString &id);
    ~KoInputDeviceHandler() override;

    /**
     * Return the id for the device.
     * @return the id for the device
     */
    QString id() const;

    /**
     * Starts the device.
     * @return true if the device was started, false otherwise
     */
    virtual bool start() = 0;

    /**
     * Stops the device.
     * @return true if the device was stopped, false otherwise
     */
    virtual bool stop() = 0;

private:
    class Private;
    Private * const d;
};

#endif // KOINPUTDEVICEHANDLER_H
