/* This file is part of the KDE project
 * Copyright (c) 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KODEVICEREGISTRY_H
#define KODEVICEREGISTRY_H

#include <QObject>

#include "KoGenericRegistry.h"
#include <KoInputDeviceHandler.h>
#include "flake_export.h"

/**
 * This singleton class keeps a register of all custom input devices
 * for instance the 3D Space Navigator which generate input events.
 * These will get routed the the active tools which can then do whatever
 * they like with it.
 */
class FLAKE_EXPORT KoInputDeviceHandlerRegistry : public KoGenericRegistry<KoInputDeviceHandler*>
{
public:
    ~KoInputDeviceHandlerRegistry();

    /**
     * Return an instance of the KoInputDeviceHandlerRegistry
     * Create a new instance on first call and return the singleton.
     */
    static KoInputDeviceHandlerRegistry *instance();

private:
    KoInputDeviceHandlerRegistry();
    KoInputDeviceHandlerRegistry(const KoInputDeviceHandlerRegistry&);
    KoInputDeviceHandlerRegistry operator=(const KoInputDeviceHandlerRegistry&);
    void init();

    class Private;
    Private *d;
};

#endif // KODEVICEREGISTRY_H
