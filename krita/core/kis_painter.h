/*
 *  Copyright (c) 1999 Michael Koch <koch@kde.org>
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
#if !defined KIS_PAINTER_H_
#define KIS_PAINTER_H_

#include <qstring.h>
#include "kis_global.h"
#include "kis_types.h"

class QRect;
class KCommand;
class KMacroCommand;

class KisPainter {
public:
	KisPainter();
	KisPainter(KisPaintDeviceSP device);
	~KisPainter();

public:
	void begin(KisPaintDeviceSP device);
	KCommand *end();
	void beginTransaction(KMacroCommand *command);
	void beginTransaction(const QString& customName = QString::null);
	KCommand *endTransaction();
	void bitBlt(Q_INT32 dx, Q_INT32 dy, CompositeOp op, KisPixelDataSP src, Q_INT32 sx = 0, Q_INT32 sy = 0, Q_INT32 sw = -1, Q_INT32 sh = -1);
	void bitBlt(Q_INT32 dx, Q_INT32 dy, CompositeOp op, KisPixelDataSP src, QUANTUM opacity, Q_INT32 sx = 0, Q_INT32 sy = 0, Q_INT32 sw = -1, Q_INT32 sh = -1);
	void bitBlt(Q_INT32 dx, Q_INT32 dy, CompositeOp op, KisPaintDeviceSP src, Q_INT32 sx = 0, Q_INT32 sy = 0, Q_INT32 sw = -1, Q_INT32 sh = -1);
	void bitBlt(Q_INT32 dx, Q_INT32 dy, CompositeOp op, KisPaintDeviceSP src, QUANTUM opacity, Q_INT32 sx = 0, Q_INT32 sy = 0, Q_INT32 sw = -1, Q_INT32 sh = -1);
	KisPaintDeviceSP device() const;
	void eraseRect(Q_INT32 x1, Q_INT32 y1, Q_INT32 w, Q_INT32 h);
	void eraseRect(const QRect& rc);
	void fillRect(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h, const KoColor& c);
	void fillRect(const QRect& rc, const KoColor& c);
	void fillRect(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h, const KoColor& c, QUANTUM opacity);
	void fillRect(const QRect& rc, const KoColor& c, QUANTUM opacity);

private:
	void tileBlt(QUANTUM *dst, KisTileSP dsttile, QUANTUM *src, KisTileSP srctile, Q_INT32 rows, Q_INT32 cols, CompositeOp op);
	void tileBlt(QUANTUM *dst, KisTileSP dsttile, QUANTUM *src, KisTileSP srctile, QUANTUM opacity, Q_INT32 rows, Q_INT32 cols, CompositeOp op);
	KisPainter(const KisPainter&);
	KisPainter& operator=(const KisPainter&);

private:
	KisPaintDeviceSP m_device;	
	KMacroCommand *m_transaction;
};

inline
void KisPainter::fillRect(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h, const KoColor& c)
{
	fillRect(x, y, w, h, c, OPACITY_OPAQUE);
}

inline
void KisPainter::fillRect(const QRect& rc, const KoColor& c)
{
	fillRect(rc.x(), rc.y(), rc.width(), rc.height(), c, OPACITY_OPAQUE);
}

inline
void KisPainter::eraseRect(Q_INT32 x1, Q_INT32 y1, Q_INT32 w, Q_INT32 h)
{
	fillRect(x1, y1, w, h, KoColor::black(), OPACITY_TRANSPARENT);
}

inline
void KisPainter::eraseRect(const QRect& rc)
{
	fillRect(rc, KoColor::black(), OPACITY_TRANSPARENT);
}

inline
void KisPainter::fillRect(const QRect& rc, const KoColor& c, QUANTUM opacity)
{
	fillRect(rc.x(), rc.y(), rc.width(), rc.height(), c, opacity);
}

inline
KisPaintDeviceSP KisPainter::device() const
{
	return m_device;
}

#endif // KIS_PAINTER_H_-

