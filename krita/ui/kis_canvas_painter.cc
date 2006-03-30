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
 *  GNU General Public License for more details.g
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_canvas.h"
#include "kis_canvas_painter.h"
#include "kis_qpaintdevice_canvas_painter.h"
//Added by qt3to4:
#include <QPixmap>
#include <Q3PointArray>

KisCanvasWidgetPainter::KisCanvasWidgetPainter()
{
}

KisCanvasWidgetPainter::~KisCanvasWidgetPainter()
{
}

bool KisCanvasWidgetPainter::end()
{
    return true;
}

void KisCanvasWidgetPainter::save()
{
}

void KisCanvasWidgetPainter::restore()
{
}

QFontMetrics KisCanvasWidgetPainter::fontMetrics() const
{
    return QFontMetrics(QFont());
}

QFontInfo KisCanvasWidgetPainter::fontInfo() const
{
    return QFontInfo(QFont());
}

const QFont& KisCanvasWidgetPainter::font() const
{
    return m_defaultFont;
}

void KisCanvasWidgetPainter::setFont(const QFont& /*font*/)
{
}

const QPen& KisCanvasWidgetPainter::pen() const
{
    return m_defaultPen;
}

void KisCanvasWidgetPainter::setPen(const QPen& /*pen*/)
{
}

void KisCanvasWidgetPainter::setPen(Qt::PenStyle /*penStyle*/)
{
}

void KisCanvasWidgetPainter::setPen(const QColor& /*color*/)
{
}

const QBrush& KisCanvasWidgetPainter::brush() const
{
    return m_defaultBrush;
}

void KisCanvasWidgetPainter::setBrush(const QBrush& /*brush*/)
{
}

void KisCanvasWidgetPainter::setBrush(Qt::BrushStyle /*brushStyle*/)
{
}

void KisCanvasWidgetPainter::setBrush(const QColor& /*color*/)
{
}

QPoint KisCanvasWidgetPainter::pos() const
{
    return QPoint();
}

const QColor& KisCanvasWidgetPainter::backgroundColor() const
{
    return m_defaultColor;
}

void KisCanvasWidgetPainter::setBackgroundColor(const QColor& /*color*/)
{
}

Qt::Qt::BGMode KisCanvasWidgetPainter::backgroundMode() const
{
    return Qt::TransparentMode;
}

void KisCanvasWidgetPainter::setBackgroundMode(Qt::Qt::BGMode /*bgMode*/)
{
}

Qt::Qt::RasterOp KisCanvasWidgetPainter::rasterOp() const
{
    return Qt::CopyROP;
}

void KisCanvasWidgetPainter::setRasterOp(Qt::RasterOp /*rasterOp*/)
{
}

const QPoint& KisCanvasWidgetPainter::brushOrigin() const
{
    return m_defaultBrushOrigin;
}

void KisCanvasWidgetPainter::setBrushOrigin(int /*x*/, int /*y*/)
{
}

void KisCanvasWidgetPainter::setBrushOrigin(const QPoint& /*origin*/)
{
}

bool KisCanvasWidgetPainter::hasViewXForm() const
{
    return false;
}

bool KisCanvasWidgetPainter::hasWorldXForm() const
{
    return false;
}

void KisCanvasWidgetPainter::setViewXForm(bool /*enable*/)
{
}

QRect KisCanvasWidgetPainter::window() const
{
    return QRect();
}

void KisCanvasWidgetPainter::setWindow(const QRect& /*r*/)
{
}

void KisCanvasWidgetPainter::setWindow(int /*x*/, int /*y*/, int /*w*/, int /*h*/)
{
}

QRect KisCanvasWidgetPainter::viewport() const
{
    return QRect();
}

void KisCanvasWidgetPainter::setViewport(const QRect& /*r*/)
{
}

void KisCanvasWidgetPainter::setViewport(int /*x*/, int /*y*/, int /*w*/, int /*h*/)
{
}


void KisCanvasWidgetPainter::setWorldXForm(bool /*enable*/)
{
}

const QMatrix& KisCanvasWidgetPainter::worldMatrix() const
{
    return m_defaultWorldMatrix;
}

void KisCanvasWidgetPainter::setMatrix(const QMatrix& /*matrix*/, bool /*combine*/)
{
}

void KisCanvasWidgetPainter::saveWorldMatrix()
{
}

void KisCanvasWidgetPainter::restoreWorldMatrix()
{
}

