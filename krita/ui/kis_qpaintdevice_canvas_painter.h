/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
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

#ifndef KIS_QPAINTDEVICE_CANVAS_PAINTER_H_
#define KIS_QPAINTDEVICE_CANVAS_PAINTER_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qwidget.h>
#include <qpainter.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3PointArray>

#include "kis_global.h"
#include "kis_canvas_painter.h"

class KisQPaintDeviceCanvasPainter : public KisCanvasWidgetPainter {
public:
    KisQPaintDeviceCanvasPainter();
    KisQPaintDeviceCanvasPainter(const QPaintDevice *paintDevice);
    virtual ~KisQPaintDeviceCanvasPainter();

    bool begin(const QPaintDevice* paintDevice, bool unclipped = false);

    virtual bool begin(KisCanvasWidget *canvasWidget, bool unclipped = false);
    virtual bool end();

    virtual void save();
    virtual void restore();

    virtual QFontMetrics fontMetrics() const;
    virtual QFontInfo fontInfo() const;

    virtual const QFont& font() const;
    virtual void setFont(const QFont&);
    virtual const QPen& pen() const;
    virtual void setPen(const QPen&);
    virtual void setPen(Qt::PenStyle);
    virtual void setPen(const QColor&);
    virtual const QBrush&brush() const;
    virtual void setBrush(const QBrush&);
    virtual void setBrush(Qt::BrushStyle);
    virtual void setBrush(const QColor&);
    virtual QPoint pos() const;

    virtual const QColor&backgroundColor() const;
    virtual void setBackgroundColor(const QColor&);
    virtual Qt::BGMode backgroundMode() const;
    virtual void setBackgroundMode(Qt::BGMode);
    //virtual Qt::RasterOp rasterOp()	const;
    //virtual void setRasterOp(Qt::RasterOp);
    virtual const QPoint&brushOrigin() const;
    virtual void setBrushOrigin(int x, int y);
    virtual void setBrushOrigin(const QPoint&);

    virtual bool hasViewXForm() const;
    virtual bool hasWorldXForm() const;

    virtual void setViewXForm(bool);
    virtual QRect window() const;
    virtual void setWindow(const QRect&);
    virtual void setWindow(int x, int y, int w, int h);
    virtual QRect viewport() const;
    virtual void setViewport(const QRect&);
    virtual void setViewport(int x, int y, int w, int h);

    virtual void setWorldXForm(bool);
    virtual const QMatrix&worldMatrix() const;
    virtual void setMatrix(const QMatrix&, bool combine=FALSE);

    virtual void saveWorldMatrix();
    virtual void restoreWorldMatrix();

    virtual void scale(double sx, double sy);
    virtual void shear(double sh, double sv);
    virtual void rotate(double a);

    virtual void translate(double dx, double dy);
    virtual void resetXForm();
    virtual double translationX() const;
    virtual double translationY() const;

    virtual QPoint xForm(const QPoint&) const;
    virtual QRect xForm(const QRect&)	const;
    virtual Q3PointArray xForm(const Q3PointArray&) const;
    virtual Q3PointArray xForm(const Q3PointArray&, int index, int npoints) const;
    virtual QPoint xFormDev(const QPoint&) const;
    virtual QRect xFormDev(const QRect&)  const;
    virtual Q3PointArray xFormDev(const Q3PointArray&) const;
    virtual Q3PointArray xFormDev(const Q3PointArray&, int index, int npoints) const;

    virtual void setClipping(bool);
    virtual bool hasClipping() const;
    virtual QRegion clipRegion(QPainter::CoordinateMode = QPainter::CoordDevice) const;
    virtual void setClipRect(const QRect&, QPainter::CoordinateMode = QPainter::CoordDevice);
    virtual void setClipRect(int x, int y, int w, int h, QPainter::CoordinateMode = QPainter::CoordDevice);
    virtual void setClipRegion(const QRegion&, QPainter::CoordinateMode = QPainter::CoordDevice);

