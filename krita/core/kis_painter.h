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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_PAINTER_H_
#define KIS_PAINTER_H_

#include <qbrush.h>
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

#include <kcommand.h>

#include "kis_color.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_paint_device_impl.h"
#include "kis_point.h"
#include "kis_matrix.h"
#include "kis_filter.h"
#include "kis_progress_subject.h"
#include "kis_paintop.h"
#include "kis_color.h"

#include <koffice_export.h>

class QRect;
class KisTransaction;
class KisBrush;
class KisPattern;

/**
 * KisPainter contains the graphics primitives necessary to draw on a
 * KisPaintDeviceImpl. This is the same kind of abstraction as used in Qt
 * itself, where you have QPainter and QPaintDevice.
 *
 * However, KisPainter works on a tiled image and supports different
 * colour models, and that's a lot more complicated.
 *
 * KisPainter supports transactions that can group various paint operations
 * in one undoable step.
 */
class KRITACORE_EXPORT KisPainter : public KisProgressSubject {
    typedef KisProgressSubject super;

public:
        KisPainter();
        KisPainter(KisPaintDeviceImplSP device);
    KisPainter(KisLayerSP device);
        virtual ~KisPainter();

private:
    // Implement KisProgressSubject
    virtual void cancel() { m_cancelRequested = true; }

public:
        /**
     * Start painting on the specified device. Not undoable.
     */
        void begin(KisPaintDeviceImplSP device);

        /**
     * Finish painting on the current device
     */
        KCommand *end();

        // Begin an undoable paint operation
        void beginTransaction(const QString& customName = QString::null);

        // Finish the undoable paint operation
        KCommand *endTransaction();

        // begin a transaction with the given command
        void beginTransaction( KisTransaction* command);

    // Return the current transcation
    KisTransaction  * transaction() { return m_transaction; }


        // The current paint device.
        KisPaintDeviceImplSP device() const { return m_device; } 


    // ------------------------------------------------------------------------------------------
    //  Native paint methods that are undo/redo-able,
        // uses the color strategies and the composite operations.
            
    /**
     * Blast the specified region from src onto the current paint device.
     */
        void bitBlt(Q_INT32 dx, Q_INT32 dy, 
            const KisCompositeOp& op, 
            KisPaintDeviceImplSP src,
                    Q_INT32 sx, Q_INT32 sy, 
            Q_INT32 sw, Q_INT32 sh)
    {
        bitBlt(dx, dy, op, src, OPACITY_OPAQUE, sx, sy, sw, sh);
    }

        /**
     * Overloaded version of the previous, differs in that it is possible to specify
         * a value for opacity
     */
        void bitBlt(Q_INT32 dx, Q_INT32 dy, 
            const KisCompositeOp& op, 
            KisPaintDeviceImplSP src,
                    QUANTUM opacity,
                    Q_INT32 sx, Q_INT32 sy, 
            Q_INT32 sw, Q_INT32 sh);


    /** A version of bitBlt that applies an external selection mask first to the source device and
     * only then blits. This means that the source device is permanently altered.
     */
    void bltSelection(Q_INT32 dx, Q_INT32 dy,
              const KisCompositeOp &op,
              KisPaintDeviceImplSP src,
              KisSelectionSP selMask,
              QUANTUM opacity,
              Q_INT32 sx, Q_INT32 sy,
              Q_INT32 sw, Q_INT32 sh);
 

    /**
     * A version of bitBlt that applies the destination selection mask
     * first to the source device and
     * only then blits. This means that the source device is permanently altered.
     */
    void bltSelection(Q_INT32 dx, Q_INT32 dy,
              const KisCompositeOp &op, 
              KisPaintDeviceImplSP src,
              QUANTUM opacity,
              Q_INT32 sx, Q_INT32 sy, 
              Q_INT32 sw, Q_INT32 sh);


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

    /**
     * Fill points with the points along the Bezier curve between pos1 and pos2 using control points 1 and 2,
     * excluding the final pos2.
     */
    void getBezierCurvePoints(const KisPoint &pos1,
                  const KisPoint &control1,
                  const KisPoint &control2,
                  const KisPoint &pos2,
                  vKisPoint& points);


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