void KisCanvasWidgetPainter::scale(double /*sx*/, double /*sy*/)
{
}

void KisCanvasWidgetPainter::shear(double /*sh*/, double /*sv*/)
{
}

void KisCanvasWidgetPainter::rotate(double /*a*/)
{
}

void KisCanvasWidgetPainter::translate(double /*dx*/, double /*dy*/)
{
}

void KisCanvasWidgetPainter::resetXForm()
{
}

double KisCanvasWidgetPainter::translationX() const
{
    return 0;
}

double KisCanvasWidgetPainter::translationY() const
{
    return 0;
}

QPoint KisCanvasWidgetPainter::xForm(const QPoint& point) const
{
    return point;
}

QRect KisCanvasWidgetPainter::xForm(const QRect& r) const
{
    return r;
}

Q3PointArray KisCanvasWidgetPainter::xForm(const Q3PointArray& pointArray) const
{
    return pointArray;
}

Q3PointArray KisCanvasWidgetPainter::xForm(const Q3PointArray& pointArray, int /*index*/, int /*npoints*/) const
{
    return pointArray;
}

QPoint KisCanvasWidgetPainter::xFormDev(const QPoint& point) const
{
    return point;
}

QRect KisCanvasWidgetPainter::xFormDev(const QRect& r) const
{
    return r;
}

Q3PointArray KisCanvasWidgetPainter::xFormDev(const Q3PointArray& pointArray) const
{
    return pointArray;
}

Q3PointArray KisCanvasWidgetPainter::xFormDev(const Q3PointArray& pointArray, int /*index*/, int /*npoints*/) const
{
    return pointArray;
}

void KisCanvasWidgetPainter::setClipping(bool /*enable*/)
{
}

bool KisCanvasWidgetPainter::hasClipping() const
{
    return true;
}

QRegion KisCanvasWidgetPainter::clipRegion(QPainter::CoordinateMode /*mode*/) const
{
    return QRegion();
}

void KisCanvasWidgetPainter::setClipRect(const QRect& /*r*/, QPainter::CoordinateMode /*mode*/)
{
}

void KisCanvasWidgetPainter::setClipRect(int /*x*/, int /*y*/, int /*w*/, int /*h*/, QPainter::CoordinateMode /*mode*/)
{
}

void KisCanvasWidgetPainter::setClipRegion(const QRegion& /*rgn*/, QPainter::CoordinateMode /*mode*/)
{
}

void KisCanvasWidgetPainter::drawPoint(int /*x*/, int /*y*/)
{
}

void KisCanvasWidgetPainter::drawPoint(const QPoint& /*point*/)
{
}

void KisCanvasWidgetPainter::drawPoints(const Q3PointArray& /*pointArray*/, int /*index*/, int /*npoints*/)
{
}

void KisCanvasWidgetPainter::moveTo(int /*x*/, int /*y*/)
{
}

void KisCanvasWidgetPainter::moveTo(const QPoint& /*point*/)
{
}

void KisCanvasWidgetPainter::lineTo(int /*x*/, int /*y*/)
{
}

void KisCanvasWidgetPainter::lineTo(const QPoint& /*point*/)
{
}

void KisCanvasWidgetPainter::drawLine(int /*x1*/, int /*y1*/, int /*x2*/, int /*y2*/)
{
}

void KisCanvasWidgetPainter::drawLine(const QPoint& /*start*/, const QPoint& /*end*/)
{
}

void KisCanvasWidgetPainter::drawRect(int /*x*/, int /*y*/, int /*w*/, int /*h*/)
{
}

void KisCanvasWidgetPainter::drawRect(const QRect& /*r*/)
{
}

void KisCanvasWidgetPainter::drawWinFocusRect(int /*x*/, int /*y*/, int /*w*/, int /*h*/)
{
}

void KisCanvasWidgetPainter::drawWinFocusRect(int /*x*/, int /*y*/, int /*w*/, int /*h*/, const QColor& /*bgColor*/)
{
}

void KisCanvasWidgetPainter::drawWinFocusRect(const QRect& /*r*/)
{
}

void KisCanvasWidgetPainter::drawWinFocusRect(const QRect& /*r*/, const QColor& /*bgColor*/)
{
}

void KisCanvasWidgetPainter::drawRoundRect(int /*x*/, int /*y*/, int /*w*/, int /*h*/, int /*xRnd*/, int /*yRnd*/)
{
}

