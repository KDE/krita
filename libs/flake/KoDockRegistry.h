/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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

#ifndef KODOCKREGISTRY_
#define KODOCKREGISTRY_

#include "KoGenericRegistry.h"
#include "KoDockFactoryBase.h"
#include "flake_export.h"

#include <QObject>

/**
 * This singleton class keeps a register of all available dockers,
 * or rather, of the factories that can create the QDockWidget instances
 * for the mainwindows.
 * Note that adding your KoDockFactoryBase to this registry will mean it will automatically be
 * added to an application, no extra code is required for that.
 *
 * @see KoCanvasObserverBase
 */
class FLAKE_EXPORT KoDockRegistry : public KoGenericRegistry<KoDockFactoryBase*>
{
public:
    ~KoDockRegistry();

    /**
     * Return an instance of the KoDockRegistry
     * Create a new instance on first call and return the singleton.
     */
    static KoDockRegistry *instance();

private:
    KoDockRegistry();
    KoDockRegistry(const KoDockRegistry&);
    KoDockRegistry operator=(const KoDockRegistry&);
    void init();

    class Private;
    Private *d;
};

#endif
