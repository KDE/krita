/*
 *  Copyright (c) 1999 Michael Koch <koch@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@ideasandassociates.com>
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
#if !defined KIS_PAINTER_H_
#define KIS_PAINTER_H_

#include "kis_global.h"
#include "kis_types.h"

class QRect;

class KisPainter {
public:
	KisPainter();
	KisPainter(KisTileSP tile);
	KisPainter(KisPixelDataSP pd);
	KisPainter(KisPaintDeviceSP device);
	KisPainter(KisPaintDeviceSP device, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);
	KisPainter(KisPaintDeviceSP device, const QRect& rc);
	~KisPainter();

public:
	void begin(KisTileSP tile);
	void begin(KisPixelDataSP pd);
	void begin(KisPaintDeviceSP device);
	void begin(KisPaintDeviceSP device, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);
	void begin(KisPaintDeviceSP device, const QRect& rc);
	void end();
	void bitBlt(Q_INT32 dx, Q_INT32 dy, CompositeOp op, KisTileSP src, Q_INT32 sx = 0, Q_INT32 sy = 0, Q_INT32 sw = -1, Q_INT32 sh = -1);
	void bitBlt(Q_INT32 dx, Q_INT32 dy, CompositeOp op, KisPixelDataSP src, Q_INT32 sx = 0, Q_INT32 sy = 0, Q_INT32 sw = -1, Q_INT32 sh = -1);
	void bitBlt(Q_INT32 dx, Q_INT32 dy, CompositeOp op, KisPaintDeviceSP src, Q_INT32 sx = 0, Q_INT32 sy = 0, Q_INT32 sw = -1, Q_INT32 sh = -1);
	void bitBlt(Q_INT32 dx, Q_INT32 dy, CompositeOp op, KisTileSP src, QUANTUM opacity, Q_INT32 sx = 0, Q_INT32 sy = 0, Q_INT32 sw = -1, Q_INT32 sh = -1);
	void fillRect(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h, const KoColor& c);
	void fillRect(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h, const KoColor& c, QUANTUM opacity);

private:
	KisPainter(const KisPainter&);
	KisPainter& operator=(const KisPainter&);

private:
	KisPixelDataSP m_dst;
};

#endif // KIS_PAINTER_H_-