void KisCanvasWidgetPainter::drawRoundRect(const QRect& /*r*/, int /*xRnd*/, int /*yRnd*/)
{
}

void KisCanvasWidgetPainter::drawEllipse(int /*x*/, int /*y*/, int /*w*/, int /*h*/)
{
}

void KisCanvasWidgetPainter::drawEllipse(const QRect& /*r*/)
{
}

void KisCanvasWidgetPainter::drawArc(int /*x*/, int /*y*/, int /*w*/, int /*h*/, int /*a*/, int /*alen*/)
{
}

void KisCanvasWidgetPainter::drawArc(const QRect& /*r*/, int /*a*/, int /*alen*/)
{
}

void KisCanvasWidgetPainter::drawPie(int /*x*/, int /*y*/, int /*w*/, int /*h*/, int /*a*/, int /*alen*/)
{
}

void KisCanvasWidgetPainter::drawPie(const QRect& /*r*/, int /*a*/, int /*alen*/)
{
}

void KisCanvasWidgetPainter::drawChord(int /*x*/, int /*y*/, int /*w*/, int /*h*/, int /*a*/, int /*alen*/)
{
}

void KisCanvasWidgetPainter::drawChord(const QRect& /*r*/, int /*a*/, int /*alen*/)
{
}

void KisCanvasWidgetPainter::drawLineSegments(const Q3PointArray& /*pointArray*/, int /*index*/, int /*nlines*/)
{
}

void KisCanvasWidgetPainter::drawPolyline(const Q3PointArray& /*pointArray*/, int /*index*/, int /*npoints*/)
{
}

void KisCanvasWidgetPainter::drawPolygon(const Q3PointArray& /*pointArray*/, bool /*winding*/, int /*index*/, int /*npoints*/)
{
}

void KisCanvasWidgetPainter::drawConvexPolygon(const Q3PointArray& /*pointArray*/, int /*index*/, int /*npoints*/)
{
}

void KisCanvasWidgetPainter::drawCubicBezier(const Q3PointArray& /*pointArray*/, int /*index*/)
{
}

void KisCanvasWidgetPainter::drawPixmap(int /*x*/, int /*y*/, const QPixmap& /*pixmap*/, int /*sx*/, int /*sy*/, int /*sw*/, int /*sh*/)
{
}

void KisCanvasWidgetPainter::drawPixmap(const QPoint& /*point*/, const QPixmap& /*pixmap*/, const QRect& /*sr*/)
{
}

void KisCanvasWidgetPainter::drawPixmap(const QPoint& /*point*/, const QPixmap& /*pixmap*/)
{
}

void KisCanvasWidgetPainter::drawPixmap(const QRect& /*r*/, const QPixmap& /*pixmap*/)
{
}

void KisCanvasWidgetPainter::drawImage(int /*x*/, int /*y*/, const QImage& /*image*/, int /*sx*/, int /*sy*/, int /*sw*/, int /*sh*/, int /*conversionFlags*/)
{
}

void KisCanvasWidgetPainter::drawImage(const QPoint& /*point*/, const QImage& /*image*/, const QRect& /*sr*/, int /*conversionFlags*/)
{
}

void KisCanvasWidgetPainter::drawImage(const QPoint& /*point*/, const QImage& /*image*/, int /*conversion_flags*/)
{
}

void KisCanvasWidgetPainter::drawImage(const QRect& /*r*/, const QImage& /*image*/)
{
}

void KisCanvasWidgetPainter::drawTiledPixmap(int /*x*/, int /*y*/, int /*w*/, int /*h*/, const QPixmap& /*pixmap*/, int /*sx*/, int /*sy*/)
{
}

void KisCanvasWidgetPainter::drawTiledPixmap(const QRect& /*r*/, const QPixmap& /*pixmap*/, const QPoint& /*point*/)
{
}

void KisCanvasWidgetPainter::drawTiledPixmap(const QRect& /*r*/, const QPixmap& /*pixmap*/)
{
}

void KisCanvasWidgetPainter::fillRect(int /*x*/, int /*y*/, int /*w*/, int /*h*/, const QBrush& /*brush*/)
{
}

void KisCanvasWidgetPainter::fillRect(const QRect& /*r*/, const QBrush& /*brush*/)
{
}

void KisCanvasWidgetPainter::eraseRect(int /*x*/, int /*y*/, int /*w*/, int /*h*/)
{
}

void KisCanvasWidgetPainter::eraseRect(const QRect& /*r*/)
{
}

