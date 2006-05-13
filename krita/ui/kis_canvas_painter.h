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

#ifndef KIS_CANVAS_PAINTER_H_
#define KIS_CANVAS_PAINTER_H_

#include <config.h>

#include <QPainter>
//Added by qt3to4:
#include <QPixmap>
#include <QPolygon>

#include "kis_global.h"
#include <krita_export.h>

class KisCanvas;
class KisCanvasWidget;

class KRITAUI_EXPORT KisCanvasWidgetPainter {
public:
    KisCanvasWidgetPainter();
    virtual ~KisCanvasWidgetPainter();

    virtual bool begin(KisCanvasWidget *canvasWidget, bool unclipped = false) = 0;
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
    virtual QPolygon xForm(const QPolygon&) const;
    virtual QPolygon xForm(const QPolygon&, int index, int npoints) const;
    virtual QPoint xFormDev(const QPoint&) const;
    virtual QRect xFormDev(const QRect&)  const;
    virtual QPolygon xFormDev(const QPolygon&) const;
    virtual QPolygon xFormDev(const QPolygon&, int index, int npoints) const;

    virtual void setClipping(bool);
    virtual bool hasClipping() const;
    virtual QRegion clipRegion() const;
    virtual void setClipRect(const QRect&);
    virtual void setClipRect(int x, int y, int w, int h);
    virtual void setClipRegion(const QRegion&);

    virtual void drawPoint(int x, int y);
    virtual void drawPoint(const QPoint&);
    virtual void drawPoints(const QPolygon& a, int index=0, int npoints=-1);
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
    virtual void drawLineSegments(const QPolygon&, int index=0, int nlines=-1);
    virtual void drawPolyline(const QPolygon&, int index=0, int npoints=-1);
    virtual void drawPolygon(const QPolygon&, bool winding=FALSE, int index=0, int npoints=-1);
    virtual void drawConvexPolygon(const QPolygon&, int index=0, int npoints=-1);
    virtual void drawCubicBezier(const QPolygon&, int index=0);
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

//     virtual void drawText(int x, int y, const QString&, int len = -1, QPainter::TextDirection dir = QPainter::Auto);
//     virtual void drawText(const QPoint&, const QString&, int len = -1, QPainter::TextDirection dir = QPainter::Auto);
//
//     virtual void drawText(int x, int y, const QString&, int pos, int len, QPainter::TextDirection dir = QPainter::Auto);
//     virtual void drawText(const QPoint&p, const QString&, int pos, int len, QPainter::TextDirection dir = QPainter::Auto);
//
//     virtual void drawText(int x, int y, int w, int h, int flags, const QString&, int len = -1, QRect *br=0, QTextParag **intern=0);
//     virtual void drawText(const QRect&, int flags, const QString&, int len = -1, QRect *br=0, QTextParag **intern=0);
//
//     virtual void drawTextItem(int x, int y, const QTextItem&ti, int textflags = 0);
//     virtual void drawTextItem(const QPoint& p, const QTextItem&ti, int textflags = 0);
//
//     virtual QRect boundingRect(int x, int y, int w, int h, int flags, const QString&, int len = -1, QTextParag **intern=0);
//     virtual QRect boundingRect(const QRect&, int flags, const QString&, int len = -1, QTextParag **intern=0);

    virtual int	tabStops() const;
    virtual void setTabStops(int);
    virtual int	*tabArray() const;
    virtual void setTabArray(int *);

protected:
    QFont m_defaultFont;
    QPen m_defaultPen;
    QBrush m_defaultBrush;
    QColor m_defaultColor;
    QPoint m_defaultBrushOrigin;
    QMatrix m_defaultWorldMatrix;
};

class KRITAUI_EXPORT KisCanvasPainter {
public:
    KisCanvasPainter();
    KisCanvasPainter(KisCanvas *canvas);
    KisCanvasPainter(QPaintDevice *paintDevice);

    ~KisCanvasPainter();

    bool begin(KisCanvas *canvas, bool unclipped = false);
    bool begin(QPaintDevice *paintDevice, bool unclipped = false);

    bool end();

    void save();
    void restore();

    QFontMetrics fontMetrics() const;
    QFontInfo fontInfo() const;

    const QFont& font() const;
    void setFont(const QFont&);
    const QPen& pen() const;
    void setPen(const QPen&);
    void setPen(Qt::PenStyle);
    void setPen(const QColor&);
    const QBrush&brush() const;
    void setBrush(const QBrush&);
    void setBrush(Qt::BrushStyle);
    void setBrush(const QColor&);
    QPoint pos() const;

    const QColor&backgroundColor() const;
    void setBackgroundColor(const QColor&);
    Qt::BGMode backgroundMode() const;
    void setBackgroundMode(Qt::BGMode);
    //Qt::RasterOp rasterOp()	const;
    //void setRasterOp(Qt::RasterOp);
    const QPoint&brushOrigin() const;
    void setBrushOrigin(int x, int y);
    void setBrushOrigin(const QPoint&);

    bool hasViewXForm() const;
    bool hasWorldXForm() const;

    void setViewXForm(bool);
    QRect window() const;
    void setWindow(const QRect&);
    void setWindow(int x, int y, int w, int h);
    QRect viewport() const;
    void setViewport(const QRect&);
    void setViewport(int x, int y, int w, int h);

    void setWorldXForm(bool);
    const QMatrix&worldMatrix() const;
    void setMatrix(const QMatrix&, bool combine=FALSE);

    void saveWorldMatrix();
    void restoreWorldMatrix();

    void scale(double sx, double sy);
    void shear(double sh, double sv);
    void rotate(double a);

