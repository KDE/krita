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
#if !defined KISTILE_H_
#define KISTILE_H_

#include <qcstring.h>
#include <qimage.h>
#include <qglobal.h>
#include <qmutex.h>
#include <qvaluevector.h>
#include <ksharedptr.h>
#include "kis_types.h"

class KisTileCacheInterface;
class KisTileSwapInterface;

/**
 * Provides abstraction to a tile.  A tile contains
 * a part of a layer.  Layers form an image.
 */
class KisTile : public KShared {
	typedef KShared super;

public:
	enum drawingHints { broken, opaque, transparent, mixed, outofrange, undef, unknown };

public:
	KisTile(Q_INT32 depth, KisTileCacheInterface *cache = 0, KisTileSwapInterface *swap = 0);
	KisTile(KisTile& rhs);
	virtual ~KisTile();

public:
	QMutex *mutex();
	void lock();
	void lockAsync();
	void release();
	void allocate();
	QUANTUM *data(Q_INT32 xoff = 0, Q_INT32 yoff = 0) const;
	Q_INT32 width() const;
	void width(Q_INT32 w);
	Q_INT32 height() const;
	void height(Q_INT32 h);
	Q_INT32 depth() const;
	Q_INT32 size() const;
	bool dirty() const;
	void dirty(bool val);
	bool valid() const;
	void valid(bool valid);
	Q_INT32 rowHint(Q_INT32 row) const;
	void setRowHint(Q_INT32 row, drawingHints hint);
	Q_INT32 refCount() const;
	void ref();
	Q_INT32 shareCount() const;
	void shareRef();
	void shareRelease();
	Q_INT32 writeCount() const;
	void writeRef();
	QImage convertToImage();

private:
	KisTile& operator=(const KisTile&);
	void init(Q_INT32 depth, KisTileCacheInterface *cache, KisTileSwapInterface *swap);
	void initRowHints();

private:
	bool m_dirty;
	bool m_valid;
	QUANTUM *m_data;
	Q_INT32 m_width;
	Q_INT32 m_height;
	Q_INT32 m_depth;
	QMutex m_mutex;
	QValueVector<drawingHints> m_hints;
	KisTileSwapInterface *m_swap;
	KisTileCacheInterface *m_cache;
	Q_INT32 m_swapNo;
	Q_INT32 m_nref;
	Q_INT32 m_nshare;
	Q_INT32 m_nwrite;
};

#endif // KISTILE_H_

