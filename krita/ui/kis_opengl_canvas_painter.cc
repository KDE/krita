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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_GL

#include <kdebug.h>

#include "kis_canvas.h"
#include "kis_canvas_painter.h"
#include "kis_opengl_canvas_painter.h"

KisOpenGLCanvasPainter::KisOpenGLCanvasPainter()
: m_active(false), m_widget(0)
{
}

KisOpenGLCanvasPainter::KisOpenGLCanvasPainter(QGLWidget *widget)
    : m_active(true), m_widget(widget)
{
    prepareForDrawing();
}

KisOpenGLCanvasPainter::~KisOpenGLCanvasPainter()
{
   if (m_widget) {
        if (m_active) {
            end();
        }
        m_widget->doneCurrent();
    }
}

bool KisOpenGLCanvasPainter::begin(KisCanvasWidget *canvasWidget, bool /*unclipped*/)
{
    m_widget = dynamic_cast<QGLWidget *>(canvasWidget);

    if (m_widget != 0) {
        prepareForDrawing();
        return true;
    } else {
        return false;
    }
    return false;
}

void KisOpenGLCanvasPainter::prepareForDrawing()
{
    if (m_widget != 0) {
        m_widget->makeCurrent();
        m_active = true;
        save();
        glDrawBuffer(GL_FRONT);
        glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
        glEnable(GL_BLEND);

        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();

        m_window = QRect(0, 0, m_widget->width(), m_widget->height());
        m_viewport = m_window;
        updateViewTransformation();

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        setPen(m_defaultPen);
    }
}

void KisOpenGLCanvasPainter::updateViewTransformation()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // We don't set the GL viewport directly from the Qt one as the GL
    // has a limited size. Instead we fold it into the projection matrix.
    glViewport(0, 0, m_widget->width(), m_widget->height());
    glOrtho(0, m_widget->width(), m_widget->height(), 0, -1, 1);

    glTranslatef(m_viewport.x(), m_viewport.y(), 0.0);
    glScalef(static_cast<float>(m_viewport.width()) / m_window.width(),
             static_cast<float>(m_viewport.height()) / m_window.height(),
             1.0);
    glTranslatef(-m_window.x(), -m_window.y(), 0.0);
}

bool KisOpenGLCanvasPainter::end()
{
    if (m_active) {
        restore();
        m_active = false;
        return true;
    } else {
        return false;
    }
}

void KisOpenGLCanvasPainter::save()
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
}

void KisOpenGLCanvasPainter::restore()
{
    glPopAttrib();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
}

void KisOpenGLCanvasPainter::setPenStyle(Qt::PenStyle penStyle)
{
    if (penStyle == Qt::SolidLine) {
        glDisable(GL_LINE_STIPPLE);
    } else {
        GLushort lineStipple;

        switch (penStyle) {
        case Qt::NoPen:
            lineStipple = 0;
            break;
        default:
        case Qt::DashLine:
            lineStipple = 0x3fff;
            break;
        case Qt::DotLine:
            lineStipple = 0x3333;
            break;
        case Qt::DashDotLine:
            lineStipple = 0x33ff;
            break;
        case Qt::DashDotDotLine:
            lineStipple = 0x333f;
            break;
        }

        glEnable(GL_LINE_STIPPLE);
        glLineStipple(1, lineStipple);
    }
}

QFontMetrics KisOpenGLCanvasPainter::fontMetrics() const
{
    return QFontMetrics(QFont());
}

QFontInfo KisOpenGLCanvasPainter::fontInfo() const
{
    return QFontInfo(QFont());
}

const QFont& KisOpenGLCanvasPainter::font() const
{
    return m_defaultFont;
}

void KisOpenGLCanvasPainter::setFont(const QFont& /*font*/)
{
}

const QPen& KisOpenGLCanvasPainter::pen() const
{
    return m_defaultPen;
}

void KisOpenGLCanvasPainter::setPen(const QPen& pen)
{
    setPenStyle(pen.style());
}

void KisOpenGLCanvasPainter::setPen(Qt::PenStyle penStyle)
{
    setPenStyle(penStyle);
}

void KisOpenGLCanvasPainter::setPen(const QColor& /*color*/)
{
}

const QBrush& KisOpenGLCanvasPainter::brush() const
{
    return m_defaultBrush;
}

void KisOpenGLCanvasPainter::setBrush(const QBrush& /*brush*/)
{
}

void KisOpenGLCanvasPainter::setBrush(Qt::BrushStyle /*brushStyle*/)
{
}

void KisOpenGLCanvasPainter::setBrush(const QColor& /*color*/)
{
}

