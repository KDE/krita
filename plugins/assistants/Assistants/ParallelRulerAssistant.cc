/*
 * SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 * SPDX-FileCopyrightText: 2010 Geoffry Song <goffrie@gmail.com>
 * SPDX-FileCopyrightText: 2014 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 * SPDX-FileCopyrightText: 2017 Scott Petrovic <scottpetrovic@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "ParallelRulerAssistant.h"

#include "kis_debug.h"
#include <klocalizedstring.h>

#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QTransform>

#include <kis_canvas2.h>
#include <kis_coordinates_converter.h>
#include <kis_algebra_2d.h>
#include <kis_dom_utils.h>

#include <math.h>

ParallelRulerAssistant::ParallelRulerAssistant()
    : KisPaintingAssistant("parallel ruler", i18n("Parallel Ruler assistant"))
    , m_followBrushPosition(false)
    , m_adjustedPositionValid(false)
{
}

KisPaintingAssistantSP ParallelRulerAssistant::clone(QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap) const
{
    return KisPaintingAssistantSP(new ParallelRulerAssistant(*this, handleMap));
}

ParallelRulerAssistant::ParallelRulerAssistant(const ParallelRulerAssistant &rhs, QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap)
    : KisPaintingAssistant(rhs, handleMap)
    , m_followBrushPosition(rhs.m_followBrushPosition)
    , m_adjustedPositionValid(rhs.m_adjustedPositionValid)
    , m_adjustedBrushPosition(rhs.m_adjustedBrushPosition)
{
}

void ParallelRulerAssistant::setAdjustedBrushPosition(const QPointF position)
{
    m_adjustedBrushPosition = position;
    m_adjustedPositionValid = true;
}

void ParallelRulerAssistant::endStroke()
{
    // Brush stroke ended, guides should follow the brush position again.
    m_followBrushPosition = false;
    m_adjustedPositionValid = false;
    m_hasBeenInsideLocalRect = false;
}


void ParallelRulerAssistant::setFollowBrushPosition(bool follow)
{
    m_followBrushPosition = follow;
}

QPointF ParallelRulerAssistant::project(const QPointF& pt, const QPointF& strokeBegin)
{
    Q_ASSERT(isAssistantComplete());

    //code nicked from the perspective ruler.
    qreal dx = pt.x() - strokeBegin.x();
    qreal dy = pt.y() - strokeBegin.y();

    if (dx * dx + dy * dy < 4.0) {
        return strokeBegin; // allow some movement before snapping
    }

    if (isLocal() && isAssistantComplete()) {
        if (getLocalRect().contains(pt)) {
            m_hasBeenInsideLocalRect = true;
        } else if (isLocal() && !m_hasBeenInsideLocalRect) {
            return QPointF(qQNaN(), qQNaN());
        }
    }

    //dbgKrita<<strokeBegin<< ", " <<*handles()[0];
    QLineF snapLine = QLineF(*handles()[0], *handles()[1]);
    QPointF translation = (*handles()[0]-strokeBegin)*-1.0;
    snapLine = snapLine.translated(translation);

    dx = snapLine.dx();
    dy = snapLine.dy();

    const qreal
            dx2 = dx * dx,
            dy2 = dy * dy,
            invsqrlen = 1.0 / (dx2 + dy2);
    QPointF r(dx2 * pt.x() + dy2 * snapLine.x1() + dx * dy * (pt.y() - snapLine.y1()),
              dx2 * snapLine.y1() + dy2 * pt.y() + dx * dy * (pt.x() - snapLine.x1()));
    r *= invsqrlen;
    return r;
    //return pt;
}

QPointF ParallelRulerAssistant::adjustPosition(const QPointF& pt, const QPointF& strokeBegin, const bool /*snapToAny*/)
{
    return project(pt, strokeBegin);
}

