/*
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
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
#ifndef KIS_FILL_PAINTER_H_
#define KIS_FILL_PAINTER_H_

#include <qbrush.h>
#include <qcolor.h>
#include <qfontinfo.h>
#include <qfontmetrics.h>
#include <qpen.h>
#include <qregion.h>
#include <qregion.h>
#include <qwmatrix.h>
#include <qimage.h>
#include <qmap.h>
#include <qpixmap.h>
#include <qpointarray.h>
#include <qstring.h>
#include <qpainter.h>
#include <qvaluevector.h>

#include <koColor.h>
#include <kcommand.h>

#include "kis_global.h"
#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_point.h"
#include "kis_matrix.h"
#include "kis_progress_subject.h"
#include "kis_painter.h"

class KisFillPainter : public KisPainter
{

	typedef KisPainter super;

public:

        KisFillPainter();
        KisFillPainter(KisPaintDeviceSP device);


	/**
         * Fill a rectangle with black transparent pixels (0, 0, 0, 0 for RGBA).
	 */
        void eraseRect(Q_INT32 x1, Q_INT32 y1, Q_INT32 w, Q_INT32 h);
        void eraseRect(const QRect& rc);

	/**
         * Fill a rectangle with a certain color.
	 */
        void fillRect(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h, const KoColor& c);
        void fillRect(const QRect& rc, const KoColor& c);

	/**
         * Fill a rectangle with a certain color and opacity.
	 */
        void fillRect(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h, const KoColor& c, QUANTUM opacity);
        void fillRect(const QRect& rc, const KoColor& c, QUANTUM opacity);

private:


};


inline
void KisFillPainter::fillRect(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h, const KoColor& c)
{
        fillRect(x, y, w, h, c, OPACITY_OPAQUE);
}

inline
void KisFillPainter::fillRect(const QRect& rc, const KoColor& c)
{
        fillRect(rc.x(), rc.y(), rc.width(), rc.height(), c, OPACITY_OPAQUE);
}

inline
void KisFillPainter::eraseRect(Q_INT32 x1, Q_INT32 y1, Q_INT32 w, Q_INT32 h)
{
        fillRect(x1, y1, w, h, KoColor::black(), OPACITY_TRANSPARENT);
}

inline
void KisFillPainter::eraseRect(const QRect& rc)
{
        fillRect(rc, KoColor::black(), OPACITY_TRANSPARENT);
}

inline
void KisFillPainter::fillRect(const QRect& rc, const KoColor& c, QUANTUM opacity)
{
        fillRect(rc.x(), rc.y(), rc.width(), rc.height(), c, opacity);
}



#endif //KIS_FILL_PAINTER_H_
