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
#ifndef KISTILEMGR_H_
#define KISTILEMGR_H_

#include <qcstring.h>
#include <qglobal.h>
#include <qmutex.h>
#include <qpair.h>
#include <qvaluelist.h>

#include <ksharedptr.h>

#include "kistile.h"
#include "kis_types.h"
#include "kis_pixel_manager.h"

class QPoint;
struct KisPixelData;
class KisTileMediator;

#define TILEMODE_NONE 0
#define TILEMODE_READ 1
#define TILEMODE_WRITE 2
#define TILEMODE_RW (TILEMODE_READ|TILEMODE_WRITE)

/**
 * KisTileMgr manages the imagedata for implementations of KisRenderInterface.
 * Those are:
 *
 * KisPaintDevices
 *    KisLayer
 *       KisBackround
 *       KisSelection
 *    KisChannel
 *       KisMask
 *
 * and
 *
 * KisImage
 * 
 * Imagedata is structured in the form of tiles, by default 64 by 64
 * pixels big. The KisTileMgr is smart enough to hide most of the
 * details of reading on and writing from tiles.
 *
 * @short KisTileMgr manages the pixels for every layer.
 *
 */
class KisTileMgr : public KisPixelManagerInterface, public KShared {

public:
        /**
          Create a KisTileMgr of width and height with the specified
          depth. For now, this depth is in QUANTUMS, which are bytes,
	  and is this equivelent to channels. Later, with will still be
	  bytes, but how the bytes are divided up by the color strategy
	  into channels, is up to that strategy.
         */
	KisTileMgr(Q_UINT32 depth, Q_UINT32 width, Q_UINT32 height);

        /**
          Create a new KisTileMgr of width and height with the
          specified colour depth. Deep copies the tiles of tm to the
          new KisTileMgr.
         
          XXX: I am not sure what happens when tm is bigger or smaller
          than width and height, nor what happens when tm has a
          different depth.
         */
	KisTileMgr(KisTileMgr *tm, Q_UINT32 depth, Q_UINT32 width, Q_UINT32 height);

        /**
          Creates a new KisTileMgr based on rhs, shares a reference
          to the tiles of rhs.
         */
	KisTileMgr(const KisTileMgr& rhs);

	virtual ~KisTileMgr();
        /**
          XXX?
         */
	void attach(KisTileSP tile, Q_INT32 tilenum);

        /**
          XXX?
        */
	void detach(KisTileSP tile, Q_INT32 tilenum);

        /**
           Create or duplicate a tile at location xpix/ypix.
         */
	KisTileSP tile(Q_INT32 xpix, Q_INT32 ypix, Q_INT32 mode);

        /**
           Create or duplicate a tile with identity tilenum; tilenum
           is computed from the x/y coordinates of the tile in the
           image
         */
	KisTileSP tile(Q_INT32 tilenum, Q_INT32 mode);

        /**
           XXX
        */
	void tileMap(Q_INT32 xpix, Q_INT32 ypix, KisTileSP src);
        /**
           XXX
        */
	void tileMap(Q_INT32 tilenum, KisTileSP src);

        /**
           Returns true if this KisTileMgr does not manage any
           tiles.
        */
	bool empty() const;

        /**
           Width in pixels of the total area managed by this KisTileMgr
         */
	Q_INT32 width() const;

        /**
           Height in pixels of the total area managed by this KisTileMgr
         */
	Q_INT32 height() const;

        /**
           Number of rows of tiles managed by this KisTileMgr
         */
	Q_UINT32 nrows() const;

        /**
           Number of columns of tiles managed by this KisTileMgr
         */
	Q_UINT32 ncols() const;

        /**
         * Depth in bytes (i.e., for now == channels) of a pixel
         */
	Q_INT32 depth() const;

        /**
           Total size in memory the data managed by this KisTileMgr 
        */
	Q_UINT32 memSize();

        /**
           Puts the x/y coordinates of the top left (?) corner
           of tile in coord.
        */
	void tileCoord(const KisTileSP& tile, QPoint& coord);

        /**
           Returns the KisPixelData defined by the rectangle x1, y1,
           x2, y2. Depending on mode, this data is readable, writable
           or both.
        */
	KisPixelDataSP pixelData(Q_INT32 x1, Q_INT32 y1,
                                 Q_INT32 x2, Q_INT32 y2, 
                                 Q_INT32 mode);

	void releasePixelData(KisPixelDataSP pd);

	void readPixelData(KisPixelDataSP pd);

        /**
           Commit the pixel data in buffer.
        */
	void writePixelData(KisPixelDataSP pd);

	Q_INT32 tileNum(Q_UINT32 xpix, Q_UINT32 ypix) const;

	void readPixelData(Q_INT32 x1, Q_INT32 y1, Q_INT32 x2, Q_INT32 y2, QUANTUM *buffer, Q_UINT32 stride);
	void writePixelData(Q_INT32 x1, Q_INT32 y1, Q_INT32 x2, Q_INT32 y2, QUANTUM *buffer, Q_UINT32 stride);

private:
	KisTileMgr& operator=(const KisTileMgr&);
	void allocate(Q_INT32 ntiles);
	void duplicate(Q_INT32 ntiles, KisTileMgr *tm);

private:

        Q_UINT32 m_depth;
	Q_UINT32 m_width;
	Q_UINT32 m_height;
	Q_UINT32 m_ntileRows;
	Q_UINT32 m_ntileCols;
	vKisTileSP m_tiles;
	QMutex m_mutex;
	KisTileMediator *m_mediator;
};


inline Q_INT32 KisTileMgr::width() const
{
	return m_width;
}

inline Q_INT32 KisTileMgr::height() const
{
	return m_height;
}

inline Q_INT32 KisTileMgr::depth() const
{
	return m_depth;
}

inline Q_UINT32 KisTileMgr::nrows() const
{
	return m_ntileRows;
}

inline Q_UINT32 KisTileMgr::ncols() const
{
	return m_ntileCols;
}

inline bool KisTileMgr::empty() const
{
	return m_tiles.empty();
}

inline Q_INT32 KisTileMgr::tileNum(Q_UINT32 xpix, Q_UINT32 ypix) const
{
	if (xpix >= m_width || ypix >= m_height)
		return -1;

	return ypix / TILE_HEIGHT * m_ntileCols + xpix / TILE_WIDTH;
}


#endif // KISTILEMGR_H_