    virtual void drawPoint(int x, int y);
    virtual void drawPoint(const QPoint&);
    virtual void drawPoints(const Q3PointArray& a, int index=0, int npoints=-1);
    virtual void moveTo(int x, int y);
    virtual void moveTo(const QPoint&);
    virtual void lineTo(int x, int y);
    virtual void lineTo(const QPoint&);
    virtual void drawLine(int x1, int y1, int x2, int y2);
    virtual void drawLine(const QPoint&, const QPoint&);
    virtual void drawRect(int x, int y, int w, int h);
    virtual void drawRect(const QRect&);
    virtual void drawWinFocusRect(int x, int y, int w, int h);
    virtual void drawWinFocusRect(int x, int y, int w, int h, const QColor&bgColor);
    virtual void drawWinFocusRect(const QRect&);
    virtual void drawWinFocusRect(const QRect&, const QColor&bgColor);
    virtual void drawRoundRect(int x, int y, int w, int h, int = 25, int = 25);
    virtual void drawRoundRect(const QRect&, int = 25, int = 25);
    virtual void drawEllipse(int x, int y, int w, int h);
    virtual void drawEllipse(const QRect&);
    virtual void drawArc(int x, int y, int w, int h, int a, int alen);
    virtual void drawArc(const QRect&, int a, int alen);
    virtual void drawPie(int x, int y, int w, int h, int a, int alen);
    virtual void drawPie(const QRect&, int a, int alen);
    virtual void drawChord(int x, int y, int w, int h, int a, int alen);
    virtual void drawChord(const QRect&, int a, int alen);
    virtual void drawLineSegments(const Q3PointArray&, int index=0, int nlines=-1);
    virtual void drawPolyline(const Q3PointArray&, int index=0, int npoints=-1);
    virtual void drawPolygon(const Q3PointArray&, bool winding=FALSE, int index=0, int npoints=-1);
    virtual void drawConvexPolygon(const Q3PointArray&, int index=0, int npoints=-1);
    virtual void drawCubicBezier(const Q3PointArray&, int index=0);
    virtual void drawPixmap(int x, int y, const QPixmap&, int sx=0, int sy=0, int sw=-1, int sh=-1);
    virtual void drawPixmap(const QPoint&, const QPixmap&, const QRect&sr);
    virtual void drawPixmap(const QPoint&, const QPixmap&);
    virtual void drawPixmap(const QRect&, const QPixmap&);
    virtual void drawImage(int x, int y, const QImage&, int sx = 0, int sy = 0, int sw = -1, int sh = -1, int conversionFlags = 0);
    virtual void drawImage(const QPoint&, const QImage&, const QRect&sr, int conversionFlags = 0);
    virtual void drawImage(const QPoint&, const QImage&, int conversion_flags = 0);
    virtual void drawImage(const QRect&, const QImage&);
    virtual void drawTiledPixmap(int x, int y, int w, int h, const QPixmap&, int sx=0, int sy=0);
    virtual void drawTiledPixmap(const QRect&, const QPixmap&, const QPoint&);
    virtual void drawTiledPixmap(const QRect&, const QPixmap&);
    //virtual void drawPicture(const QPicture&);
    //virtual void drawPicture(int x, int y, const QPicture&);
    //virtual void drawPicture(const QPoint&, const QPicture&);

    virtual void fillRect(int x, int y, int w, int h, const QBrush&);
    virtual void fillRect(const QRect&, const QBrush&);
    virtual void eraseRect(int x, int y, int w, int h);
    virtual void eraseRect(const QRect&);

    virtual void drawText(int x, int y, const QString&, int len = -1, QPainter::TextDirection dir = QPainter::Auto);
    virtual void drawText(const QPoint&, const QString&, int len = -1, QPainter::TextDirection dir = QPainter::Auto);

    virtual void drawText(int x, int y, const QString&, int pos, int len, QPainter::TextDirection dir = QPainter::Auto);
    virtual void drawText(const QPoint&p, const QString&, int pos, int len, QPainter::TextDirection dir = QPainter::Auto);

    virtual void drawText(int x, int y, int w, int h, int flags, const QString&, int len = -1, QRect *br=0, QTextParag **intern=0);
    virtual void drawText(const QRect&, int flags, const QString&, int len = -1, QRect *br=0, QTextParag **intern=0);

    virtual void drawTextItem(int x, int y, const QTextItem&ti, int textflags = 0);
    virtual void drawTextItem(const QPoint& p, const QTextItem&ti, int textflags = 0);

    virtual QRect boundingRect(int x, int y, int w, int h, int flags, const QString&, int len = -1, QTextParag **intern=0);
    virtual QRect boundingRect(const QRect&, int flags, const QString&, int len = -1, QTextParag **intern=0);

    virtual int	tabStops() const;
    virtual void setTabStops(int);
    virtual int	*tabArray() const;
    virtual void setTabArray(int *);

protected:
    QPainter m_painter;
};

#endif // KIS_QPAINTDEVICE_CANVAS_PAINTER_H_

