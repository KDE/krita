/* This file is part of the KDE project
 * Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#ifndef KO_TOOL_REGISTRY_H_
#define KO_TOOL_REGISTRY_H_

#include <QObject>

#include "KoGenericRegistry.h"
#include <KoToolFactoryBase.h>
#include "flake_export.h"

/**
 * This singleton class keeps a register of all available flake tools,
 * or rather, of the factories that the KoToolBox (and KoToolManager) will use
 * to create flake tools.
 */
class FLAKE_EXPORT KoToolRegistry : public KoGenericRegistry<KoToolFactoryBase*>
{
public:
    ~KoToolRegistry();

    /**
     * Return an instance of the KoToolRegistry
     * Create a new instance on first call and return the singleton.
     */
    static KoToolRegistry *instance();

    /**
     * Add a toolfactory from a deferred plugin. This will cause the toolFactoryAdded signal
     * to be emitted, which is caught by the KoToolManager which then adds the tool to all
     * canvases.
     */
    void addDeferred(KoToolFactoryBase *toolFactory);

private:
    KoToolRegistry();
    KoToolRegistry(const KoToolRegistry&);
    KoToolRegistry operator=(const KoToolRegistry&);
    void init();

    class Private;
    Private *d;
};

#endif