void KisCanvasWidgetPainter::drawText(int /*x*/, int /*y*/, const QString& /*text*/, int /*len*/, QPainter::TextDirection /*dir*/)
{
}

void KisCanvasWidgetPainter::drawText(const QPoint& /*point*/, const QString& /*text*/, int /*len*/, QPainter::TextDirection /*dir*/)
{
}

void KisCanvasWidgetPainter::drawText(int /*x*/, int /*y*/, const QString& /*text*/, int /*pos*/, int /*len*/, QPainter::TextDirection /*dir*/)
{
}

void KisCanvasWidgetPainter::drawText(const QPoint& /*point*/, const QString& /*text*/, int /*pos*/, int /*len*/, QPainter::TextDirection /*dir*/)
{
}

void KisCanvasWidgetPainter::drawText(int /*x*/, int /*y*/, int /*w*/, int /*h*/, int /*flags*/, const QString& /*text*/, int /*len*/, QRect */*br*/, QTextParag **/*intern*/)
{
}

void KisCanvasWidgetPainter::drawText(const QRect& /*r*/, int /*flags*/, const QString& /*text*/, int /*len*/, QRect */*br*/, QTextParag **/*intern*/)
{
}

void KisCanvasWidgetPainter::drawTextItem(int /*x*/, int /*y*/, const QTextItem& /*ti*/, int /*textflags*/)
{
}

void KisCanvasWidgetPainter::drawTextItem(const QPoint& /*p*/, const QTextItem& /*ti*/, int /*textflags*/)
{
}

QRect KisCanvasWidgetPainter::boundingRect(int /*x*/, int /*y*/, int /*w*/, int /*h*/, int /*flags*/, const QString& /*text*/, int /*len*/, QTextParag **/*intern*/)
{
    return QRect();
}

QRect KisCanvasWidgetPainter::boundingRect(const QRect& /*r*/, int /*flags*/, const QString& /*text*/, int /*len*/, QTextParag **/*intern*/)
{
    return QRect();
}

int	KisCanvasWidgetPainter::tabStops() const
{
    return 0;
}

void KisCanvasWidgetPainter::setTabStops(int /*ts*/)
{
}

int	*KisCanvasWidgetPainter::tabArray() const
{
    return 0;
}

void KisCanvasWidgetPainter::setTabArray(int */*ts*/)
{
}

/*************************************************************************/

KisCanvasPainter::KisCanvasPainter()
{
    m_canvasWidgetPainter = 0;
}

KisCanvasPainter::KisCanvasPainter(KisCanvas *canvas)
{
    m_canvasWidgetPainter = canvas->createPainter();
}

KisCanvasPainter::KisCanvasPainter(const QPaintDevice *paintDevice)
{
    m_canvasWidgetPainter = new KisQPaintDeviceCanvasPainter(paintDevice);
}

KisCanvasPainter::~KisCanvasPainter()
{
    delete m_canvasWidgetPainter;
}

bool KisCanvasPainter::begin(KisCanvas *canvas, bool unclipped)
{
    delete m_canvasWidgetPainter;
    m_canvasWidgetPainter = canvas->createPainter();
    return m_canvasWidgetPainter->begin(canvas->canvasWidget(), unclipped);
}

bool KisCanvasPainter::begin(const QPaintDevice *paintDevice, bool unclipped)
{
    delete m_canvasWidgetPainter;
    m_canvasWidgetPainter = new KisQPaintDeviceCanvasPainter();
    return static_cast<KisQPaintDeviceCanvasPainter *>(m_canvasWidgetPainter)->begin(paintDevice, unclipped);
}

bool KisCanvasPainter::end()
{
    if (m_canvasWidgetPainter != 0) {
        return m_canvasWidgetPainter->end();
    }
    return false;
}

void KisCanvasPainter::save()
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->save();
    }
}

void KisCanvasPainter::restore()
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->restore();
    }
}

QFontMetrics KisCanvasPainter::fontMetrics() const
{
    if (m_canvasWidgetPainter != 0) {
        return m_canvasWidgetPainter->fontMetrics();
    }
    return QFontMetrics(QFont());
}

QFontInfo KisCanvasPainter::fontInfo() const
{
    if (m_canvasWidgetPainter != 0) {
        return m_canvasWidgetPainter->fontInfo();
    }
    return QFontInfo(QFont());
}

const QFont& KisCanvasPainter::font() const
{
    if (m_canvasWidgetPainter != 0) {
        return m_canvasWidgetPainter->font();
    }
    return m_defaultFont;
}

