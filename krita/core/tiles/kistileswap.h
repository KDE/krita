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
#ifndef KISTILESWAP_H_
#define KISTILESWAP_H_

#include <qglobal.h>
#include "kistile.h"

#define SWAP_IN_CORE -1

class KisTileSwapInterface : public KShared {
	typedef KShared super;

public:
	KisTileSwapInterface();
	virtual ~KisTileSwapInterface();

public:
	virtual Q_INT32 add(KisTileSP tile) = 0;
	virtual void remove(Q_INT32 swapNo) = 0;

	virtual void swapIn(KisTileSP tile) = 0;
	virtual void swapInAsync(KisTileSP tile) = 0;
	virtual void swapOut(KisTileSP tile) = 0;
	virtual void swapDel(KisTileSP tile) = 0;
	virtual void swapCompress(KisTileSP tile) = 0;
};

inline
KisTileSwapInterface::KisTileSwapInterface()
{
}

inline
KisTileSwapInterface::~KisTileSwapInterface()
{
}

#endif // KISTILESWAP_H_

