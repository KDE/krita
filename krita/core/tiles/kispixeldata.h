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
#if !defined KISPIXELDATA_H_
#define KISPIXELDATA_H_

#include "kistile.h"
#include "kistilemgr.h"

struct KisPixelData : public KShared {
	virtual ~KisPixelData();

	KisTileMgrSP mgr;
	KisTileSP tile;
	Q_INT32 mode;
	Q_INT32 x1;
	Q_INT32 y1;
	Q_INT32 x2;
	Q_INT32 y2;
	Q_UINT16 *data;
	bool owner;
	Q_INT32 width;
	Q_INT32 height;
	Q_INT32 stride;
	Q_INT32 depth;
};	

typedef KSharedPtr<KisPixelData> KisPixelDataSP;

#endif // KISPIXELDATA_H_

