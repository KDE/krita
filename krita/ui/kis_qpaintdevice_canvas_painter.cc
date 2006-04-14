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
#include <QPolygon>

KisQPaintDeviceCanvasPainter::KisQPaintDeviceCanvasPainter()
{
}

KisQPaintDeviceCanvasPainter::KisQPaintDeviceCanvasPainter(QPaintDevice *paintDevice)
    : m_painter(paintDevice)
{
}

KisQPaintDeviceCanvasPainter::~KisQPaintDeviceCanvasPainter()
{
}

bool KisQPaintDeviceCanvasPainter::begin(KisCanvasWidget *canvasWidget, bool /*unclipped*/)
{
    QWidget *widget = dynamic_cast<QWidget *>(canvasWidget);

    if (widget != 0) {
        return m_painter.begin(widget);
    } else {
        return false;
    }
}

bool KisQPaintDeviceCanvasPainter::begin(QPaintDevice* paintDevice, bool /*unclipped*/)
{
    return m_painter.begin(paintDevice);
}

bool KisQPaintDeviceCanvasPainter::end()
{
    return m_painter.end();
}

void KisQPaintDeviceCanvasPainter::save()
{
    m_painter.save();
}

void KisQPaintDeviceCanvasPainter::restore()
{
    m_painter.restore();
}

QFontMetrics KisQPaintDeviceCanvasPainter::fontMetrics() const
{
    return m_painter.fontMetrics();
}

QFontInfo KisQPaintDeviceCanvasPainter::fontInfo() const
{
    return m_painter.fontInfo();
}

const QFont& KisQPaintDeviceCanvasPainter::font() const
{
    return m_painter.font();
}

void KisQPaintDeviceCanvasPainter::setFont(const QFont& font)
{
    m_painter.setFont(font);
}

const QPen& KisQPaintDeviceCanvasPainter::pen() const
{
    return m_painter.pen();
}

void KisQPaintDeviceCanvasPainter::setPen(const QPen& pen)
{
    m_painter.setPen(pen);
}

void KisQPaintDeviceCanvasPainter::setPen(Qt::PenStyle penStyle)
{
    m_painter.setPen(penStyle);
}

void KisQPaintDeviceCanvasPainter::setPen(const QColor& color)
{
    m_painter.setPen(color);;
}

const QBrush& KisQPaintDeviceCanvasPainter::brush() const
{
    return m_painter.brush();
}

void KisQPaintDeviceCanvasPainter::setBrush(const QBrush& brush)
{
    m_painter.setBrush(brush);
}

void KisQPaintDeviceCanvasPainter::setBrush(Qt::BrushStyle brushStyle)
{
    m_painter.setBrush(brushStyle);
}

void KisQPaintDeviceCanvasPainter::setBrush(const QColor& color)
{
    m_painter.setBrush(color);
}

QPoint KisQPaintDeviceCanvasPainter::pos() const
{
    return QPoint();
}

const QColor& KisQPaintDeviceCanvasPainter::backgroundColor() const
{
    return m_painter.backgroundColor();
}

void KisQPaintDeviceCanvasPainter::setBackgroundColor(const QColor& color)
{
    m_painter.setBackgroundColor(color);
}

Qt::BGMode KisQPaintDeviceCanvasPainter::backgroundMode() const
{
    return m_painter.backgroundMode();
}

void KisQPaintDeviceCanvasPainter::setBackgroundMode(Qt::BGMode bgMode)
{
    m_painter.setBackgroundMode(bgMode);
}

// Qt::RasterOp KisQPaintDeviceCanvasPainter::rasterOp() const
// {
//     return m_painter.rasterOp();
// }

// void KisQPaintDeviceCanvasPainter::setRasterOp(Qt::RasterOp rasterOp)
// {
//     m_painter.setRasterOp(rasterOp);
// }

const QPoint& KisQPaintDeviceCanvasPainter::brushOrigin() const
{
    return m_painter.brushOrigin();
}

