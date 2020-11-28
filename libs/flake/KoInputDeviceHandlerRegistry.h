/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOINPUTDEVICEHANDLERREGISTRY_H
#define KOINPUTDEVICEHANDLERREGISTRY_H

#include "KoGenericRegistry.h"
#include <KoInputDeviceHandler.h>
#include "kritaflake_export.h"

/**
 * This singleton class keeps a register of all custom input devices
 * for instance the 3D Space Navigator which generate input events.
 * These will get routed the active tools which can then do whatever
 * they like with it.
 */
class KRITAFLAKE_EXPORT KoInputDeviceHandlerRegistry : public KoGenericRegistry<KoInputDeviceHandler*>
{
public:
    KoInputDeviceHandlerRegistry();
    ~KoInputDeviceHandlerRegistry() override;

    /**
     * Return an instance of the KoInputDeviceHandlerRegistry
     * Create a new instance on first call and return the singleton.
     */
    static KoInputDeviceHandlerRegistry *instance();

private:
    KoInputDeviceHandlerRegistry(const KoInputDeviceHandlerRegistry&);
    KoInputDeviceHandlerRegistry operator=(const KoInputDeviceHandlerRegistry&);
    void init();

    class Private;
    Private *d;
};

#endif // KOINPUTDEVICEHANDLERREGISTRY_H