    void paintPolygon(const vKisPoint& points);

    /** Draw a spot at pos using the currently set paint op, brush and color */
    void paintAt(const KisPoint &pos,
             const double pressure,
             const double /*xTilt*/,
             const double /*yTilt*/);


    // ------------------------------------------------------------------------------------------
    // Set the parameters for the higher level graphics primitives.

    void setBrush(KisBrush* brush) { m_brush = brush; }
    KisBrush * brush() const { return m_brush; }

    void setPattern(KisPattern * pattern) { m_pattern = pattern; }
    KisPattern * pattern() const { return m_pattern; }

    void setPaintColor(const KisColor& color) { m_paintColor = color;}
    KisColor paintColor() const { return m_paintColor; }

    void setBackgroundColor(const KisColor& color) {m_backgroundColor = color; }
    KisColor backgroundColor() const { return m_backgroundColor; }

    void setFillColor(const KisColor& color) { m_fillColor = color; }
    KisColor fillColor() const { return m_fillColor; }


    enum FillStyle {
        FillStyleNone,
        FillStyleForegroundColor,
        FillStyleBackgroundColor,
        FillStylePattern,
        FillStyleGradient,
        FillStyleStrokes
    };

    void setFillStyle(FillStyle fillStyle) { m_fillStyle = fillStyle; }
    FillStyle fillStyle() const { return m_fillStyle; }

    enum StrokeStyle {
        StrokeStyleNone,
        StrokeStyleBrush
    };

    void setStrokeStyle(StrokeStyle strokeStyle) { m_strokeStyle = strokeStyle; }
    StrokeStyle strokeStyle() const { return m_strokeStyle; }

    void setOpacity(QUANTUM opacity) { m_opacity = opacity; }
    QUANTUM opacity() const { return m_opacity; }

    void setCompositeOp(const KisCompositeOp& op) { m_compositeOp = op; }
    KisCompositeOp compositeOp() const { return m_compositeOp; }

    void setFilter(KisFilterSP filter) { m_filter = filter; }
    KisFilterSP filter() { return m_filter; }

    void setDuplicateOffset(const KisPoint offset) { m_duplicateOffset = offset; }
    KisPoint duplicateOffset(){ return m_duplicateOffset; }

    void setPressure(double pressure) { m_pressure = pressure; }
    double pressure() { return m_pressure; }
    
    void setPaintOp(KisPaintOp * paintOp) { m_paintOp = paintOp; }
    KisPaintOp * paintOp() const { return m_paintOp; }

    void setDab(KisPaintDeviceImplSP dab) { m_dab = dab; }
    KisPaintDeviceImplSP dab() const { return m_dab; }

    bool cancelRequested() const { return m_cancelRequested; }

protected:

    void init();
        KisPainter(const KisPainter&);
        KisPainter& operator=(const KisPainter&);

    static double pointToLineDistance(const KisPoint& p, const KisPoint& l0, const KisPoint& l1);

    void fillPolygon(const vKisPoint& points, FillStyle fillStyle);

protected:
        KisPaintDeviceImplSP m_device;
        KisTransaction  *m_transaction;

    QRect m_dirtyRect;

    KisColor m_paintColor;
    KisColor m_backgroundColor;
    KisColor m_fillColor;
    FillStyle m_fillStyle;
    StrokeStyle m_strokeStyle;
    KisBrush *m_brush;
    KisPattern *m_pattern;
    KisPoint m_duplicateOffset;
    QUANTUM m_opacity;
    KisCompositeOp m_compositeOp;
    KisFilterSP m_filter;
    KisPaintOp * m_paintOp;
    double m_pressure;
    bool m_cancelRequested;
    Q_INT32 m_pixelSize;
    KisAbstractColorSpace * m_colorSpace;
    KisProfileSP m_profile;
    KisPaintDeviceImplSP m_dab;

};


#endif // KIS_PAINTER_H_