    void translate(double dx, double dy);
    void resetXForm();
    double translationX() const;
    double translationY() const;

    QPoint xForm(const QPoint&) const;
    QRect xForm(const QRect&)	const;
    QPolygon xForm(const QPolygon&) const;
    QPolygon xForm(const QPolygon&, int index, int npoints) const;
    QPoint xFormDev(const QPoint&) const;
    QRect xFormDev(const QRect&)  const;
    QPolygon xFormDev(const QPolygon&) const;
    QPolygon xFormDev(const QPolygon&, int index, int npoints) const;

    void setClipping(bool);
    bool hasClipping() const;
    QRegion clipRegion() const;
    void setClipRect(const QRect&);
    void setClipRect(int x, int y, int w, int h);
    void setClipRegion(const QRegion&);

    void drawPoint(int x, int y);
    void drawPoint(const QPoint&);
    void drawPoints(const QPolygon& a, int index=0, int npoints=-1);
    void moveTo(int x, int y);
    void moveTo(const QPoint&);
    void lineTo(int x, int y);
    void lineTo(const QPoint&);
    void drawLine(int x1, int y1, int x2, int y2);
    void drawLine(const QPoint&, const QPoint&);
    void drawRect(int x, int y, int w, int h);
    void drawRect(const QRect&);
    void drawWinFocusRect(int x, int y, int w, int h);
    void drawWinFocusRect(int x, int y, int w, int h, const QColor&bgColor);
    void drawWinFocusRect(const QRect&);
    void drawWinFocusRect(const QRect&, const QColor&bgColor);
    void drawRoundRect(int x, int y, int w, int h, int = 25, int = 25);
    void drawRoundRect(const QRect&, int = 25, int = 25);
    void drawEllipse(int x, int y, int w, int h);
    void drawEllipse(const QRect&);
    void drawArc(int x, int y, int w, int h, int a, int alen);
    void drawArc(const QRect&, int a, int alen);
    void drawPie(int x, int y, int w, int h, int a, int alen);
    void drawPie(const QRect&, int a, int alen);
    void drawChord(int x, int y, int w, int h, int a, int alen);
    void drawChord(const QRect&, int a, int alen);
    void drawLineSegments(const QPolygon&, int index=0, int nlines=-1);
    void drawPolyline(const QPolygon&, int index=0, int npoints=-1);
    void drawPolygon(const QPolygon&, bool winding=FALSE, int index=0, int npoints=-1);
    void drawConvexPolygon(const QPolygon&, int index=0, int npoints=-1);
    void drawCubicBezier(const QPolygon&, int index=0);
    void drawPixmap(int x, int y, const QPixmap&, int sx=0, int sy=0, int sw=-1, int sh=-1);
    void drawPixmap(const QPoint&, const QPixmap&, const QRect&sr);
    void drawPixmap(const QPoint&, const QPixmap&);
    void drawPixmap(const QRect&, const QPixmap&);
    void drawImage(int x, int y, const QImage&, int sx = 0, int sy = 0, int sw = -1, int sh = -1, int conversionFlags = 0);
    void drawImage(const QPoint&, const QImage&, const QRect&sr, int conversionFlags = 0);
    void drawImage(const QPoint&, const QImage&, int conversion_flags = 0);
    void drawImage(const QRect&, const QImage&);
    void drawTiledPixmap(int x, int y, int w, int h, const QPixmap&, int sx=0, int sy=0);
    void drawTiledPixmap(const QRect&, const QPixmap&, const QPoint&);
    void drawTiledPixmap(const QRect&, const QPixmap&);
    //void drawPicture(const QPicture&);
    //void drawPicture(int x, int y, const QPicture&);
    //void drawPicture(const QPoint&, const QPicture&);

    void fillRect(int x, int y, int w, int h, const QBrush&);
    void fillRect(const QRect&, const QBrush&);
    void eraseRect(int x, int y, int w, int h);
    void eraseRect(const QRect&);

//     void drawText(int x, int y, const QString&, int len = -1, QPainter::TextDirection dir = QPainter::Auto);
//     void drawText(const QPoint&, const QString&, int len = -1, QPainter::TextDirection dir = QPainter::Auto);
//
//     void drawText(int x, int y, const QString&, int pos, int len, QPainter::TextDirection dir = QPainter::Auto);
//     void drawText(const QPoint&p, const QString&, int pos, int len, QPainter::TextDirection dir = QPainter::Auto);
//
//     void drawText(int x, int y, int w, int h, int flags, const QString&, int len = -1, QRect *br=0, QTextParag **intern=0);
//     void drawText(const QRect&, int flags, const QString&, int len = -1, QRect *br=0, QTextParag **intern=0);
//
//     void drawTextItem(int x, int y, const QTextItem&ti, int textflags = 0);
//     void drawTextItem(const QPoint& p, const QTextItem&ti, int textflags = 0);
//
//     QRect boundingRect(int x, int y, int w, int h, int flags, const QString&, int len = -1, QTextParag **intern=0);
//     QRect boundingRect(const QRect&, int flags, const QString&, int len = -1, QTextParag **intern=0);

    int	tabStops() const;
    void setTabStops(int);
    int	*tabArray() const;
    void setTabArray(int *);

protected:
    KisCanvasWidgetPainter *m_canvasWidgetPainter;
    QFont m_defaultFont;
    QPen m_defaultPen;
    QBrush m_defaultBrush;
    QColor m_defaultColor;
    QPoint m_defaultBrushOrigin;
    QMatrix m_defaultWorldMatrix;
};

#endif // KIS_CANVAS_PAINTER_H_

