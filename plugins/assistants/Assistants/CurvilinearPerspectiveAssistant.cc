/*
 * SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 * SPDX-FileCopyrightText: 2010 Geoffry Song <goffrie@gmail.com>
 * SPDX-FileCopyrightText: 2014 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 * SPDX-FileCopyrightText: 2017 Scott Petrovic <scottpetrovic@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "CurvilinearPerspectiveAssistant.h"

#include "../../../libs/ui/tool/kis_tool.h"
#include "kis_debug.h"
#include <klocalizedstring.h>

#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QTransform>

#include <kis_canvas2.h>
#include <kis_coordinates_converter.h>
#include <kis_algebra_2d.h>

#include <math.h>
#include <limits>

CurvilinearPerspectiveAssistant::CurvilinearPerspectiveAssistant()
    : KisPaintingAssistant("curvilinear-perspective", i18n("Curvilinear Perspective assistant"))
{
}

CurvilinearPerspectiveAssistant::CurvilinearPerspectiveAssistant(const CurvilinearPerspectiveAssistant &rhs, QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap)
    : KisPaintingAssistant(rhs, handleMap)
{
}

KisPaintingAssistantSP CurvilinearPerspectiveAssistant::clone(QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap) const
{
    return KisPaintingAssistantSP(new CurvilinearPerspectiveAssistant(*this, handleMap));
}

QPointF CurvilinearPerspectiveAssistant::project(const QPointF& pt, const QPointF& strokeBegin, qreal moveThresholdPt)
{
    const static QPointF nullPoint(std::numeric_limits<qreal>::quiet_NaN(), std::numeric_limits<qreal>::quiet_NaN());
    Q_ASSERT(isAssistantComplete());

    if (KisAlgebra2D::norm(pt - strokeBegin) < moveThresholdPt) {
        // allow some movement before snapping
        return strokeBegin;
    }

    return nullPoint;

}

QPointF CurvilinearPerspectiveAssistant::adjustPosition(const QPointF& pt, const QPointF& strokeBegin, const bool /*snapToAny*/, qreal moveThresholdPt)
{
    return project(pt, strokeBegin, moveThresholdPt);
}

void CurvilinearPerspectiveAssistant::adjustLine(QPointF &point, QPointF &strokeBegin)
{
    point = QPointF();
    strokeBegin = QPointF();
}

