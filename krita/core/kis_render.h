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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#if !defined KIS_RENDER_H_
#define KIS_RENDER_H_

#include <qpixmap.h>
#include <ksharedptr.h>

class QRect;

class KisRenderInterface : public KShared {
public:
	KisRenderInterface();
	KisRenderInterface(const KisRenderInterface& rhs);
	KisRenderInterface& operator=(const KisRenderInterface& rhs);
	virtual ~KisRenderInterface();

public:
	virtual void invalidate(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h) = 0;
	virtual void invalidate(const QRect& rc) = 0;
	virtual void invalidate() = 0;
	virtual QPixmap pixmap() = 0;
	virtual QPixmap recreatePixmap() = 0;
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

