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
#if !defined KISTILEMGR_H_
#define KISTILEMGR_H_

#include <qcstring.h>
#include <qglobal.h>
#include <qmutex.h>
#include <qpair.h>
#include <qvaluelist.h>
#include <ksharedptr.h>
#include "kistile.h"

class QPoint;
struct KisPixelData;
typedef KSharedPtr<KisPixelData> KisPixelDataSP;
class KisTileMediator;

#define TILEMODE_NONE 0
#define TILEMODE_READ 1
#define TILEMODE_WRITE 2
#define TILEMODE_RW (TILEMODE_READ|TILEMODE_WRITE)

class KisTileMgr : public KShared {
	typedef KShared super;

public:
	KisTileMgr(Q_UINT32 depth, Q_UINT32 width, Q_UINT32 height);
	virtual ~KisTileMgr();

public:
	void attach(KisTileSP tile, Q_INT32 tilenum);
	void detach(KisTileSP tile, Q_INT32 tilenum);

	KisTileSP tile(Q_INT32 xpix, Q_INT32 ypix, Q_INT32 mode);
	KisTileSP tile(Q_INT32 tilenum, Q_INT32 mode);
	void tileAsync(Q_INT32 xpix, Q_INT32 ypix);

	void tileMap(Q_INT32 xpix, Q_INT32 ypix, KisTileSP src);
	void tileMap(Q_INT32 tilenum, KisTileSP src);

	bool completetlyValid() const;
	void validate(KisTileSP tile);
	KisTileSP invalidate(Q_INT32 xpix, Q_INT32 ypix);
	KisTileSP invalidate(KisTileSP tile, Q_INT32 xpix, Q_INT32 ypix);
	void invalidateTiles(KisTileSP tile);

	bool empty() const;
	Q_UINT32 width() const;
	Q_UINT32 height() const;
	Q_UINT32 nrows() const;
	Q_UINT32 ncols() const;
	Q_UINT32 depth() const;

	Q_UINT32 memSize();
	void tileCoord(const KisTileSP& tile, QPoint& coord);
	void tileCoord(const KisTileSP& tile, Q_INT32 *x, Q_INT32 *y);

	void mapOver(KisTileSP dst, KisTileSP src);

	KisPixelDataSP pixelData(Q_INT32 x1, Q_INT32 y1, Q_INT32 x2, Q_INT32 y2, Q_INT32 mode);
	void releasePixelData(KisPixelDataSP pd);
	Q_INT32 tileNum(Q_UINT32 xpix, Q_UINT32 ypix) const;

private:
	KisTileMgr(const KisTileMgr& rhs);
	KisTileMgr& operator=(const KisTileMgr&);
	void allocate(Q_INT32 ntiles);
	KisTileSP invalidateTile(KisTileSP tile, Q_INT32 tilenum);

	void readPixelData(Q_INT32 x1, Q_INT32 y1, Q_INT32 x2, Q_INT32 y2, QUANTUM *buffer, Q_UINT32 stride);
	void readPixelData(KisPixelDataSP pd);

	void writePixelData(Q_INT32 x1, Q_INT32 y1, Q_INT32 x2, Q_INT32 y2, QUANTUM *buffer, Q_UINT32 stride);
	void writePixelData(KisPixelDataSP pd);

private:
	Q_UINT32 m_width;
	Q_UINT32 m_height;
	Q_UINT32 m_depth;
	Q_UINT32 m_ntileRows;
	Q_UINT32 m_ntileCols;
	vKisTileSP m_tiles;
	QMutex m_mutex;
	KisTileMediator *m_mediator;
};

#endif // KISTILEMGR_H_

