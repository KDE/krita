/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
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

#ifndef KIS_PAINTER_H_
#define KIS_PAINTER_H_

#include <qbrush.h>
#include <qcolor.h>
#include <qfontinfo.h>
#include <qfontmetrics.h>
#include <qpen.h>
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
#include "kis_filter.h"
#include "kis_progress_subject.h"
#include "kis_paintop.h"

class QRect;
class KisTileCommand;
class KisBrush;
class KisPattern;

/**
  KisPainter contains the graphics primitives necessary to draw on a
  KisPaintDevice. This is the same kind of abstraction as used in Qt
  itself, where you have QPainter and QPaintDevice.

  However, KisPainter works on a tiled image and supports different
  colour models, and that's a lot more complicated.

  KisPainter supports transactions that can group various paint operations
  in one undoable step.

 */
class KisPainter : public KisProgressSubject {
	typedef KisProgressSubject super;

public:
        KisPainter();
        KisPainter(KisPaintDeviceSP device);
        virtual ~KisPainter();

	/**
	 * The methods below are 'higher' level than the above methods. They need brushes, colors etc.
	 * set before they can be called. The methods do not directly tell the image to update, but
	 * you can call dirtyRect() to get the rect that needs to be notified by your painting code.
	 *
	 * Call will reset it!
	*/
	QRect dirtyRect();

	/**
	 * Add the r to the current dirty rect.
	 */
	QRect addDirtyRect(QRect r) { m_dirtyRect |= r; return m_dirtyRect; }

private:
	// Implement KisProgressSubject
	virtual void cancel() { m_cancelRequested = true; }

public:
        /**
	 * Start painting on the specified device. Not undoable.
	 */
        void begin(KisPaintDeviceSP device);

        /**
	 * Finish painting on the current device
	 */
        KCommand *end();

        // Begin an undoable paint operation
        void beginTransaction(const QString& customName = QString::null);

        // Finish the undoable paint operation
        KCommand *endTransaction();

        // begin a transaction with the given command
        void beginTransaction( KisTileCommand* command);

	// Return the current transcation
	KisTileCommand  * transaction() { return m_transaction; }


        // The current paint device.
        KisPaintDeviceSP device() const { return m_device; } 


	// ------------------------------------------------------------------------------------------
	//  Native paint methods that are tile aware, undo/redo-able,
        // use the color strategies and the composite operations.

	/**
	 * Blast the specified region from src onto the current paint device.
	 */
        void bitBlt(Q_INT32 dx, Q_INT32 dy, 
		    CompositeOp op, 
		    KisPaintDeviceSP src,
                    Q_INT32 sx = 0, Q_INT32 sy = 0, 
		    Q_INT32 sw = -1, Q_INT32 sh = -1);

        /**
	 * Overloaded version of the previous, differs in that it is possible to specify
         * a value for opacity
	 */
        void bitBlt(Q_INT32 dx, Q_INT32 dy, 
		    CompositeOp op, 
		    KisPaintDeviceSP src,
                    QUANTUM opacity,
                    Q_INT32 sx = 0, Q_INT32 sy = 0, 
		    Q_INT32 sw = -1, Q_INT32 sh = -1);


	/**
	 * Paint a line that connects the dots in points
	 */
        void paintPolyline (const QValueVector <KisPoint> &points,
                            int index = 0, int numPoints = -1);

	/**
	 * Draw a line between pos1 and pos2 using the currently set brush and color.
	 * If savedDist is less than zero, the brush is painted at pos1 before being
	 * painted along the line using the spacing setting.
	 * @return the drag distance, that is the remains of the distance between p1 and p2 not covered
	 * because the currenlty set brush has a spacing greater than that distance.
	 */
	double paintLine(const KisPoint &pos1,
			 const double pressure1,
			 const double xTilt1,
			 const double yTilt1,
			 const KisPoint &pos2,
			 const double pressure2,
			 const double xTilt2,
			 const double yTilt2,
			 const double savedDist = -1);
	/**
	 * Draw a Bezier curve between pos1 and pos2 using control points 1 and 2.
	 * If savedDist is less than zero, the brush is painted at pos1 before being
	 * painted along the curve using the spacing setting.
	 * @return the drag distance, that is the remains of the distance between p1 and p2 not covered
	 * because the currenlty set brush has a spacing greater than that distance.
	 */
	double paintBezierCurve(const KisPoint &pos1,
				const double pressure1,
				const double xTilt1,
				const double yTilt1,
				const KisPoint &control1,
				const KisPoint &control2,
				const KisPoint &pos2,
				const double pressure2,
				const double xTilt2,
				const double yTilt2,
				const double savedDist = -1);


