/*
 * SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 * SPDX-FileCopyrightText: 2010 Geoffry Song <goffrie@gmail.com>
 * SPDX-FileCopyrightText: 2014 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 * SPDX-FileCopyrightText: 2017 Scott Petrovic <scottpetrovic@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "CurvilinearPerspectiveAssistant.h"

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

void CurvilinearPerspectiveAssistant::adjustLine(QPointF &point, QPointF &strokeBegin)
{
    point = QPointF();
    strokeBegin = QPointF();
}

void CurvilinearPerspectiveAssistant::drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool cached, KisCanvas2* canvas, bool assistantVisible, bool previewVisible)
{
    Q_UNUSED(cached);
    Q_UNUSED(updateRect);

    gc.save();
    gc.resetTransform();
    
    if (isSnappingActive()) {

        QTransform initialTransform = converter->documentToWidgetTransform();
        QPainterPath baseGuidePath;
        QPainterPath mouseGuidePath;

        gc.setTransform(initialTransform);

        /*
         * Curvilinear perspective is created by circular arcs that intersect 2 vanishing points.
         * As such, the center of the circle and the radius of the circle need to be determined.
         * 
         * Create guidelines by selecting incremental multipliers for the assistant size between [-1, 1),
         * and calculating the center location and radius of the circle to include the point 
         * at the location specified by the multiplier (the "arbitrary point").
         * 
         * Formulas: 
         * Radius^2 = HalfHandleDist^2 + CenterDist^2 (b.c. The circle must include both vanishing points.)
         * Radius^2 = (CenterDist + Multiplier * HalfHandleDist)^2 (b.c. The circle must include the arbitrary point)
         * 
         * Solve for CenterDist and Radius:
         * CenterDist = HalfHandleDist * (1 - Multiplier * Multiplier) / ( 2 * Multiplier)
         * Radius     = HalfHandleDist * (1 + Multiplier * Multiplier) / ( 2 * Multiplier)
         * 
         */
        
        QPointF p1 = *handles()[0];
        QPointF p2 = *handles()[1];

        double deltaX = p2.x() - p1.x();
        double deltaY = p2.y() - p1.y();

        // Copied from Two-Point Perspective's fading effect for approaching vanishing points.
        // Set up the fading effect for the grid lines
        // Needed so the grid density doesn't look distracting
        QColor color = effectiveAssistantColor();
        QGradient fade = QLinearGradient(
            QPointF(p1.x() - deltaY, p1.y() + deltaX), 
            QPointF(p1.x() + deltaY, p1.y() - deltaX));
        
        color.setAlphaF(0.0);
        fade.setColorAt(0.42, effectiveAssistantColor());
        fade.setColorAt(0.5, color);
        fade.setColorAt(0.58, effectiveAssistantColor());
        const QPen pen = gc.pen();
        const QBrush new_brush = QBrush(fade);
        int width = 0;
        const QPen new_pen = QPen(new_brush, width, pen.style());
        gc.setPen(new_pen);

        double handleDistance = KisAlgebra2D::norm(QPointF(deltaX, deltaY));
        double halfHandleDist = handleDistance / 2.0;

        double avgX = deltaX / 2.0 + p1.x();
        double avgY = deltaY / 2.0 + p1.y();

        // Rotate 90 degrees by formula: (-y, x)
        // Then normalize vector.
        double dirX = -deltaY / handleDistance;
        double dirY = deltaX / handleDistance;

        int resolution = halfHandleDist / 3;

        if(assistantVisible) {
            
            for(int i = -resolution; i < resolution; i++) {
                // If i = 0, the circle would be infinitely far away with an infinite radius (aka a line)
                if(i == 0) {
                    baseGuidePath.moveTo(QPointF(p1.x() - deltaX*2, p1.y() - deltaY*2));
                    baseGuidePath.lineTo(QPointF(p2.x() + deltaX*2, p2.y() + deltaY*2));
                    continue;
                }
                // Map loop iterator to multiplier. This line gives the depth-like effect.
                double mult = 1.0 / i;
                // Use formula to calculate CenterDist
                double centerDist = halfHandleDist * (1 - pow2(mult)) / (2*mult);

                // Use the distance to the center (from the average point) to calculate the center location.
                double circleCenterX = centerDist * dirX + avgX;
                double circleCenterY = centerDist * dirY + avgY;
                // Use formula to calculate Radius
                double radius = halfHandleDist * (1 + pow2(mult)) / (2*mult);

                baseGuidePath.addEllipse(QPointF(circleCenterX, circleCenterY), radius, radius);

            }
            gc.drawPath(baseGuidePath);//drawPath(gc, baseGuidePath);
        }
        
        if(previewVisible) {
            // Draw guideline for the mouse, based on mouse position.
            QPointF mousePos = effectiveBrushPosition(converter, canvas);
            // Get location on the screen of handles.
            QPointF screenP1 = initialTransform.map(*handles()[0]);
            QPointF screenP2 = initialTransform.map(*handles()[1]);
            // Don't draw if mouse is too close to vanishing points (will flicker if not)
            // Use distance squared to avoid expensive sqrt.
            if(
                kisSquareDistance(mousePos, screenP2) > 9 &&
                kisSquareDistance(mousePos, screenP1) > 9
            ) {
                QLineF circle = identifyCircle(initialTransform.inverted().map(mousePos));
                double radius = circle.length();
                mouseGuidePath.addEllipse(circle.p1(), radius, radius);
            }

            gc.drawPath(mouseGuidePath);//drawPath(gc, mouseGuidePath);
        }

    }
    gc.restore();

    //KisPaintingAssistant::drawAssistant(gc, updateRect, converter, cached, canvas, assistantVisible, previewVisible);

}