QPoint KisOpenGLCanvasPainter::pos() const
{
    return QPoint();
}

const QColor& KisOpenGLCanvasPainter::backgroundColor() const
{
    return m_defaultColor;
}

void KisOpenGLCanvasPainter::setBackgroundColor(const QColor& /*color*/)
{
}

Qt::Qt::BGMode KisOpenGLCanvasPainter::backgroundMode() const
{
    return Qt::TransparentMode;
}

void KisOpenGLCanvasPainter::setBackgroundMode(Qt::Qt::BGMode /*bgMode*/)
{
}

Qt::Qt::RasterOp KisOpenGLCanvasPainter::rasterOp() const
{
    return Qt::CopyROP;
}

void KisOpenGLCanvasPainter::setRasterOp(Qt::RasterOp /*rasterOp*/)
{
}

const QPoint& KisOpenGLCanvasPainter::brushOrigin() const
{
    return m_defaultBrushOrigin;
}

void KisOpenGLCanvasPainter::setBrushOrigin(int /*x*/, int /*y*/)
{
}

void KisOpenGLCanvasPainter::setBrushOrigin(const QPoint& /*origin*/)
{
}

bool KisOpenGLCanvasPainter::hasViewXForm() const
{
    return false;
}

bool KisOpenGLCanvasPainter::hasWorldXForm() const
{
    return false;
}

void KisOpenGLCanvasPainter::setViewXForm(bool /*enable*/)
{
}

QRect KisOpenGLCanvasPainter::window() const
{
    return m_window;
}

void KisOpenGLCanvasPainter::setWindow(const QRect& r)
{
    m_window = r;
    updateViewTransformation();
}

void KisOpenGLCanvasPainter::setWindow(int x, int y, int w, int h)
{
    setWindow(QRect(x, y, w, h));
}

QRect KisOpenGLCanvasPainter::viewport() const
{
    return m_viewport;
}

void KisOpenGLCanvasPainter::setViewport(const QRect& r)
{
    m_viewport = r;
    updateViewTransformation();
}

void KisOpenGLCanvasPainter::setViewport(int x, int y, int w, int h)
{
    setViewport(QRect(x, y, w, h));
}

void KisOpenGLCanvasPainter::setWorldXForm(bool /*enable*/)
{
}

const QWMatrix& KisOpenGLCanvasPainter::worldMatrix() const
{
    return m_defaultWorldMatrix;
}

void KisOpenGLCanvasPainter::setWorldMatrix(const QWMatrix& /*matrix*/, bool /*combine*/)
{
}

void KisOpenGLCanvasPainter::saveWorldMatrix()
{
}

void KisOpenGLCanvasPainter::restoreWorldMatrix()
{
}

void KisOpenGLCanvasPainter::scale(double /*sx*/, double /*sy*/)
{
}

void KisOpenGLCanvasPainter::shear(double /*sh*/, double /*sv*/)
{
}

void KisOpenGLCanvasPainter::rotate(double /*a*/)
{
}

void KisOpenGLCanvasPainter::translate(double dx, double dy)
{
    glMatrixMode(GL_MODELVIEW);
    glTranslated(dx, dy, 0.0);
}

void KisOpenGLCanvasPainter::resetXForm()
{
}

double KisOpenGLCanvasPainter::translationX() const
{
    return 0;
}

double KisOpenGLCanvasPainter::translationY() const
{
    return 0;
}

QPoint KisOpenGLCanvasPainter::xForm(const QPoint& point) const
{
    return point;
}

QRect KisOpenGLCanvasPainter::xForm(const QRect& r) const
{
    return r;
}

QPointArray KisOpenGLCanvasPainter::xForm(const QPointArray& pointArray) const
{
    return pointArray;
}

QPointArray KisOpenGLCanvasPainter::xForm(const QPointArray& pointArray, int /*index*/, int /*npoints*/) const
{
    return pointArray;
}

QPoint KisOpenGLCanvasPainter::xFormDev(const QPoint& point) const
{
    return point;
}

QRect KisOpenGLCanvasPainter::xFormDev(const QRect& r) const
{
    return r;
}

QPointArray KisOpenGLCanvasPainter::xFormDev(const QPointArray& pointArray) const
{
    return pointArray;
}

QPointArray KisOpenGLCanvasPainter::xFormDev(const QPointArray& pointArray, int /*index*/, int /*npoints*/) const
{
    return pointArray;
}

void KisOpenGLCanvasPainter::setClipping(bool /*enable*/)
{
}

bool KisOpenGLCanvasPainter::hasClipping() const
{
    return true;
}