        void paintRect(const KisPoint &startPoint,
		       const KisPoint &endPoint,
		       const double pressure,
		       const double xTilt,
		       const double yTilt);


        void paintEllipse(const KisPoint &startPoint,
                          const KisPoint &endPoint,
                          const double pressure,
			  const double /*xTilt*/,
			  const double /*yTilt*/);


	/** Draw a spot at pos using the currently set paint op, brush and color */
	void paintAt(const KisPoint &pos,
		     const double pressure,
		     const double /*xTilt*/,
		     const double /*yTilt*/);


	// ------------------------------------------------------------------------------------------
	// Set the parameters for the higher level graphics primitives.

	void setBrush(KisBrush* brush) { m_brush = brush; }
	KisBrush * brush() const { return m_brush; }

	void setPattern(KisPattern& pattern) { m_pattern = &pattern; }
	KisPattern * pattern() const { return m_pattern; }

	void setPaintColor(const KoColor& color) {m_paintColor = color; }
	KoColor paintColor() const { return m_paintColor; }

	void setBackgroundColor(const KoColor& color) {m_backgroundColor = color; }
	KoColor backgroundColor() const { return m_backgroundColor; }

	void setFillColor(const KoColor& color) { m_fillColor = color; }
	KoColor fillColor() const { return m_fillColor; }

	void setOpacity(QUANTUM opacity) { m_opacity = opacity; }
	QUANTUM opacity() const { return m_opacity; }

	void setCompositeOp(CompositeOp op) { m_compositeOp = op; }
	CompositeOp compositeOp() const { return m_compositeOp; }

	void setFilter(KisFilterSP filter) { m_filter = filter; }
	KisFilterSP filter() { return m_filter; }

	void setDuplicateOffset(const KisPoint offset) { m_duplicateOffset = offset; }
	KisPoint duplicateOffset(){ return m_duplicateOffset; }

	void setPressure(double pressure) { m_pressure = pressure; }
	double pressure() { return m_pressure; }
	
	void setPaintOp(KisPaintOp * paintOp) { m_paintOp = paintOp; }
	KisPaintOp * paintOp() const { return m_paintOp; }

	void setDab(KisPaintDeviceSP dab) { m_dab = dab; }
	KisPaintDeviceSP dab() const { return m_dab; }

protected:

	void init();
        KisPainter(const KisPainter&);
        KisPainter& operator=(const KisPainter&);

	static double pointToLineDistance(const KisPoint& p, const KisPoint& l0, const KisPoint& l1);

//         void tileBlt(QUANTUM *dst, KisTileSP dsttile, 
// 		     KisStrategyColorSpaceSP srcSpace, QUANTUM *src, KisTileSP srctile, 
// 		     Q_INT32 rows, Q_INT32 cols,
//                      CompositeOp op);


        void tileBlt(QUANTUM *dst, 
		     KisTileSP dsttile, 
		     KisStrategyColorSpaceSP srcSpace, 
		     QUANTUM *src, 
		     KisTileSP srctile, 
                     Q_INT32 rows, Q_INT32 cols,
		     QUANTUM opacity, 
                     CompositeOp op);



private:

        void paintEllipsePixel (bool invert,
                                int xc, int yc, int x1, int y1, int x2, int y2,
                                const double pressure);

        void paintEllipseSymmetry (double ratio, bool invert,
                                   int x, int y, int xc, int yc,
                                   const double pressure);

        void paintEllipseInternal (double ratio, bool invert,
                                   int xc, int yc, int radius,
                                   const double pressure);


protected:
        KisPaintDeviceSP m_device;
        KisTileCommand  *m_transaction;

	QRect m_dirtyRect;

	KoColor m_paintColor;
	KoColor m_backgroundColor;
	KoColor m_fillColor;
	KisBrush *m_brush;
	KisPattern *m_pattern;
	KisPoint m_duplicateOffset;
	QUANTUM m_opacity;
	CompositeOp m_compositeOp;
	KisFilterSP m_filter;
	KisPaintOp * m_paintOp;
	double m_pressure;
	bool m_cancelRequested;
	KisPaintDeviceSP m_dab;
};


#endif // KIS_PAINTER_H_