void CurvilinearPerspectiveAssistant::drawCache(QPainter& gc, const KisCoordinatesConverter *converter, bool assistantVisible)
{
    Q_UNUSED(gc);
    Q_UNUSED(converter);
    Q_UNUSED(assistantVisible);
}

QLineF CurvilinearPerspectiveAssistant::identifyCircle(const QPointF thirdPoint) {
    /*
    * Calculate center location and radius for an arbitrary point (usually the mouse location).
    * Given Formulas:
    * Radius^2 = HalfHandleDist^2 + CenterDist^2
    * avgX + CenterDist * dirX = CenterX
    * avgY + CenterDist * dirY = CenterY
    * 
    * For ease of use, let BetaX = MouseX - AvgX, BetaY = MouseY - AvgY
    * Calculated Formula for CenterDist:
    * CenterDist = (BetaX^2 + BetaY^2 - HalfHandleDist^2) / (2 * DirY * BetaX + 2 * DirY * BetaY)
    * 
    * Returns line from center to the arbitrary point.
    * 
    */
    QPointF p1 = *handles()[0];
    QPointF p2 = *handles()[1];

    double deltaX = p2.x() - p1.x();
    double deltaY = p2.y() - p1.y();

    double handleDistance = KisAlgebra2D::norm(QPointF(deltaX, deltaY));
    double halfHandleDist = handleDistance / 2.0;

    double avgX = deltaX / 2.0 + p1.x();
    double avgY = deltaY / 2.0 + p1.y();

    double dirX = -deltaY / handleDistance;
    double dirY = deltaX / handleDistance;

    double betaX = thirdPoint.x() - avgX;
    double betaY = thirdPoint.y() - avgY;

    double centerDist = 
        (pow2(betaX) + pow2(betaY) - pow2(halfHandleDist)) 
        / 
        (2 * dirX * betaX + 2 * dirY * betaY);
    
    double circleCenterX = centerDist*dirX + avgX;
    double circleCenterY = centerDist*dirY + avgY;
    return QLineF(QPointF(circleCenterX, circleCenterY), thirdPoint);
}

QPointF CurvilinearPerspectiveAssistant::adjustPosition(const QPointF& pt, const QPointF& strokeBegin, const bool /*snapToAny*/, qreal /*moveThresholdPt*/)
{
    // Get the center and radius for the given point
    QLineF initialCircle = identifyCircle(strokeBegin);

    // Set the new point onto the circle.
    QLineF magnetizedCircle(initialCircle.p1(), pt);
    magnetizedCircle.setLength(initialCircle.length());

    return magnetizedCircle.p2();

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
