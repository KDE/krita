/* 
 * SPDX-FileCopyrightText: 2015 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * Based on the Digikam CIE Tongue widget
 * SPDX-FileCopyrightText: 2006-2013 Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * Any source code are inspired from lprof project and
 * SPDX-FileCopyrightText: 1998-2001 Marti Maria
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 **/
#include <QPointF>
#include <QPolygonF>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QImage>
#include <QTextStream>
#include <cmath>
#include <klocalizedstring.h>


#include "kis_tone_curve_widget.h"

class Q_DECL_HIDDEN KisToneCurveWidget::Private
{
public:
 
    Private() :
        profileDataAvailable(false),
        needUpdatePixmap(false),
        TRCGray(true),
        xBias(0),
        yBias(0),
        pxcols(0),
        pxrows(0),
        gridside(0)
        
    {
    }
    
    bool            profileDataAvailable;
    bool            needUpdatePixmap;
    bool            TRCGray;
    bool            TRCRGB;
 
    int             xBias;
    int             yBias;
    int             pxcols;
    int             pxrows;

    QPolygonF       ToneCurveGray;
    QPolygonF       ToneCurveRed;
    QPolygonF       ToneCurveGreen;
    QPolygonF       ToneCurveBlue;
 
    double          gridside;
 
    QPainter        painter;
    QPainter        painter2;
    QPixmap         pixmap;
    QPixmap         curvemap;
};

KisToneCurveWidget::KisToneCurveWidget(QWidget *parent) :
    QWidget(parent), d(new Private)
{
    /*this is a tone curve widget*/
}

KisToneCurveWidget::~KisToneCurveWidget()
{
    delete d;
}

void KisToneCurveWidget::setGreyscaleCurve(QPolygonF poly)
{
    d->ToneCurveGray = poly;
    d->TRCGray = true;
    d->TRCRGB = false;
    d->profileDataAvailable = true;
    d->needUpdatePixmap = true;
}

void KisToneCurveWidget::setRGBCurve(QPolygonF red, QPolygonF green, QPolygonF blue)
{
    d->ToneCurveRed = red;
    d->ToneCurveGreen = green;
    d->ToneCurveBlue = blue;
    d->profileDataAvailable = true;
    d->TRCGray = false;
    d->TRCRGB = true;
    d->needUpdatePixmap = true;
}
void KisToneCurveWidget::setCMYKCurve(QPolygonF cyan, QPolygonF magenta, QPolygonF yellow, QPolygonF key)
{
    d->ToneCurveRed = cyan;
    d->ToneCurveGreen = magenta;
    d->ToneCurveBlue = yellow;
    d->ToneCurveGray = key;
    d->profileDataAvailable = true;
    d->TRCGray = false;
    d->TRCRGB = false;
    d->needUpdatePixmap = true;
}
void KisToneCurveWidget::setProfileDataAvailable(bool dataAvailable)
{
    d->profileDataAvailable = dataAvailable;
}
int KisToneCurveWidget::grids(double val) const
{
    return (int) floor(val * d->gridside + 0.5);
}

void KisToneCurveWidget::mapPoint(QPointF & xy)
{
    QPointF dummy = xy;
    xy.setX( (int) floor((dummy.x() * (d->pxcols - 1)) + .5) + d->xBias);
    xy.setY( (int) floor(((d->pxrows - 1) - dummy.y() * (d->pxrows - 1)) + .5) );
}

void KisToneCurveWidget::biasedLine(int x1, int y1, int x2, int y2)
{
    d->painter.drawLine(x1 + d->xBias, y1, x2 + d->xBias, y2);
}
 
void KisToneCurveWidget::biasedText(int x, int y, const QString& txt)
{
    d->painter.drawText(QPoint(d->xBias + x, y), txt);
}

void KisToneCurveWidget::drawGrid()
{
    d->painter.setOpacity(1.0);
    d->painter.setPen(qRgb(255, 255, 255));
    biasedLine(0, 0,           0,           d->pxrows - 1);
    biasedLine(0, d->pxrows-1, d->pxcols-1, d->pxrows - 1);
    
    QFont font;
    font.setPointSize(6);
    d->painter.setFont(font);
    
    for (int y = 1; y <= 9; y += 1)
    {
        QString s;
        int xstart = (y * (d->pxcols - 1)) / 10;
        int ystart = (y * (d->pxrows - 1)) / 10;
 
        QTextStream(&s) << y;
        biasedLine(xstart, d->pxrows - grids(1), xstart,   d->pxrows - grids(4));
        biasedText(xstart - grids(11), d->pxrows + grids(15), s);
 
        QTextStream(&s) << 10 - y;
        biasedLine(0, ystart, grids(3), ystart);
        biasedText(grids(-25), ystart + grids(5), s);
    }
    
    d->painter.setPen(qRgb(128, 128, 128));
    d->painter.setOpacity(0.5);
 
    for (int y = 1; y <= 9; y += 1)
    {
        int xstart =  (y * (d->pxcols - 1)) / 10;
        int ystart =  (y * (d->pxrows - 1)) / 10;
 
        biasedLine(xstart, grids(4), xstart,   d->pxrows - grids(4) - 1);
        biasedLine(grids(7), ystart, d->pxcols-1-grids(7), ystart);
    }
    d->painter.setOpacity(1.0);
    d->painter.setOpacity(1.0);
}