void KisCanvasPainter::setFont(const QFont& font)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->setFont(font);
    }
}

const QPen& KisCanvasPainter::pen() const
{
    if (m_canvasWidgetPainter != 0) {
        return m_canvasWidgetPainter->pen();
    }
    return m_defaultPen;
}

void KisCanvasPainter::setPen(const QPen& pen)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->setPen(pen);
    }
}

void KisCanvasPainter::setPen(Qt::PenStyle penStyle)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->setPen(penStyle);
    }
}

void KisCanvasPainter::setPen(const QColor& color)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->setPen(color);;
    }
}

const QBrush& KisCanvasPainter::brush() const
{
    if (m_canvasWidgetPainter != 0) {
        return m_canvasWidgetPainter->brush();
    }
    return m_defaultBrush;
}

void KisCanvasPainter::setBrush(const QBrush& brush)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->setBrush(brush);
    }
}

void KisCanvasPainter::setBrush(Qt::BrushStyle brushStyle)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->setBrush(brushStyle);
    }
}

void KisCanvasPainter::setBrush(const QColor& color)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->setBrush(color);
    }
}

QPoint KisCanvasPainter::pos() const
{
    if (m_canvasWidgetPainter != 0) {
        return m_canvasWidgetPainter->pos();
    }
    return QPoint();
}

const QColor& KisCanvasPainter::backgroundColor() const
{
    if (m_canvasWidgetPainter != 0) {
        return m_canvasWidgetPainter->backgroundColor();
    }
    return m_defaultColor;
}

void KisCanvasPainter::setBackgroundColor(const QColor& color)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->setBackgroundColor(color);
    }
}

Qt::BGMode KisCanvasPainter::backgroundMode() const
{
    if (m_canvasWidgetPainter != 0) {
        return m_canvasWidgetPainter->backgroundMode();
    }
    return Qt::TransparentMode;
}

void KisCanvasPainter::setBackgroundMode(Qt::BGMode bgMode)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->setBackgroundMode(bgMode);
    }
}

Qt::RasterOp KisCanvasPainter::rasterOp() const
{
    if (m_canvasWidgetPainter != 0) {
        return m_canvasWidgetPainter->rasterOp();
    }
    return Qt::CopyROP;
}

void KisCanvasPainter::setRasterOp(Qt::RasterOp rasterOp)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->setRasterOp(rasterOp);
    }
}

const QPoint& KisCanvasPainter::brushOrigin() const
{
    if (m_canvasWidgetPainter != 0) {
        return m_canvasWidgetPainter->brushOrigin();
    }
    return m_defaultBrushOrigin;
}

void KisCanvasPainter::setBrushOrigin(int x, int y)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->setBrushOrigin(x, y);
    }
}

void KisCanvasPainter::setBrushOrigin(const QPoint& origin)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->setBrushOrigin(origin);
    }
}

bool KisCanvasPainter::hasViewXForm() const
{
    if (m_canvasWidgetPainter != 0) {
        return m_canvasWidgetPainter->hasViewXForm();
    }
    return false;
}

bool KisCanvasPainter::hasWorldXForm() const
{
    if (m_canvasWidgetPainter != 0) {
        return m_canvasWidgetPainter->hasWorldXForm();
    }
    return false;
}

void KisCanvasPainter::setViewXForm(bool enable)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->setViewXForm(enable);
    }
}

QRect KisCanvasPainter::window() const
{
    if (m_canvasWidgetPainter != 0) {
        return m_canvasWidgetPainter->window();
    }
    return QRect();
}

void KisCanvasPainter::setWindow(const QRect& r)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->setWindow(r);
    }
}

void KisCanvasPainter::setWindow(int x, int y, int w, int h)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->setWindow(x, y, w, h);
    }
}

QRect KisCanvasPainter::viewport() const
{
    if (m_canvasWidgetPainter != 0) {
        return m_canvasWidgetPainter->viewport();
    }
    return QRect();
}

void KisCanvasPainter::setViewport(const QRect& r)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->setViewport(r);
    }
}

void KisCanvasPainter::setViewport(int x, int y, int w, int h)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->setViewport(x, y, w, h);
    }
}

void KisCanvasPainter::setWorldXForm(bool enable)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->setWorldXForm(enable);
    }
}

const QMatrix& KisCanvasPainter::worldMatrix() const
{
    if (m_canvasWidgetPainter != 0) {
        return m_canvasWidgetPainter->worldMatrix();
    }
    return m_defaultWorldMatrix;
}

