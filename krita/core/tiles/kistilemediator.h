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

class KisTileMediator {
public:
	KisTileMediator();
	~KisTileMediator();

	void attach(KisTileSP tile, KisTileMgrSP mgr, Q_INT32 tilenum);
	void detach(KisTileSP tile, KisTileMgrSP mgr, Q_INT32 tilenum);
	void detachAll(KisTileMgr *mgr);
	Q_INT32 tileNum(KisTileSP tile, KisTileMgrSP mgr);

private:
	KisTileMediator(const KisTileMediator&);
	KisTileMediator& operator=(const KisTileMediator&);
};

#endif // KISTILEMEDIATOR_H_