QRegion KisOpenGLCanvasPainter::clipRegion(QPainter::CoordinateMode /*mode*/) const
{
    return QRegion();
}

void KisOpenGLCanvasPainter::setClipRect(const QRect& /*r*/, QPainter::CoordinateMode /*mode*/)
{
}

void KisOpenGLCanvasPainter::setClipRect(int /*x*/, int /*y*/, int /*w*/, int /*h*/, QPainter::CoordinateMode /*mode*/)
{
}

void KisOpenGLCanvasPainter::setClipRegion(const QRegion& /*rgn*/, QPainter::CoordinateMode /*mode*/)
{
}

void KisOpenGLCanvasPainter::drawPoint(int x, int y)
{
    glBegin(GL_POINTS);
    glVertex2i(x, y);
    glEnd();
}

void KisOpenGLCanvasPainter::drawPoint(const QPoint& point)
{
    drawPoint(point.x(), point.y());
}

void KisOpenGLCanvasPainter::drawPoints(const QPointArray& pointArray, int index, int npoints)
{
    int firstPointIndex = index;

    if (firstPointIndex < 0) {
        firstPointIndex = 0;
    }
    if (firstPointIndex > (int)pointArray.count() - 1) {
        return;
    }

    int lastPointIndex;

    if (npoints < 0) {
        lastPointIndex = pointArray.count() - 1;
    } else {
        lastPointIndex = firstPointIndex + npoints;
        if (lastPointIndex > (int)pointArray.count() - 1) {
            lastPointIndex = pointArray.count() - 1;
        }
    }

    glBegin(GL_POINTS);

    for (int pointIndex = firstPointIndex; pointIndex <= lastPointIndex; pointIndex++) {
        QPoint point = pointArray.point(pointIndex);
        glVertex2i(point.x(), point.y());
    }

    glEnd();
}

void KisOpenGLCanvasPainter::moveTo(int /*x*/, int /*y*/)
{
}

void KisOpenGLCanvasPainter::moveTo(const QPoint& /*point*/)
{
}

void KisOpenGLCanvasPainter::lineTo(int /*x*/, int /*y*/)
{
}

void KisOpenGLCanvasPainter::lineTo(const QPoint& /*point*/)
{
}

void KisOpenGLCanvasPainter::drawLine(int x1, int y1, int x2, int y2)
{
    glBegin(GL_LINES);
    glVertex2i(x1, y1);
    glVertex2i(x2, y2);
    glEnd();
}

void KisOpenGLCanvasPainter::drawLine(const QPoint& start, const QPoint& end)
{
    drawLine(start.x(), start.y(), end.x(), end.y());
}

void KisOpenGLCanvasPainter::drawRect(int x, int y, int w, int h)
{
    glBegin(GL_LINES);

    glVertex2i(x, y);
    glVertex2i(x + w - 1, y);

    glVertex2i(x + w - 1, y);
    glVertex2i(x + w - 1, y + h - 1);

    glVertex2i(x + w - 1, y + h - 1);
    glVertex2i(x, y + h - 1);

    glVertex2i(x, y + h - 1);
    glVertex2i(x, y);

    glEnd();
}

void KisOpenGLCanvasPainter::drawRect(const QRect& r)
{
    drawRect(r.x(), r.y(), r.width(), r.height());
}

void KisOpenGLCanvasPainter::drawWinFocusRect(int /*x*/, int /*y*/, int /*w*/, int /*h*/)
{
}

void KisOpenGLCanvasPainter::drawWinFocusRect(int /*x*/, int /*y*/, int /*w*/, int /*h*/, const QColor& /*bgColor*/)
{
}

void KisOpenGLCanvasPainter::drawWinFocusRect(const QRect& /*r*/)
{
}

void KisOpenGLCanvasPainter::drawWinFocusRect(const QRect& /*r*/, const QColor& /*bgColor*/)
{
}

void KisOpenGLCanvasPainter::drawRoundRect(int /*x*/, int /*y*/, int /*w*/, int /*h*/, int /*xRnd*/, int /*yRnd*/)
{
}

void KisOpenGLCanvasPainter::drawRoundRect(const QRect& /*r*/, int /*xRnd*/, int /*yRnd*/)
{
}

void KisOpenGLCanvasPainter::drawEllipse(int x, int y, int w, int h)
{
    QRect r(x, y, w, h);
    r = r.normalize();

    QPointArray points;

    points.makeEllipse(r.x(), r.y(), r.width(), r.height());
    drawPoints(points);
}