void KisQPaintDeviceCanvasPainter::setBrushOrigin(int x, int y)
{
    m_painter.setBrushOrigin(x, y);
}

void KisQPaintDeviceCanvasPainter::setBrushOrigin(const QPoint& origin)
{
    m_painter.setBrushOrigin(origin);
}

bool KisQPaintDeviceCanvasPainter::hasViewXForm() const
{
    return m_painter.hasViewXForm();
}

bool KisQPaintDeviceCanvasPainter::hasWorldXForm() const
{
    return m_painter.hasWorldXForm();
}

void KisQPaintDeviceCanvasPainter::setViewXForm(bool enable)
{
    m_painter.setViewXForm(enable);
}

QRect KisQPaintDeviceCanvasPainter::window() const
{
    return m_painter.window();
}

void KisQPaintDeviceCanvasPainter::setWindow(const QRect& r)
{
    m_painter.setWindow(r);
}

void KisQPaintDeviceCanvasPainter::setWindow(int x, int y, int w, int h)
{
    m_painter.setWindow(x, y, w, h);
}

QRect KisQPaintDeviceCanvasPainter::viewport() const
{
    return m_painter.viewport();
}

void KisQPaintDeviceCanvasPainter::setViewport(const QRect& r)
{
    m_painter.setViewport(r);
}

void KisQPaintDeviceCanvasPainter::setViewport(int x, int y, int w, int h)
{
    m_painter.setViewport(x, y, w, h);
}

void KisQPaintDeviceCanvasPainter::setWorldXForm(bool enable)
{
    m_painter.setWorldXForm(enable);
}

const QMatrix& KisQPaintDeviceCanvasPainter::worldMatrix() const
{
    return m_painter.worldMatrix();
}

void KisQPaintDeviceCanvasPainter::setMatrix(const QMatrix& matrix, bool combine)
{
    m_painter.setMatrix(matrix, combine);
}

void KisQPaintDeviceCanvasPainter::saveWorldMatrix()
{
    //m_painter.saveWorldMatrix();
}

void KisQPaintDeviceCanvasPainter::restoreWorldMatrix()
{
    //m_painter.restoreWorldMatrix();
}

void KisQPaintDeviceCanvasPainter::scale(double sx, double sy)
{
    m_painter.scale(sx, sy);
}

void KisQPaintDeviceCanvasPainter::shear(double sh, double sv)
{
    m_painter.shear(sh, sv);
}

void KisQPaintDeviceCanvasPainter::rotate(double a)
{
    m_painter.rotate(a);
}

void KisQPaintDeviceCanvasPainter::translate(double dx, double dy)
{
    m_painter.translate(dx, dy);
}

void KisQPaintDeviceCanvasPainter::resetXForm()
{
    m_painter.resetXForm();
}

double KisQPaintDeviceCanvasPainter::translationX() const
{
    return m_painter.translationX();
}

double KisQPaintDeviceCanvasPainter::translationY() const
{
    return m_painter.translationY();
}

QPoint KisQPaintDeviceCanvasPainter::xForm(const QPoint& point) const
{
    return m_painter.xForm(point);
}

QRect KisQPaintDeviceCanvasPainter::xForm(const QRect& r) const
{
    return m_painter.xForm(r);
}

QPolygon KisQPaintDeviceCanvasPainter::xForm(const QPolygon& pointArray) const
{
    return m_painter.xForm(pointArray);
}

QPolygon KisQPaintDeviceCanvasPainter::xForm(const QPolygon& pointArray, int index, int npoints) const
{
    return m_painter.xForm(pointArray, index, npoints);
}

QPoint KisQPaintDeviceCanvasPainter::xFormDev(const QPoint& point) const
{
    return m_painter.xFormDev(point);
}

QRect KisQPaintDeviceCanvasPainter::xFormDev(const QRect& r) const
{
    return m_painter.xFormDev(r);
}

