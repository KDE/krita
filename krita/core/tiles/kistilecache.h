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
#ifndef KISTILECACHE_H_
#define KISTILECACHE_H_

#include <qglobal.h>
#include <ksharedptr.h>
#include "kistile.h"

class KisTileCacheInterface : public KShared {
	typedef KShared super;

public:
	KisTileCacheInterface();
	virtual ~KisTileCacheInterface();

public:
	virtual void sizeHint(Q_INT32 nelements) = 0;
	virtual void flush(KisTileSP tile) = 0;
	virtual void insert(KisTileSP tile) = 0;
};

inline
KisTileCacheInterface::KisTileCacheInterface()
{
}

inline
KisTileCacheInterface::~KisTileCacheInterface()
{
}

#endif // KISTILECACHE_H_