void KisOpenGLCanvasPainter::drawEllipse(const QRect& r)
{
    drawEllipse(r.x(), r.y(), r.width(), r.height());
}

void KisOpenGLCanvasPainter::drawArc(int /*x*/, int /*y*/, int /*w*/, int /*h*/, int /*a*/, int /*alen*/)
{
}

void KisOpenGLCanvasPainter::drawArc(const QRect& /*r*/, int /*a*/, int /*alen*/)
{
}

void KisOpenGLCanvasPainter::drawPie(int /*x*/, int /*y*/, int /*w*/, int /*h*/, int /*a*/, int /*alen*/)
{
}

void KisOpenGLCanvasPainter::drawPie(const QRect& /*r*/, int /*a*/, int /*alen*/)
{
}

void KisOpenGLCanvasPainter::drawChord(int /*x*/, int /*y*/, int /*w*/, int /*h*/, int /*a*/, int /*alen*/)
{
}

void KisOpenGLCanvasPainter::drawChord(const QRect& /*r*/, int /*a*/, int /*alen*/)
{
}

void KisOpenGLCanvasPainter::drawLineSegments(const QPointArray& /*pointArray*/, int /*index*/, int /*nlines*/)
{
}

void KisOpenGLCanvasPainter::drawPolyline(const QPointArray& pointArray, int index, int npoints)
{
    int firstPointIndex = index;

    if (firstPointIndex < 0) {
        firstPointIndex = 0;
    }
    if (firstPointIndex > (int)pointArray.count() - 2) {
        return;
    }

    int lastPointIndex;

    if (npoints < 0) {
        lastPointIndex = pointArray.count() - 1;
    } else {
        lastPointIndex = firstPointIndex + npoints - 1;
        if (lastPointIndex > (int)pointArray.count() - 1) {
            lastPointIndex = pointArray.count() - 1;
        }
    }

    if (firstPointIndex >= lastPointIndex) {
        return;
    }

    glBegin(GL_LINES);

    for (int pointIndex = firstPointIndex; pointIndex <= lastPointIndex; pointIndex++) {
        QPoint point = pointArray.point(pointIndex);
        glVertex2i(point.x(), point.y());
    }

    glEnd();
}

void KisOpenGLCanvasPainter::drawPolygon(const QPointArray& /*pointArray*/, bool /*winding*/, int /*index*/, int /*npoints*/)
{
}

void KisOpenGLCanvasPainter::drawConvexPolygon(const QPointArray& /*pointArray*/, int /*index*/, int /*npoints*/)
{
}

QPoint midpoint (const QPoint& P1, const QPoint& P2)
{
    QPoint temp;
    temp.setX((P1.x()+P2.x())/2);
    temp.setY((P1.y()+P2.y())/2);
    return temp;
}

#define MAX_LEVEL 5

void recursiveCurve (const QPoint& P1, const QPoint& P2, const QPoint& P3,
                     const QPoint& P4, int level, QValueList<QPoint>& dest)
{
    if (level > MAX_LEVEL) {
        dest.append(midpoint(P1,P4));
        return;
    }

    QPoint L1, L2, L3, L4;
    QPoint H, R1, R2, R3, R4;

    L1 = P1;
    L2 = midpoint(P1, P2);
    H  = midpoint(P2, P3);
    R3 = midpoint(P3, P4);
    R4 = P4;
    L3 = midpoint(L2, H);
    R2 = midpoint(R3, H);
    L4 = midpoint(L3, R2);
    R1 = L4;
    recursiveCurve(L1, L2, L3, L4, level + 1, dest);
    recursiveCurve(R1, R2, R3, R4, level + 1, dest);
}

void KisOpenGLCanvasPainter::drawCubicBezier(const QPointArray& pointArray, int index)
{
    QPoint P1, P2, P3, P4;
    QValueList<QPoint> dest;
    P1 = pointArray[index++];
    P2 = pointArray[index++];
    P3 = pointArray[index++];
    P4 = pointArray[index];

    recursiveCurve(P1, P2, P3, P4, 1, dest);

    glBegin(GL_LINES);

    for (QValueList<QPoint>::iterator it = dest.begin(); it != dest.end(); it++) {
        QPoint point = (*it);
        glVertex2i(point.x(), point.y());
    }

    glEnd();
}

void KisOpenGLCanvasPainter::drawPixmap(int /*x*/, int /*y*/, const QPixmap& /*pixmap*/, int /*sx*/, int /*sy*/, int /*sw*/, int /*sh*/)
{
}

void KisOpenGLCanvasPainter::drawPixmap(const QPoint& /*point*/, const QPixmap& /*pixmap*/, const QRect& /*sr*/)
{
}

