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

#ifndef KIS_TOOL_FACTORY_H_
#define KIS_TOOL_FACTORY_H_

#include "kis_types.h"
#include "kactioncollection.h"

class KisToolFactory  : public KShared
{

public:
	KisToolFactory(KActionCollection * ac) {m_ac = ac;}
	virtual ~KisToolFactory() {};

	virtual KisTool * createTool() = 0;
	virtual QString name() { return QString("Abstract Tool"); }

protected:

	KActionCollection * m_ac;
};

#endif // KIS_TOOL_FACTORY_H_