void KisCanvasPainter::setMatrix(const QMatrix& matrix, bool combine)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->setMatrix(matrix, combine);
    }
}

void KisCanvasPainter::saveWorldMatrix()
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->saveWorldMatrix();
    }
}

void KisCanvasPainter::restoreWorldMatrix()
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->restoreWorldMatrix();
    }
}

void KisCanvasPainter::scale(double sx, double sy)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->scale(sx, sy);
    }
}

void KisCanvasPainter::shear(double sh, double sv)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->shear(sh, sv);
    }
}

void KisCanvasPainter::rotate(double a)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->rotate(a);
    }
}

void KisCanvasPainter::translate(double dx, double dy)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->translate(dx, dy);
    }
}

void KisCanvasPainter::resetXForm()
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->resetXForm();
    }
}

double KisCanvasPainter::translationX() const
{
    if (m_canvasWidgetPainter != 0) {
        return m_canvasWidgetPainter->translationX();
    }
    return 0;
}

double KisCanvasPainter::translationY() const
{
    if (m_canvasWidgetPainter != 0) {
        return m_canvasWidgetPainter->translationY();
    }
    return 0;
}

QPoint KisCanvasPainter::xForm(const QPoint& point) const
{
    if (m_canvasWidgetPainter != 0) {
        return m_canvasWidgetPainter->transformed(point);
    }
    return point;
}

QRect KisCanvasPainter::xForm(const QRect& r) const
{
    if (m_canvasWidgetPainter != 0) {
        return m_canvasWidgetPainter->transformed(r);
    }
    return r;
}

Q3PointArray KisCanvasPainter::xForm(const Q3PointArray& pointArray) const
{
    if (m_canvasWidgetPainter != 0) {
        return m_canvasWidgetPainter->transformed(pointArray);
    }
    return pointArray;
}

Q3PointArray KisCanvasPainter::xForm(const Q3PointArray& pointArray, int index, int npoints) const
{
    if (m_canvasWidgetPainter != 0) {
        return m_canvasWidgetPainter->transformed(pointArray, index, npoints);
    }
    return pointArray;
}

QPoint KisCanvasPainter::xFormDev(const QPoint& point) const
{
    if (m_canvasWidgetPainter != 0) {
        return m_canvasWidgetPainter->xFormDev(point);
    }
    return point;
}

QRect KisCanvasPainter::xFormDev(const QRect& r) const
{
    if (m_canvasWidgetPainter != 0) {
        return m_canvasWidgetPainter->xFormDev(r);
    }
    return r;
}

Q3PointArray KisCanvasPainter::xFormDev(const Q3PointArray& pointArray) const
{
    if (m_canvasWidgetPainter != 0) {
        return m_canvasWidgetPainter->xFormDev(pointArray);
    }
    return pointArray;
}

Q3PointArray KisCanvasPainter::xFormDev(const Q3PointArray& pointArray, int index, int npoints) const
{
    if (m_canvasWidgetPainter != 0) {
        return m_canvasWidgetPainter->xFormDev(pointArray, index, npoints);
    }
    return pointArray;
}

void KisCanvasPainter::setClipping(bool enable)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->setClipping(enable);
    }
}

bool KisCanvasPainter::hasClipping() const
{
    if (m_canvasWidgetPainter != 0) {
        return m_canvasWidgetPainter->hasClipping();
    }
    return true;
}

QRegion KisCanvasPainter::clipRegion(QPainter::CoordinateMode mode) const
{
    if (m_canvasWidgetPainter != 0) {
        return m_canvasWidgetPainter->clipRegion(mode);
    }
    return QRegion();
}

void KisCanvasPainter::setClipRect(const QRect& r, QPainter::CoordinateMode mode)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->setClipRect(r, mode);
    }
}

void KisCanvasPainter::setClipRect(int x, int y, int w, int h, QPainter::CoordinateMode mode)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->setClipRect(x, y, w, h, mode);
    }
}

void KisCanvasPainter::setClipRegion(const QRegion& rgn, QPainter::CoordinateMode mode)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->setClipRegion(rgn, mode);
    }
}

void KisCanvasPainter::drawPoint(int x, int y)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawPoint(x, y);
    }
}

void KisCanvasPainter::drawPoint(const QPoint& point)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawPoint(point);
    }
}

