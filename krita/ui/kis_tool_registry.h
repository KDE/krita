/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_TOOL_REGISTRY_H_
#define KIS_TOOL_REGISTRY_H_

#include <QObject>

#include "kis_tool_types.h"
#include "KoGenericRegistry.h"
#include <krita_export.h>

class KActionCollection;
class KisCanvasSubject;
class QStringList;

/**
 * A registry, similar to the tool and colormodel registry
 * where new tool plugins can register themselves. KisToolRegistry
 * in contrast to the paintop and colormodel registries, creates
 * a vector containing instances of all registered tools.
 */
class KRITAUI_EXPORT KisToolRegistry : public QObject, public KoGenericRegistry<KisToolFactorySP>{

    Q_OBJECT

public:
    virtual ~KisToolRegistry();

     static KisToolRegistry* instance();

    vKisTool createTools(KActionCollection * ac, KisCanvasSubject *subject) const;
    KisTool * createTool(KActionCollection * ac, KisCanvasSubject * subject, KoID & id) const;

private:
    KisToolRegistry();
     KisToolRegistry(const KisToolRegistry&);
     KisToolRegistry operator=(const KisToolRegistry&);

    static KisToolRegistry *m_singleton;
};

#endif // KIS_TOOL_REGISTRY_H_

