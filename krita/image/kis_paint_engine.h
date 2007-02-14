/*
 *  copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_PAINT_ENGINE
#define KIS_PAINT_ENGINE

#include <QPaintEngine>

class QPaintDevice;
class QPaintEngineState;
class QRect;
class QRectF;
class QLine;
class QLineF;
class QPainterPath;
class QPoint;
class QPointF;
class QPixmap;
class QTextTiem;
class QImage;

/**
   KisPaintEngine is an implementation of Qt's paint engine that works
   on KisPaintDevices. KisPaintDevices generally have a tiled data
   backend and can have any colorspace as defined in Pigment

   @see KisPaintDevice
   @see KoColorSpace
*/
class KisPaintEngine : public QPaintEngine
{
public:

    KisPaintEngine();
    ~KisPaintEngine();

    Type type() const {
        return User;
    }

    bool begin(QPaintDevice *pdev);
    bool end();

    void updateState(const QPaintEngineState &state);

    void drawRects(const QRect *rects, int rectCount);
    void drawRects(const QRectF *rects, int rectCount);

    void drawLines(const QLine *lines, int lineCount);
    void drawLines(const QLineF *lines, int lineCount);

    void drawEllipse(const QRectF &r);
    void drawEllipse(const QRect &r);

    void drawPath(const QPainterPath &path);

    void drawPoints(const QPointF *points, int pointCount);
    void drawPoints(const QPoint *points, int pointCount);

    void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode);
    void drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode);

    void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr);
    void drawTextItem(const QPointF &p, const QTextItem &textItem);
    void drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s);
    void drawImage(const QRectF &r, const QImage &pm, const QRectF &sr,
                   Qt::ImageConversionFlags flags = Qt::AutoColor);

private:

    class KisPaintEnginePrivate;
    KisPaintEnginePrivate * d;

    void updatePen (const QPen &newPen);
    void updateBrush (const QBrush &newBrush, const QPointF& newOrigin);
    void initPainter ();
    // TODO: updateFont(), clipping...

};

#endif
