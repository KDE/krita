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

#if !defined KISTILEMEDIATOR_H_
#define KISTILEMEDIATOR_H_

#include <qmutex.h>
#include "kistile.h"
#include "kistilemgr.h"

#define TILE_NOT_ATTACHED -1


/**
   Comment taken from Patrick Julien's mail dd. 11/10/03

   KisTileMgr helps upper layers have a unified, simple interface to
   find and manipulate tiles. However, lower layers (swapping, for
   example, if it's ever written) would find this complicated to use,
   you see, krita, being a koffice application, supports multiple
   frames, documents, etc. Layers, channels, masks, images for all the
   documents are KisPaintDevices, this is a whole lot of KisTileMgr's
   to go over, KisTileMediator allows a lower layer (like swapping) to
   access tiles by different criterias without actually worrying where
   tiles actually are in memory since it simply does not care. Here we
   would access tiles by their age, not by which layer etc.

   KisTilemediator is a singleton that contains a reference to each
   and every tile that Krita contains.

   This is currently only productively used in the
   KisTileMgr::tileCoord(const KisTileSP& tile, QPoint& coord) method,
   which is only used in KisImage::renderToProjection. Is it usefully
   used here? I'm not sure.

 */
class KisTileMediator {
public:
	KisTileMediator();
	~KisTileMediator();

	void attach(KisTile *tile, KisTileMgr *mgr, Q_INT32 tilenum);
	void detach(KisTile *tile, KisTileMgr *mgr, Q_INT32 tilenum);
	void detachAll(KisTileMgr *mgr);
	Q_INT32 tileNum(KisTile *tile, KisTileMgr *mgr);

private:
	KisTileMediator(const KisTileMediator&);
	KisTileMediator& operator=(const KisTileMediator&);
};

#endif // KISTILEMEDIATOR_H_