QPolygon KisQPaintDeviceCanvasPainter::xFormDev(const QPolygon& pointArray) const
{
    return m_painter.xFormDev(pointArray);
}

QPolygon KisQPaintDeviceCanvasPainter::xFormDev(const QPolygon& pointArray, int index, int npoints) const
{
    return m_painter.xFormDev(pointArray, index, npoints);
}

void KisQPaintDeviceCanvasPainter::setClipping(bool enable)
{
    m_painter.setClipping(enable);
}

bool KisQPaintDeviceCanvasPainter::hasClipping() const
{
    return m_painter.hasClipping();
}

QRegion KisQPaintDeviceCanvasPainter::clipRegion() const
{
    return m_painter.clipRegion();
}

void KisQPaintDeviceCanvasPainter::setClipRect(const QRect& r)
{
    m_painter.setClipRect(r);
}

void KisQPaintDeviceCanvasPainter::setClipRect(int x, int y, int w, int h)
{
    m_painter.setClipRect(x, y, w, h);
}

void KisQPaintDeviceCanvasPainter::setClipRegion(const QRegion& rgn)
{
    m_painter.setClipRegion(rgn);
}

void KisQPaintDeviceCanvasPainter::drawPoint(int x, int y)
{
    m_painter.drawPoint(x, y);
}

void KisQPaintDeviceCanvasPainter::drawPoint(const QPoint& point)
{
    m_painter.drawPoint(point);
}

void KisQPaintDeviceCanvasPainter::drawPoints(const QPolygon& pointArray, int index, int npoints)
{
    m_painter.drawPoints(pointArray, index, npoints);
}

void KisQPaintDeviceCanvasPainter::moveTo(int x, int y)
{
    //m_painter.moveTo(x, y);
}

void KisQPaintDeviceCanvasPainter::moveTo(const QPoint& point)
{
    //m_painter.moveTo(point);
}

void KisQPaintDeviceCanvasPainter::lineTo(int x, int y)
{
    //m_painter.lineTo(x, y);
}

void KisQPaintDeviceCanvasPainter::lineTo(const QPoint& point)
{
    //m_painter.lineTo(point);
}

void KisQPaintDeviceCanvasPainter::drawLine(int x1, int y1, int x2, int y2)
{
    m_painter.drawLine(x1, y1, x2, y2);
}

void KisQPaintDeviceCanvasPainter::drawLine(const QPoint& start, const QPoint& end)
{
    m_painter.drawLine(start, end);
}

void KisQPaintDeviceCanvasPainter::drawRect(int x, int y, int w, int h)
{
    m_painter.drawRect(x, y, w, h);
}

void KisQPaintDeviceCanvasPainter::drawRect(const QRect& r)
{
    m_painter.drawRect(r);
}

void KisQPaintDeviceCanvasPainter::drawWinFocusRect(int x, int y, int w, int h)
{
    //m_painter.drawWinFocusRect(x, y, w, h);
}

void KisQPaintDeviceCanvasPainter::drawWinFocusRect(int x, int y, int w, int h, const QColor& bgColor)
{
    //m_painter.drawWinFocusRect(x, y, w, h, bgColor);
}

void KisQPaintDeviceCanvasPainter::drawWinFocusRect(const QRect& r)
{
    //m_painter.drawWinFocusRect(r);
}

void KisQPaintDeviceCanvasPainter::drawWinFocusRect(const QRect& r, const QColor& bgColor)
{
    //m_painter.drawWinFocusRect(r, bgColor);
}

void KisQPaintDeviceCanvasPainter::drawRoundRect(int x, int y, int w, int h, int xRnd, int yRnd)
{
    m_painter.drawRoundRect(x, y, w, h, xRnd, yRnd);
}

void KisQPaintDeviceCanvasPainter::drawRoundRect(const QRect& r, int xRnd, int yRnd)
{
    m_painter.drawRoundRect(r, xRnd, yRnd);
}