void KisOpenGLCanvasPainter::drawPixmap(const QPoint& /*point*/, const QPixmap& /*pixmap*/)
{
}

void KisOpenGLCanvasPainter::drawPixmap(const QRect& /*r*/, const QPixmap& /*pixmap*/)
{
}

void KisOpenGLCanvasPainter::drawImage(int /*x*/, int /*y*/, const QImage& /*image*/, int /*sx*/, int /*sy*/, int /*sw*/, int /*sh*/, int /*conversionFlags*/)
{
}

void KisOpenGLCanvasPainter::drawImage(const QPoint& /*point*/, const QImage& /*image*/, const QRect& /*sr*/, int /*conversionFlags*/)
{
}

void KisOpenGLCanvasPainter::drawImage(const QPoint& /*point*/, const QImage& /*image*/, int /*conversion_flags*/)
{
}

void KisOpenGLCanvasPainter::drawImage(const QRect& /*r*/, const QImage& /*image*/)
{
}

void KisOpenGLCanvasPainter::drawTiledPixmap(int /*x*/, int /*y*/, int /*w*/, int /*h*/, const QPixmap& /*pixmap*/, int /*sx*/, int /*sy*/)
{
}

void KisOpenGLCanvasPainter::drawTiledPixmap(const QRect& /*r*/, const QPixmap& /*pixmap*/, const QPoint& /*point*/)
{
}

void KisOpenGLCanvasPainter::drawTiledPixmap(const QRect& /*r*/, const QPixmap& /*pixmap*/)
{
}

void KisOpenGLCanvasPainter::fillRect(int x, int y, int w, int h, const QBrush& /*brush*/)
{
    // XXX: Set brush
    glRecti(x, y, x + w, y + h);
}

void KisOpenGLCanvasPainter::fillRect(const QRect& r, const QBrush& brush)
{
    fillRect(r.x(), r.y(), r.width(), r.height(), brush);
}

void KisOpenGLCanvasPainter::eraseRect(int /*x*/, int /*y*/, int /*w*/, int /*h*/)
{
}

void KisOpenGLCanvasPainter::eraseRect(const QRect& /*r*/)
{
}

void KisOpenGLCanvasPainter::drawText(int /*x*/, int /*y*/, const QString& /*text*/, int /*len*/, QPainter::TextDirection /*dir*/)
{
}

void KisOpenGLCanvasPainter::drawText(const QPoint& /*point*/, const QString& /*text*/, int /*len*/, QPainter::TextDirection /*dir*/)
{
}

void KisOpenGLCanvasPainter::drawText(int /*x*/, int /*y*/, const QString& /*text*/, int /*pos*/, int /*len*/, QPainter::TextDirection /*dir*/)
{
}

void KisOpenGLCanvasPainter::drawText(const QPoint& /*point*/, const QString& /*text*/, int /*pos*/, int /*len*/, QPainter::TextDirection /*dir*/)
{
}

void KisOpenGLCanvasPainter::drawText(int /*x*/, int /*y*/, int /*w*/, int /*h*/, int /*flags*/, const QString& /*text*/, int /*len*/, QRect */*br*/, QTextParag **/*intern*/)
{
}

void KisOpenGLCanvasPainter::drawText(const QRect& /*r*/, int /*flags*/, const QString& /*text*/, int /*len*/, QRect */*br*/, QTextParag **/*intern*/)
{
}

void KisOpenGLCanvasPainter::drawTextItem(int /*x*/, int /*y*/, const QTextItem& /*ti*/, int /*textflags*/)
{
}

void KisOpenGLCanvasPainter::drawTextItem(const QPoint& /*p*/, const QTextItem& /*ti*/, int /*textflags*/)
{
}

QRect KisOpenGLCanvasPainter::boundingRect(int /*x*/, int /*y*/, int /*w*/, int /*h*/, int /*flags*/, const QString& /*text*/, int /*len*/, QTextParag **/*intern*/)
{
    return QRect();
}

QRect KisOpenGLCanvasPainter::boundingRect(const QRect& /*r*/, int /*flags*/, const QString& /*text*/, int /*len*/, QTextParag **/*intern*/)
{
    return QRect();
}

int	KisOpenGLCanvasPainter::tabStops() const
{
    return 0;
}

void KisOpenGLCanvasPainter::setTabStops(int /*ts*/)
{
}

int	*KisOpenGLCanvasPainter::tabArray() const
{
    return 0;
}

void KisOpenGLCanvasPainter::setTabArray(int */*ts*/)
{
}

#endif // HAVE_GL

