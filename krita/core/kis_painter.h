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

#if !defined KIS_PAINTER_H_
#define KIS_PAINTER_H_

#include <qbrush.h>
#include <qcolor.h>
#include <qfontinfo.h>
#include <qfontmetrics.h>
#include <qpen.h>
#include <qpointarray.h>
#include <qregion.h>
#include <qregion.h>
#include <qwmatrix.h>
#include <qimage.h>
#include <qmap.h>
#include <qpixmap.h>
#include <qpointarray.h>
#include <qstring.h>
#include <qpainter.h>

#include <koColor.h>
#include <kcommand.h>

#include "kis_global.h"
#include "kis_types.h"
#include "kis_paint_device.h"
//#include "kis_gradient.h"
//#include "kis_brush.h"
//#include "kis_pattern.h"

class QRect;
class KisTileCommand;
class KisAlphaMask;
class KisBrush;
class KisPattern;
class KisGradient;

/*
  KisPainter contains the graphics primitives necessary to draw on a
  KisPaintDevice. This is the same kind of abstraction as used in Qt
  itself, where you have QPainter and QPaintDevice.

  However, KisPainter works on a tiled image and supports different
  colour models, and that's a lot more complicated.

  KisPainter supports transactions that can group various paint operations
  in one undoable step.

 */
class KisPainter {
public:
        KisPainter();
        KisPainter(KisPaintDeviceSP device);
        ~KisPainter();

public:
        // Start painting on the specified device.
        void begin(KisPaintDeviceSP device);

        // ???
        KCommand *end();

        // ???
        void beginTransaction(const QString& customName = QString::null);

        // ???
        KCommand *endTransaction();

        // The current paint device.
        KisPaintDeviceSP device() const;

        // ----------------------------------------------------------------------------------------
        // Native paint methods that are tile aware, undo/redo-able,
        // use the color strategies and the composite operations.

        // Blast the specified region from src onto the current paint device.
        void bitBlt(Q_INT32 dx, Q_INT32 dy, CompositeOp op, KisPaintDeviceSP src,
                    Q_INT32 sx = 0, Q_INT32 sy = 0, Q_INT32 sw = -1, Q_INT32 sh = -1);

        // Overloaded version of the previous, differs in that it is possible to specify
        // a value for opacity
        void bitBlt(Q_INT32 dx, Q_INT32 dy, CompositeOp op, KisPaintDeviceSP src,
                    QUANTUM opacity,
                    Q_INT32 sx = 0, Q_INT32 sy = 0, Q_INT32 sw = -1, Q_INT32 sh = -1);

        void eraseRect(Q_INT32 x1, Q_INT32 y1, Q_INT32 w, Q_INT32 h);
        void eraseRect(const QRect& rc);

	// ------------------------------------------------------------------------------------------
        // Fill a rectangle with a certain color
        void fillRect(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h, const KoColor& c);
        void fillRect(const QRect& rc, const KoColor& c);
        void fillRect(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h, const KoColor& c, QUANTUM opacity);
        void fillRect(const QRect& rc, const KoColor& c, QUANTUM opacity);

	// ------------------------------------------------------------------------------------------
	// The methods below are 'higher' level than the above methods. They need brushes, colors etc.
	// set before they can be called. The methods do not directly tell the image to update, but
	// you can call dirtyRect() to get the rect that needs to be notified by your painting code.
	QRect dirtyRect();

	// ------------------------------------------------------------------------------------------
	// Draw a line between pos1 and pos2 using the currently set brush and color. Returns
	// the drag distance, that is the remains of the distance between p1 and p2 not covered
	// because the currenlty set brush has a spacing greater than that distance.
	float paintLine(const QPoint &pos1,
			const QPoint &pos2,
			const Q_INT32 pressure,
			const Q_INT32 xTilt,
			const Q_INT32 yTilt,
			const float savedDist);

	// Draw a spot at pos using the currently set brush and color
	void paintAt(const QPoint &pos,
		     const Q_INT32 /*pressure*/,
		     const Q_INT32 /*xTilt*/,
		     const Q_INT32 /*yTilt*/);

	// ------------------------------------------------------------------------------------------
	// Set the parameters for the higher level graphics primitives. These primiti

	void setBrush(KisBrush* brush);
	void setPattern(KisPattern& pattern) { m_pattern = &pattern; }
	void setGradient(KisGradient& gradient) { m_gradient = &gradient; }
	void setPaintColor(const KoColor& color) {m_paintColor = color; }
	void setFillColor(const KoColor& color) { m_fillColor = color; }
	void setOpacity(QUANTUM opacity) { m_opacity = opacity; }


private:
        void tileBlt(QUANTUM *dst, KisTileSP dsttile, QUANTUM *src,
                     KisTileSP srctile, Q_INT32 rows, Q_INT32 cols,
                     CompositeOp op);
        void tileBlt(QUANTUM *dst, KisTileSP dsttile, QUANTUM *src,
                     KisTileSP srctile, QUANTUM opacity, Q_INT32 rows, Q_INT32 cols,
                     CompositeOp op);

        KisPainter(const KisPainter&);
        KisPainter& operator=(const KisPainter&);

	void computeDab(KisAlphaMask * mask);

private:
        KisPaintDeviceSP m_device;
        KisTileCommand  *m_transaction;

	QRect m_dirtyRect;

	KisLayerSP m_dab;
	KoColor m_paintColor;
	KoColor m_fillColor;
	KisBrush *m_brush;
	KisPattern *m_pattern;
	KisGradient *m_gradient;
	QUANTUM m_opacity;

	QPoint m_hotSpot;
        Q_INT32 m_hotSpotX;
        Q_INT32 m_hotSpotY;

        Q_INT32 m_brushWidth;
        Q_INT32 m_brushHeight;

	Q_INT32 m_lastPressure;
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

#endif // KIS_PAINTER_H_