void KisToneCurveWidget::updatePixmap()
{
    d->needUpdatePixmap = false;
    d->pixmap = QPixmap(size());
    d->curvemap = QPixmap(size());
    d->pixmap.fill(Qt::black);
    d->curvemap.fill(Qt::transparent);

    d->painter.begin(&d->pixmap);
    

    int pixcols = d->pixmap.width();
    int pixrows = d->pixmap.height();

    d->gridside = (qMin(pixcols, pixrows)) / 512.0;
    d->xBias    = grids(32);
    d->yBias    = grids(20);
    d->pxcols   = pixcols - d->xBias;
    d->pxrows   = pixrows - d->yBias;

    d->painter.setBackground(QBrush(qRgb(0, 0, 0)));
    QPointF start;
    drawGrid();
    d->painter.setRenderHint(QPainter::Antialiasing);
    if (d->TRCGray && d->ToneCurveGray.size()>0){
        QPainterPath path;
        start = d->ToneCurveGray.at(0);
        mapPoint(start);
        path.moveTo(start);
        foreach (QPointF Point, d->ToneCurveGray) {
            mapPoint(Point);
            path.lineTo(Point);
        }
        d->painter.setPen(qRgb(255, 255, 255));
        d->painter.drawPath(path);
    } else if (d->TRCRGB && d->ToneCurveRed.size()>0 && d->ToneCurveGreen.size()>0 && d->ToneCurveBlue.size()>0){
        d->painter.save();
        d->painter.setCompositionMode(QPainter::CompositionMode_Screen);
        QPainterPath path;
        start = d->ToneCurveRed.at(0);
        mapPoint(start);
        path.moveTo(start);
        foreach (QPointF Point, d->ToneCurveRed) {
            mapPoint(Point);
            path.lineTo(Point);
        }
        d->painter.setPen(qRgb(255, 0, 0));
        d->painter.drawPath(path);
        QPainterPath path2;
        start = d->ToneCurveGreen.at(0);
        mapPoint(start);
        path2.moveTo(start);
        foreach (QPointF Point, d->ToneCurveGreen) {
            mapPoint(Point);
            path2.lineTo(Point);
        }
        d->painter.setPen(qRgb(0, 255, 0));
        d->painter.drawPath(path2);
        QPainterPath path3;
        start = d->ToneCurveBlue.at(0);
        mapPoint(start);
        path3.moveTo(start);
        foreach (QPointF Point, d->ToneCurveBlue) {
            mapPoint(Point);
            path3.lineTo(Point);
        }
        d->painter.setPen(qRgb(0, 0, 255));
        d->painter.drawPath(path3);
        d->painter.restore();
    } else {
        d->painter2.begin(&d->curvemap);
        d->painter2.setRenderHint(QPainter::Antialiasing);
        //d->painter2.setCompositionMode(QPainter::CompositionMode_Multiply);
        QPainterPath path;
        start = d->ToneCurveRed.at(0);
        mapPoint(start);
        path.moveTo(start);
        foreach (QPointF Point, d->ToneCurveRed) {
            mapPoint(Point);
            path.lineTo(Point);
        }
        d->painter2.setPen(qRgb(0, 255, 255));
        d->painter2.drawPath(path);
        QPainterPath path2;
        start = d->ToneCurveGreen.at(0);
        mapPoint(start);
        path2.moveTo(start);
        foreach (QPointF Point, d->ToneCurveGreen) {
            mapPoint(Point);
            path2.lineTo(Point);
        }
        d->painter2.setPen(qRgb(255, 0, 255));
        d->painter2.drawPath(path2);
        QPainterPath path3;
        start = d->ToneCurveBlue.at(0);
        mapPoint(start);
        path3.moveTo(start);
        foreach (QPointF Point, d->ToneCurveBlue) {
            mapPoint(Point);
            path3.lineTo(Point);
        }
        d->painter2.setPen(qRgb(255, 255, 0));
        d->painter2.drawPath(path3);
        QPainterPath path4;
        start = d->ToneCurveGray.at(0);
        mapPoint(start);
        path4.moveTo(start);
        foreach (QPointF Point, d->ToneCurveGray) {
            mapPoint(Point);
            path4.lineTo(Point);
        }
        d->painter2.setPen(qRgb(80, 80, 80));
        d->painter2.drawPath(path4);
        d->painter2.end();
        QRect area(d->xBias, 0, d->pxcols, d->pxrows);
        d->painter.drawPixmap(area,d->curvemap, area);
    }
    d->painter.end();

}

void KisToneCurveWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
 
    // Widget is disable : drawing grayed frame.
 
    if ( !isEnabled() )
    {
        p.fillRect(0, 0, width(), height(),
                   palette().color(QPalette::Disabled, QPalette::Background));
 
        QPen pen(palette().color(QPalette::Disabled, QPalette::Foreground));
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(1);
 
        p.setPen(pen);
        p.drawRect(0, 0, width(), height());
 
        return;
    }

 
    // No profile data to show, or RAW file
 
    if (!d->profileDataAvailable)
    {
        p.fillRect(0, 0, width(), height(), palette().color(QPalette::Active, QPalette::Background));
        QPen pen(palette().color(QPalette::Active, QPalette::Text));
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(1);
 
        p.setPen(pen);
        p.drawRect(0, 0, width(), height());
 
        p.setPen(Qt::red);
        p.drawText(0, 0, width(), height(), Qt::AlignCenter,
        i18n("No tone curve available..."));
 
        return;
    }
 
    // Create CIE tongue if needed
    if (d->needUpdatePixmap)
    {
        updatePixmap();
    }
 
    // draw prerendered tongue
    p.drawPixmap(0, 0, d->pixmap);
}
 
void KisToneCurveWidget::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    setMinimumWidth(height());
    setMaximumWidth(height());
    d->needUpdatePixmap = true;
}
