/*
 * Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 * Copyright (c) 2010 Geoffry Song <goffrie@gmail.com>
 * Copyright (c) 2014 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 * Copyright (c) 2017 Scott Petrovic <scottpetrovic@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "FisheyePointAssistant.h"

#include "kis_debug.h"
#include <klocalizedstring.h>

#include <QPainter>
#include <QLinearGradient>
#include <QTransform>

#include <kis_canvas2.h>
#include <kis_coordinates_converter.h>
#include <kis_algebra_2d.h>

#include <math.h>
#include <limits>

FisheyePointAssistant::FisheyePointAssistant()
        : KisPaintingAssistant("fisheye-point", i18n("Fish Eye Point assistant"))
{
}

FisheyePointAssistant::FisheyePointAssistant(const FisheyePointAssistant &rhs, QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandlebSP> &handleMap)
    : KisPaintingAssistant(rhs, handleMap)
    , e(rhs.e)
    , extraE(rhs.extraE)
{
}

KisPaintingAssistantSP FisheyePointAssistant::clone(QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap) const
{
    return KisPaintingAssistantSP(new FisheyePointAssistant(*this, handleMap));
}

QPointF FisheyePointAssistant::project(const QPointF& pt, const QPointF& strokeBegin)
{
    const static QPointF nullPoint(std::numeric_limits<qreal>::quiet_NaN(), std::numeric_limits<qreal>::quiet_NaN());
    Q_ASSERT(isAssistantComplete());
    e.set(*handles()[0], *handles()[1], *handles()[2]);

    qreal
            dx = pt.x() - strokeBegin.x(),
            dy = pt.y() - strokeBegin.y();
        if (dx * dx + dy * dy < 4.0) {
            // allow some movement before snapping
            return strokeBegin;
        }

    //set the extrapolation ellipse.
    if (e.set(*handles()[0], *handles()[1], *handles()[2])){
        QLineF radius(*handles()[1], *handles()[0]);
        radius.setAngle(fmod(radius.angle()+180.0,360.0));
        QLineF radius2(*handles()[0], *handles()[1]);
        radius2.setAngle(fmod(radius2.angle()+180.0,360.0));
        if ( extraE.set(*handles()[0], *handles()[1],strokeBegin ) ) {
            return extraE.project(pt);
        } else if (extraE.set(radius.p1(), radius.p2(),strokeBegin)) {
            return extraE.project(pt);
        } else if (extraE.set(radius2.p1(), radius2.p2(),strokeBegin)){
            return extraE.project(pt);
        }   
    }
    
    return nullPoint;
    
}

QPointF FisheyePointAssistant::adjustPosition(const QPointF& pt, const QPointF& strokeBegin)
{
    return project(pt, strokeBegin);
}

void FisheyePointAssistant::drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool cached, KisCanvas2* canvas, bool assistantVisible, bool previewVisible)
{
    gc.save();
    gc.resetTransform();
    QPointF delta(0,0);
    QPointF mousePos(0,0);
    QPointF endPoint(0,0);//this is the final point that the line is being extended to, we seek it just outside the view port//
    QPointF otherHandle(0,0);
    
    if (canvas){
        //simplest, cheapest way to get the mouse-position//
        mousePos= canvas->canvasWidget()->mapFromGlobal(QCursor::pos());
    }
    else {
        //...of course, you need to have access to a canvas-widget for that.//
        mousePos = QCursor::pos();//this'll give an offset//
        dbgFile<<"canvas does not exist in ruler, you may have passed arguments incorrectly:"<<canvas;
    }
    
    QTransform initialTransform = converter->documentToWidgetTransform();
    
    if (isSnappingActive() && previewVisible == true ) {

        if (isAssistantComplete()){
                
            if (e.set(*handles()[0], *handles()[1], *handles()[2])) {
                if (extraE.set(*handles()[0], *handles()[1], initialTransform.inverted().map(mousePos))){
                    gc.setTransform(initialTransform);
                    gc.setTransform(e.getInverse(), true);
                    QPainterPath path;
                    // Draw the ellipse
                    path.addEllipse(QPointF(0, 0), extraE.semiMajor(), extraE.semiMinor());
                    drawPreview(gc, path);
                }
                QLineF radius(*handles()[1], *handles()[0]);
                radius.setAngle(fmod(radius.angle()+180.0,360.0));
                if (extraE.set(radius.p1(), radius.p2(), initialTransform.inverted().map(mousePos))){
                    gc.setTransform(initialTransform);
                    gc.setTransform(extraE.getInverse(), true);
                    QPainterPath path;
                    // Draw the ellipse
                    path.addEllipse(QPointF(0, 0), extraE.semiMajor(), extraE.semiMinor());
                    drawPreview(gc, path);
                }
                QLineF radius2(*handles()[0], *handles()[1]);
                radius2.setAngle(fmod(radius2.angle()+180.0,360.0));
                if (extraE.set(radius2.p1(), radius2.p2(), initialTransform.inverted().map(mousePos))){
                    gc.setTransform(initialTransform);
                    gc.setTransform(extraE.getInverse(), true);
                    QPainterPath path;
                    // Draw the ellipse
                    path.addEllipse(QPointF(0, 0), extraE.semiMajor(), extraE.semiMinor());
                    drawPreview(gc, path);
                }
                
            }
        }
    }
    gc.restore();
    
    KisPaintingAssistant::drawAssistant(gc, updateRect, converter, cached, canvas, assistantVisible, previewVisible);

}

void FisheyePointAssistant::drawCache(QPainter& gc, const KisCoordinatesConverter *converter, bool assistantVisible)
{
    if (assistantVisible == false){
        return;
    }
    
    QTransform initialTransform = converter->documentToWidgetTransform();

    if (handles().size() == 2) {
        // just draw the axis
        gc.setTransform(initialTransform);
        QPainterPath path;
        path.moveTo(*handles()[0]);
        path.lineTo(*handles()[1]);
        drawPath(gc, path, isSnappingActive());
        return;
    }
    if (e.set(*handles()[0], *handles()[1], *handles()[2])) {
        // valid ellipse

        gc.setTransform(initialTransform);
        gc.setTransform(e.getInverse(), true);
        QPainterPath path;
        //path.moveTo(QPointF(-e.semiMajor(), -e.semiMinor())); path.lineTo(QPointF(e.semiMajor(), -e.semiMinor()));
        path.moveTo(QPointF(-e.semiMajor(), -e.semiMinor())); path.lineTo(QPointF(-e.semiMajor(), e.semiMinor()));
        //path.moveTo(QPointF(-e.semiMajor(), e.semiMinor())); path.lineTo(QPointF(e.semiMajor(), e.semiMinor()));
        path.moveTo(QPointF(e.semiMajor(), -e.semiMinor())); path.lineTo(QPointF(e.semiMajor(), e.semiMinor()));
        path.moveTo(QPointF(-(e.semiMajor()*3), -e.semiMinor())); path.lineTo(QPointF(-(e.semiMajor()*3), e.semiMinor()));
        path.moveTo(QPointF((e.semiMajor()*3), -e.semiMinor())); path.lineTo(QPointF((e.semiMajor()*3), e.semiMinor()));
        path.moveTo(QPointF(-e.semiMajor(), 0)); path.lineTo(QPointF(e.semiMajor(), 0));
        //path.moveTo(QPointF(0, -e.semiMinor())); path.lineTo(QPointF(0, e.semiMinor()));
        // Draw the ellipse
        path.addEllipse(QPointF(0, 0), e.semiMajor(), e.semiMinor());
        drawPath(gc, path, isSnappingActive());
    }
    
}

QRect FisheyePointAssistant::boundingRect() const
{
    if (!isAssistantComplete()) {
        return KisPaintingAssistant::boundingRect();
    }

    if (e.set(*handles()[0], *handles()[1], *handles()[2])) {
        return e.boundingRect().adjusted(-(e.semiMajor()*2), -2, (e.semiMajor()*2), 2).toAlignedRect();
    } else {
        return QRect();
    }
}

QPointF FisheyePointAssistant::buttonPosition() const
{
    return (*handles()[0] + *handles()[1]) * 0.5;
}

bool FisheyePointAssistant::isAssistantComplete() const
{
    return handles().size() >= 3;
}


FisheyePointAssistantFactory::FisheyePointAssistantFactory()
{
}

FisheyePointAssistantFactory::~FisheyePointAssistantFactory()
{
}

QString FisheyePointAssistantFactory::id() const
{
    return "fisheye-point";
}

QString FisheyePointAssistantFactory::name() const
{
    return i18n("Fish Eye Point");
}

KisPaintingAssistant* FisheyePointAssistantFactory::createPaintingAssistant() const
{
    return new FisheyePointAssistant;
}