void CurvilinearPerspectiveAssistant::drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool cached, KisCanvas2* canvas, bool assistantVisible, bool previewVisible)
{
    gc.save();
    gc.resetTransform();

    QPointF mousePos(0,0);

    if (canvas){
        //simplest, cheapest way to get the mouse-position//
        mousePos= canvas->canvasWidget()->mapFromGlobal(QCursor::pos());
    }
    else {
        //...of course, you need to have access to a canvas-widget for that.//
        mousePos = QCursor::pos();//this'll give an offset//
        dbgFile<<"canvas does not exist in ruler, you may have passed arguments incorrectly:"<<canvas;
    }

    if (isAssistantComplete()){

        QTransform initialTransform = converter->documentToWidgetTransform();
        QPainterPath path;

        gc.setTransform(initialTransform);

        /*
         * Curvilinear perspective is created by circular arcs that intersect 2 vanishing points.
         * As such, the center of the circle and the radius of the circle need to be determined.
         * 
         * Create guidelines by selecting incremental multipliers for the assistant size between [-1, 1),
         * and calculating the center location and radius of the circle to include the point 
         * at the location specified by the multiplier (the "arbitary point").
         * 
         * Formulas: 
         * Radius^2 = HalfHandleDist^2 + CenterDist^2 (b.c. The circle must include both vanishing points.)
         * Radius^2 = (CenterDist + Multiplier * HalfHandleDist)^2 (b.c. The circle must include the arbitrary point)
         * 
         * Solve for CenterDist and Radius:
         * CenterDist = HalfHandleDist * (1 - Multipler * Multiplier) / ( 2 * Multiplier)
         * Radius     = HalfHandleDist * (1 + Multipler * Multiplier) / ( 2 * Multiplier)
         * 
         */
        
        QPointF p1 = *handles()[0];
        QPointF p2 = *handles()[1];

        double deltaX = p2.x() - p1.x();
        double deltaY = p2.y() - p1.y();

        double handleDistance = sqrt(pow(deltaX, 2) + pow(deltaY, 2));
        double halfHandleDist = handleDistance / 2.0;

        double avgX = deltaX / 2.0 + p1.x();
        double avgY = deltaY / 2.0 + p1.y();

        // Rotate 90 degrees by formula: (-y, x)
        // Then normalize vector.
        double dirX = -deltaY / handleDistance;
        double dirY = deltaX / handleDistance;

        int resolution = 5;

        for(int i = -resolution; i < resolution; i++) {
            // If i = 0, the circle would be infinitely far away with an infinite radius (aka a line)
            if(i == 0) {
                path.moveTo(QPointF(p1.x() - deltaX*2, p1.y() - deltaY*2));
                path.lineTo(QPointF(p2.x() + deltaX*2, p2.y() + deltaY*2));
                continue;
            }
            // Map loop iterator to multiplier
            double mult = ((double) i) / resolution;
            // Use formula to calculate CenterDist
            double centerDist = halfHandleDist * (1 - mult*mult) / (2*mult);

            // Use the distance to the center (from the average point) to calculate the center location.
            double circleCenterX = centerDist * dirX + avgX;
            double circleCenterY = centerDist * dirY + avgY;
            // Use formula to calculate Radius
            double radius = halfHandleDist * (1 + mult*mult) / (2*mult);

            path.addEllipse(QPointF(circleCenterX, circleCenterY), radius, radius);

        }
        
        // Draw guideline for the mouse, based on mouse position.
        // Get location on the screen of handles.
        QPointF screenP1 = initialTransform.map(*handles()[0]);
        QPointF screenP2 = initialTransform.map(*handles()[1]);
        // Don't draw if mouse is too close to vanishing points (will flicker if not)
        // Use distance squared to avoid expensive sqrt.
        if(
            (pow(mousePos.x() - screenP2.x(), 2) + pow(mousePos.y() - screenP2.y(), 2)) > 9 &&
            (pow(mousePos.x() - screenP1.x(), 2) + pow(mousePos.y() - screenP1.y(), 2)) > 9
        ) {
            /*
             * Calculate center location and radius for an arbitrary point (the mouse location).
             * Given Formulas:
             * Radius^2 = HalfHandleDist^2 + CenterDist^2
             * avgX + CenterDist * dirX = CenterX
             * avgY + CenterDist * dirY = CenterY
             * 
             * For ease of use, let BetaX = MouseX - AvgX, BetaY = MouseY - AvgY
             * Calculated Formula for CenterDist:
             * CenterDist = (BetaX^2 + BetaY^2 - HalfHandleDist^2) / (2 * DirY * BetaX + 2 * DirY * BetaY)
             */
            QPointF docMousePos = initialTransform.inverted().map(mousePos);
            double betaX = docMousePos.x() - avgX;
            double betaY = docMousePos.y() - avgY;

            double centerDist = 
                (betaX * betaX + betaY * betaY - halfHandleDist*halfHandleDist) 
                / 
                (2 * dirX * betaX + 2 * dirY * betaY);
            
            double circleCenterX = centerDist*dirX + avgX;
            double circleCenterY = centerDist*dirY + avgY;
            double radius = sqrt(pow(centerDist, 2) + pow(halfHandleDist, 2));
            path.addEllipse(QPointF(circleCenterX, circleCenterY), radius, radius);
        }

        drawPath(gc, path, isSnappingActive());

    }
    gc.restore();

    KisPaintingAssistant::drawAssistant(gc, updateRect, converter, cached, canvas, assistantVisible, previewVisible);

}

void CurvilinearPerspectiveAssistant::drawCache(QPainter& gc, const KisCoordinatesConverter *converter, bool assistantVisible)
{
    Q_UNUSED(gc);
    Q_UNUSED(converter);
    Q_UNUSED(assistantVisible);
}

QPointF CurvilinearPerspectiveAssistant::getDefaultEditorPosition() const
{
    return (*handles()[0] + *handles()[1]) * 0.5;
}

bool CurvilinearPerspectiveAssistant::isAssistantComplete() const
{
    return handles().size() >= 2;
}


CurvilinearPerspectiveAssistantFactory::CurvilinearPerspectiveAssistantFactory()
{
}

CurvilinearPerspectiveAssistantFactory::~CurvilinearPerspectiveAssistantFactory()
{
}

QString CurvilinearPerspectiveAssistantFactory::id() const
{
    return "curvilinear-perspective";
}

QString CurvilinearPerspectiveAssistantFactory::name() const
{
    return i18n("Curvilinear Perspective");
}

KisPaintingAssistant* CurvilinearPerspectiveAssistantFactory::createPaintingAssistant() const
{
    return new CurvilinearPerspectiveAssistant;
}