void KisQPaintDeviceCanvasPainter::drawEllipse(int x, int y, int w, int h)
{
    m_painter.drawEllipse(x, y, w, h);
}

void KisQPaintDeviceCanvasPainter::drawEllipse(const QRect& r)
{
    m_painter.drawEllipse(r);
}

void KisQPaintDeviceCanvasPainter::drawArc(int x, int y, int w, int h, int a, int alen)
{
    m_painter.drawArc(x, y, w, h, a, alen);
}

void KisQPaintDeviceCanvasPainter::drawArc(const QRect& r, int a, int alen)
{
    m_painter.drawArc(r, a, alen);
}

void KisQPaintDeviceCanvasPainter::drawPie(int x, int y, int w, int h, int a, int alen)
{
    m_painter.drawPie(x, y, w, h, a, alen);
}

void KisQPaintDeviceCanvasPainter::drawPie(const QRect& r, int a, int alen)
{
    m_painter.drawPie(r, a, alen);
}

void KisQPaintDeviceCanvasPainter::drawChord(int x, int y, int w, int h, int a, int alen)
{
    m_painter.drawChord(x, y, w, h, a, alen);
}

void KisQPaintDeviceCanvasPainter::drawChord(const QRect& r, int a, int alen)
{
    m_painter.drawChord(r, a, alen);
}

void KisQPaintDeviceCanvasPainter::drawLineSegments(const QPolygon& pointArray, int index, int nlines)
{
    m_painter.drawLineSegments(pointArray, index, nlines);
}

void KisQPaintDeviceCanvasPainter::drawPolyline(const QPolygon& pointArray, int index, int npoints)
{
    m_painter.drawPolyline(pointArray, index, npoints);
}

void KisQPaintDeviceCanvasPainter::drawPolygon(const QPolygon& pointArray, bool winding, int index, int npoints)
{
    m_painter.drawPolygon(pointArray, winding, index, npoints);
}

void KisQPaintDeviceCanvasPainter::drawConvexPolygon(const QPolygon& pointArray, int index, int npoints)
{
    m_painter.drawConvexPolygon(pointArray, index, npoints);
}

void KisQPaintDeviceCanvasPainter::drawCubicBezier(const QPolygon& pointArray, int index)
{
    m_painter.drawCubicBezier(pointArray, index);
}

void KisQPaintDeviceCanvasPainter::drawPixmap(int x, int y, const QPixmap& pixmap, int sx, int sy, int sw, int sh)
{
    m_painter.drawPixmap(x, y, pixmap, sx, sy, sw, sh);
}

void KisQPaintDeviceCanvasPainter::drawPixmap(const QPoint& point, const QPixmap& pixmap, const QRect& sr)
{
    m_painter.drawPixmap(point, pixmap, sr);
}

void KisQPaintDeviceCanvasPainter::drawPixmap(const QPoint& point, const QPixmap& pixmap)
{
    m_painter.drawPixmap(point, pixmap);
}

void KisQPaintDeviceCanvasPainter::drawPixmap(const QRect& r, const QPixmap& pixmap)
{
    m_painter.drawPixmap(r, pixmap);
}

void KisQPaintDeviceCanvasPainter::drawImage(int x, int y, const QImage& image, int sx, int sy, int sw, int sh, int conversionFlags)
{
    m_painter.drawImage(x, y, image, sx, sy, sw, sh);
}

void KisQPaintDeviceCanvasPainter::drawImage(const QPoint& point, const QImage& image, const QRect& sr, int conversionFlags)
{
    m_painter.drawImage(point, image, sr);
}

void KisQPaintDeviceCanvasPainter::drawImage(const QPoint& point, const QImage& image, int conversion_flags)
{
    m_painter.drawImage(point, image);
}

void KisQPaintDeviceCanvasPainter::drawImage(const QRect& r, const QImage& image)
{
    m_painter.drawImage(r, image);
}

