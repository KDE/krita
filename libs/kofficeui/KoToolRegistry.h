/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2006 Thomas Zander <zander@kde.org>
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

#ifndef KO_TOOL_REGISTRY_H_
#define KO_TOOL_REGISTRY_H_

#include <QObject>

#include "KoGenericRegistry.h"
#include <KoToolFactory.h>
#include <koffice_export.h>

/**
 * This singleton class keeps a register of all available flake tools,
 * or rather, of the factories that the KoToolBox (and KoToolManager) will use
 * to create flake tools.
 */
class KOFFICEUI_EXPORT KoToolRegistry : public QObject, public KoGenericRegistry<KoToolFactory*> {
    Q_OBJECT

public:
    virtual ~KoToolRegistry();

    /**
     * Return an instance of the KoToolRegistry
     * Create a new instance on first call and return the singleton.
     */
    static KoToolRegistry* instance();

private:
    KoToolRegistry();
    KoToolRegistry(const KoToolRegistry&);
    KoToolRegistry operator=(const KoToolRegistry&);

    static KoToolRegistry *s_instance;
};

#endif