void KisCanvasPainter::drawPoints(const Q3PointArray& pointArray, int index, int npoints)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawPoints(pointArray, index, npoints);
    }
}

void KisCanvasPainter::moveTo(int x, int y)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->moveTo(x, y);
    }
}

void KisCanvasPainter::moveTo(const QPoint& point)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->moveTo(point);
    }
}

void KisCanvasPainter::lineTo(int x, int y)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->lineTo(x, y);
    }
}

void KisCanvasPainter::lineTo(const QPoint& point)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->lineTo(point);
    }
}

void KisCanvasPainter::drawLine(int x1, int y1, int x2, int y2)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawLine(x1, y1, x2, y2);
    }
}

void KisCanvasPainter::drawLine(const QPoint& start, const QPoint& end)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawLine(start, end);
    }
}

void KisCanvasPainter::drawRect(int x, int y, int w, int h)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawRect(x, y, w, h);
    }
}

void KisCanvasPainter::drawRect(const QRect& r)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawRect(r);
    }
}

void KisCanvasPainter::drawWinFocusRect(int x, int y, int w, int h)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawWinFocusRect(x, y, w, h);
    }
}

void KisCanvasPainter::drawWinFocusRect(int x, int y, int w, int h, const QColor& bgColor)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawWinFocusRect(x, y, w, h, bgColor);
    }
}

void KisCanvasPainter::drawWinFocusRect(const QRect& r)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawWinFocusRect(r);
    }
}

void KisCanvasPainter::drawWinFocusRect(const QRect& r, const QColor& bgColor)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawWinFocusRect(r, bgColor);
    }
}

void KisCanvasPainter::drawRoundRect(int x, int y, int w, int h, int xRnd, int yRnd)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawRoundRect(x, y, w, h, xRnd, yRnd);
    }
}

void KisCanvasPainter::drawRoundRect(const QRect& r, int xRnd, int yRnd)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawRoundRect(r, xRnd, yRnd);
    }
}

void KisCanvasPainter::drawEllipse(int x, int y, int w, int h)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawEllipse(x, y, w, h);
    }
}

void KisCanvasPainter::drawEllipse(const QRect& r)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawEllipse(r);
    }
}

void KisCanvasPainter::drawArc(int x, int y, int w, int h, int a, int alen)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawArc(x, y, w, h, a, alen);
    }
}

void KisCanvasPainter::drawArc(const QRect& r, int a, int alen)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawArc(r, a, alen);
    }
}

void KisCanvasPainter::drawPie(int x, int y, int w, int h, int a, int alen)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawPie(x, y, w, h, a, alen);
    }
}

void KisCanvasPainter::drawPie(const QRect& r, int a, int alen)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawPie(r, a, alen);
    }
}

void KisCanvasPainter::drawChord(int x, int y, int w, int h, int a, int alen)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawChord(x, y, w, h, a, alen);
    }
}

void KisCanvasPainter::drawChord(const QRect& r, int a, int alen)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawChord(r, a, alen);
    }
}

void KisCanvasPainter::drawLineSegments(const Q3PointArray& pointArray, int index, int nlines)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawLineSegments(pointArray, index, nlines);
    }
}

void KisCanvasPainter::drawPolyline(const Q3PointArray& pointArray, int index, int npoints)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawPolyline(pointArray, index, npoints);
    }
}

void KisCanvasPainter::drawPolygon(const Q3PointArray& pointArray, bool winding, int index, int npoints)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawPolygon(pointArray, winding, index, npoints);
    }
}

void KisCanvasPainter::drawConvexPolygon(const Q3PointArray& pointArray, int index, int npoints)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawConvexPolygon(pointArray, index, npoints);
    }
}

void KisCanvasPainter::drawCubicBezier(const Q3PointArray& pointArray, int index)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawCubicBezier(pointArray, index);
    }
}

void KisCanvasPainter::drawPixmap(int x, int y, const QPixmap& pixmap, int sx, int sy, int sw, int sh)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawPixmap(x, y, pixmap, sx, sy, sw, sh);
    }
}

void KisCanvasPainter::drawPixmap(const QPoint& point, const QPixmap& pixmap, const QRect& sr)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawPixmap(point, pixmap, sr);
    }
}

void KisCanvasPainter::drawPixmap(const QPoint& point, const QPixmap& pixmap)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawPixmap(point, pixmap);
    }
}

void KisCanvasPainter::drawPixmap(const QRect& r, const QPixmap& pixmap)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawPixmap(r, pixmap);
    }
}

