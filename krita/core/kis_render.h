/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
#ifndef KIS_RENDER_H_
#define KIS_RENDER_H_

#include <qpixmap.h>
#include "kis_types.h"

class QRect;

class KisRenderInterface : public KShared {

public:
	KisRenderInterface();
	KisRenderInterface(const KisRenderInterface& rhs);
	KisRenderInterface& operator=(const KisRenderInterface& rhs);
	virtual ~KisRenderInterface();

public:
	virtual Q_INT32 tileNum(Q_INT32 xpix, Q_INT32 ypix) const = 0;
	virtual KisTileMgrSP tiles() const = 0;
};

inline 
KisRenderInterface::KisRenderInterface()
{
}

inline
KisRenderInterface::KisRenderInterface(const KisRenderInterface& rhs) : KShared(rhs)
{
}

inline
KisRenderInterface& KisRenderInterface::operator=(const KisRenderInterface& rhs)
{
	KShared::operator=(rhs);
	return *this;
}

inline 
KisRenderInterface::~KisRenderInterface()
{
}

#endif // KIS_RENDER_H_