void ParallelRulerAssistant::drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool cached, KisCanvas2* canvas, bool assistantVisible, bool previewVisible)
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

    QRectF local = getLocalRect();

    QTransform initialTransform = converter->documentToWidgetTransform();

    if (isLocal() && isAssistantComplete()) {
        QPainterPath path;
        QRectF local = getLocalRect();
        // note: be careful; bottom and right only work with RectF, not Rect
        path.moveTo(initialTransform.map(local.topLeft()));

        path.lineTo(initialTransform.map(local.topRight()));
        path.lineTo(initialTransform.map(local.bottomRight()));
        path.lineTo(initialTransform.map(local.bottomLeft()));
        path.lineTo(initialTransform.map(local.topLeft()));
        drawPreview(gc, path);//and we draw the preview.
    }


    if (isAssistantComplete() && isSnappingActive() && previewVisible==true) {
        //don't draw if invalid.

        if (!isLocal() || local.contains(mousePos)) {

            QLineF snapLine= QLineF(initialTransform.map(*handles()[0]), initialTransform.map(*handles()[1]));

            if (m_followBrushPosition && m_adjustedPositionValid) {
                mousePos = initialTransform.map(m_adjustedBrushPosition);
            }

            QPointF translation = (initialTransform.map(*handles()[0])-mousePos)*-1.0;
            snapLine= snapLine.translated(translation);

            QRect viewport= gc.viewport();
            KisAlgebra2D::intersectLineRect(snapLine, viewport, true);


            QPainterPath path;
            path.moveTo(snapLine.p1());
            path.lineTo(snapLine.p2());

            drawPreview(gc, path);//and we draw the preview.
        }
    }
    gc.restore();

    KisPaintingAssistant::drawAssistant(gc, updateRect, converter, cached, canvas, assistantVisible, previewVisible);

}

void ParallelRulerAssistant::drawCache(QPainter& gc, const KisCoordinatesConverter *converter, bool assistantVisible)
{
    if (assistantVisible == false || handles().size() < 2) {
        return;
    }

    QTransform initialTransform = converter->documentToWidgetTransform();

    // Draw the line
    QPointF p1 = *handles()[0];
    QPointF p2 = *handles()[1];

    gc.setTransform(initialTransform);
    QPainterPath path;
    path.moveTo(p1);
    path.lineTo(p2);
    drawPath(gc, path, isSnappingActive());

}

KisPaintingAssistantHandleSP ParallelRulerAssistant::firstLocalHandle() const
{
    return handles().size() > 2 ? handles()[2] : 0;
}

KisPaintingAssistantHandleSP ParallelRulerAssistant::secondLocalHandle() const
{
    return handles().size() > 3 ? handles()[3] : 0;
}

QPointF ParallelRulerAssistant::getEditorPosition() const
{
    if (handles().size() > 1) {
        return (*handles()[0] + *handles()[1]) * 0.5;
    } else if (handles().size() > 0) {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(false, *handles()[0]);
        return *handles()[0];
    } else {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(false, QPointF(0, 0));
        return QPointF(0, 0);
    }
}

bool ParallelRulerAssistant::isAssistantComplete() const
{
    return handles().size() >= numHandles();
}

bool ParallelRulerAssistant::canBeLocal() const
{
    return true;
}

void ParallelRulerAssistant::saveCustomXml(QXmlStreamWriter *xml)
{
    xml->writeStartElement("isLocal");
    xml->writeAttribute("value", KisDomUtils::toString( (int)this->isLocal()));
    xml->writeEndElement();
}

bool ParallelRulerAssistant::loadCustomXml(QXmlStreamReader *xml)
{
    if (xml && xml->name() == "isLocal") {
        this->setLocal((bool)KisDomUtils::toInt(xml->attributes().value("value").toString()));
    }
    return true;
}

ParallelRulerAssistantFactory::ParallelRulerAssistantFactory()
{
}

ParallelRulerAssistantFactory::~ParallelRulerAssistantFactory()
{
}

QString ParallelRulerAssistantFactory::id() const
{
    return "parallel ruler";
}

QString ParallelRulerAssistantFactory::name() const
{
    return i18n("Parallel Ruler");
}

KisPaintingAssistant* ParallelRulerAssistantFactory::createPaintingAssistant() const
{
    return new ParallelRulerAssistant;
}
