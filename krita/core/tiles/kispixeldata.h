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
#if !defined KISPIXELDATA_H_
#define KISPIXELDATA_H_

#include "kis_types.h"
#include "kistilemgr.h"

/**
   This struct offers access to the actual bytes with pixel data.
   
   XXX: how about endianness, and is it 32-bit aligned?
*/
   
struct KisPixelData : public KShared {
	KisPixelData();
	virtual ~KisPixelData();
    
	KisTileMgrSP mgr; // The KisTileMgr that 'owns' this block of data
	KisTileSP tile;  // The tile this data belongs to -- if any?
	Q_INT32 mode; // ???
	Q_INT32 x1; // In image coordinates, not tile coordinages
	Q_INT32 y1;
	Q_INT32 x2;
	Q_INT32 y2;
	QUANTUM *data; // Pointer to the pixeldata -- just a plain sequence of 
	// bytes
	bool owner; // Whether the data will be managed by somebody else
	Q_INT32 width;
	Q_INT32 height; 
	Q_INT32 stride; // The number of bytes one line of pixels takes
	Q_INT32 depth; // The number of bytes each pixel takes
};

inline
KisPixelData::KisPixelData()
{
	data = 0;
}

#endif // KISPIXELDATA_H_