void KisQPaintDeviceCanvasPainter::drawTiledPixmap(int x, int y, int w, int h, const QPixmap& pixmap, int sx, int sy)
{
    m_painter.drawTiledPixmap(x, y, w, h, pixmap, sx, sy);
}

void KisQPaintDeviceCanvasPainter::drawTiledPixmap(const QRect& r, const QPixmap& pixmap, const QPoint& point)
{
    m_painter.drawTiledPixmap(r, pixmap, point);
}

void KisQPaintDeviceCanvasPainter::drawTiledPixmap(const QRect& r, const QPixmap& pixmap)
{
    m_painter.drawTiledPixmap(r, pixmap);
}

void KisQPaintDeviceCanvasPainter::fillRect(int x, int y, int w, int h, const QBrush& brush)
{
    m_painter.fillRect(x, y, w, h, brush);
}

void KisQPaintDeviceCanvasPainter::fillRect(const QRect& r, const QBrush& brush)
{
    m_painter.fillRect(r, brush);
}

void KisQPaintDeviceCanvasPainter::eraseRect(int x, int y, int w, int h)
{
    m_painter.eraseRect(x, y, w, h);
}

void KisQPaintDeviceCanvasPainter::eraseRect(const QRect& r)
{
    m_painter.eraseRect(r);
}

// void KisQPaintDeviceCanvasPainter::drawText(int x, int y, const QString& text, int len, QPainter::TextDirection dir)
// {
//     m_painter.drawText(x, y, text, len, dir);
// }
//
// void KisQPaintDeviceCanvasPainter::drawText(const QPoint& point, const QString& text, int len, QPainter::TextDirection dir)
// {
//     m_painter.drawText(point, text, len, dir);
// }
//
// void KisQPaintDeviceCanvasPainter::drawText(int x, int y, const QString& text, int pos, int len, QPainter::TextDirection dir)
// {
//     m_painter.drawText(x, y, text, pos, len, dir);
// }
//
// void KisQPaintDeviceCanvasPainter::drawText(const QPoint& point, const QString& text, int pos, int len, QPainter::TextDirection dir)
// {
//     m_painter.drawText(point, text, pos, len, dir);
// }
//
// void KisQPaintDeviceCanvasPainter::drawText(int x, int y, int w, int h, int flags, const QString& text, int len, QRect *br, QTextParag **intern)
// {
//     m_painter.drawText(x, y, w, h, flags, text, len, br, intern);
// }
//
// void KisQPaintDeviceCanvasPainter::drawText(const QRect& r, int flags, const QString& text, int len, QRect *br, QTextParag **intern)
// {
//     m_painter.drawText(r, flags, text, len, br, intern);
// }
//
// void KisQPaintDeviceCanvasPainter::drawTextItem(int x, int y, const QTextItem& ti, int textflags)
// {
//     m_painter.drawTextItem(x, y, ti, textflags);
// }
//
// void KisQPaintDeviceCanvasPainter::drawTextItem(const QPoint& p, const QTextItem& ti, int textflags)
// {
//     m_painter.drawTextItem(p, ti, textflags);
// }
//
// QRect KisQPaintDeviceCanvasPainter::boundingRect(int x, int y, int w, int h, int flags, const QString& text, int len, QTextParag **intern)
// {
//     return m_painter.boundingRect(x, y, w, h, flags, text, len, intern);
// }
//
// QRect KisQPaintDeviceCanvasPainter::boundingRect(const QRect& r, int flags, const QString& text, int len, QTextParag **intern)
// {
//     return m_painter.boundingRect(r, flags, text, len, intern);
// }

int	KisQPaintDeviceCanvasPainter::tabStops() const
{
    return 0;
}

void KisQPaintDeviceCanvasPainter::setTabStops(int )
{
}

int	*KisQPaintDeviceCanvasPainter::tabArray() const
{
    return 0;
}

void KisQPaintDeviceCanvasPainter::setTabArray(int *)
{
}