void KisCanvasPainter::drawImage(int x, int y, const QImage& image, int sx, int sy, int sw, int sh, int conversionFlags)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawImage(x, y, image, sx, sy, sw, sh, conversionFlags);
    }
}

void KisCanvasPainter::drawImage(const QPoint& point, const QImage& image, const QRect& sr, int conversionFlags)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawImage(point, image, sr, conversionFlags);
    }
}

void KisCanvasPainter::drawImage(const QPoint& point, const QImage& image, int conversion_flags)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawImage(point, image, conversion_flags);
    }
}

void KisCanvasPainter::drawImage(const QRect& r, const QImage& image)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawImage(r, image);
    }
}

void KisCanvasPainter::drawTiledPixmap(int x, int y, int w, int h, const QPixmap& pixmap, int sx, int sy)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawTiledPixmap(x, y, w, h, pixmap, sx, sy);
    }
}

void KisCanvasPainter::drawTiledPixmap(const QRect& r, const QPixmap& pixmap, const QPoint& point)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawTiledPixmap(r, pixmap, point);
    }
}

void KisCanvasPainter::drawTiledPixmap(const QRect& r, const QPixmap& pixmap)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawTiledPixmap(r, pixmap);
    }
}

void KisCanvasPainter::fillRect(int x, int y, int w, int h, const QBrush& brush)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->fillRect(x, y, w, h, brush);
    }
}

void KisCanvasPainter::fillRect(const QRect& r, const QBrush& brush)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->fillRect(r, brush);
    }
}

void KisCanvasPainter::eraseRect(int x, int y, int w, int h)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->eraseRect(x, y, w, h);
    }
}

void KisCanvasPainter::eraseRect(const QRect& r)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->eraseRect(r);
    }
}

void KisCanvasPainter::drawText(int x, int y, const QString& text, int len, QPainter::TextDirection dir)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawText(x, y, text, len, dir);
    }
}

void KisCanvasPainter::drawText(const QPoint& point, const QString& text, int len, QPainter::TextDirection dir)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawText(point, text, len, dir);
    }
}

void KisCanvasPainter::drawText(int x, int y, const QString& text, int pos, int len, QPainter::TextDirection dir)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawText(x, y, text, pos, len, dir);
    }
}

void KisCanvasPainter::drawText(const QPoint& point, const QString& text, int pos, int len, QPainter::TextDirection dir)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawText(point, text, pos, len, dir);
    }
}

void KisCanvasPainter::drawText(int x, int y, int w, int h, int flags, const QString& text, int len, QRect *br, QTextParag **intern)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawText(x, y, w, h, flags, text, len, br, intern);
    }
}

void KisCanvasPainter::drawText(const QRect& r, int flags, const QString& text, int len, QRect *br, QTextParag **intern)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawText(r, flags, text, len, br, intern);
    }
}

void KisCanvasPainter::drawTextItem(int x, int y, const QTextItem& ti, int textflags)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawTextItem(x, y, ti, textflags);
    }
}

void KisCanvasPainter::drawTextItem(const QPoint& p, const QTextItem& ti, int textflags)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->drawTextItem(p, ti, textflags);
    }
}

QRect KisCanvasPainter::boundingRect(int x, int y, int w, int h, int flags, const QString& text, int len, QTextParag **intern)
{
    if (m_canvasWidgetPainter != 0) {
        return m_canvasWidgetPainter->boundingRect(x, y, w, h, flags, text, len, intern);
    }
    return QRect();
}

QRect KisCanvasPainter::boundingRect(const QRect& r, int flags, const QString& text, int len, QTextParag **intern)
{
    if (m_canvasWidgetPainter != 0) {
        return m_canvasWidgetPainter->boundingRect(r, flags, text, len, intern);
    }
    return QRect();
}

int	KisCanvasPainter::tabStops() const
{
    if (m_canvasWidgetPainter != 0) {
        return m_canvasWidgetPainter->tabStops();
    }
    return 0;
}

void KisCanvasPainter::setTabStops(int ts)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->setTabStops(ts);
    }
}

int	*KisCanvasPainter::tabArray() const
{
    if (m_canvasWidgetPainter != 0) {
        return m_canvasWidgetPainter->tabArray();
    }
    return 0;
}

void KisCanvasPainter::setTabArray(int *ts)
{
    if (m_canvasWidgetPainter != 0) {
        m_canvasWidgetPainter->setTabArray(ts);
    }
}

