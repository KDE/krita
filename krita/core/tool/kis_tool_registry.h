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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef KIS_TOOL_REGISTRY_H_
#define KIS_TOOL_REGISTRY_H_

#include "kis_types.h"
#include "kis_generic_registry.h"

class KActionCollection;
class KisCanvasSubject;
class QStringList;

/**
 * A registry, similar to the tool and colormodel registry
 * where new tool plugins can register themselves. KisToolRegistry
 * in contrast to the paintop and colormodel registries, creates
 * a vector containing instances of all registered tools.
 */
class KisToolRegistry : public KisGenericRegistry<KisToolFactorySP> {

public:
	virtual ~KisToolRegistry();

 	static KisToolRegistry* instance();

	vKisTool createTools(KisCanvasSubject *subject) const;
	KisTool * createTool(KisCanvasSubject * subject, KisID & id) const;
	
private:
	KisToolRegistry();
 	KisToolRegistry(const KisToolRegistry&);
 	KisToolRegistry operator=(const KisToolRegistry&);

	static KisToolRegistry *m_singleton;
};

#endif // KIS_TOOL_REGISTRY_H_

