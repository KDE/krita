/*
 * SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 * SPDX-FileCopyrightText: 2017 Scott Petrovic <scottpetrovic@gmail.com>
 * SPDX-FileCopyrightText: 2022 Julian Schmidt <julisch1107@web.de>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "RulerAssistant.h"

#include <kis_debug.h>
#include <klocalizedstring.h>

#include <QPainter>
#include <QPainterPath>
#include <QTransform>

#include <kis_canvas2.h>
#include <kis_coordinates_converter.h>
#include <kis_dom_utils.h>

#include <math.h>

RulerAssistant::RulerAssistant()
    : RulerAssistant("ruler", i18n("Ruler assistant"))
{
}

RulerAssistant::RulerAssistant(const QString& id, const QString& name)
    : KisPaintingAssistant(id, name)
{
}

KisPaintingAssistantSP RulerAssistant::clone(QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap) const
{
    return KisPaintingAssistantSP(new RulerAssistant(*this, handleMap));
}

RulerAssistant::RulerAssistant(const RulerAssistant &rhs, QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap)
    : KisPaintingAssistant(rhs, handleMap)
    , m_subdivisions(rhs.m_subdivisions)
    , m_minorSubdivisions(rhs.m_minorSubdivisions)
    , m_hasFixedLength(rhs.m_hasFixedLength)
    , m_fixedLength(rhs.m_fixedLength)
{
}

QPointF RulerAssistant::project(const QPointF& pt) const
{
    Q_ASSERT(isAssistantComplete());
    QPointF pt1 = *handles()[0];
    QPointF pt2 = *handles()[1];
    
    QPointF a = pt - pt1;
    QPointF u = pt2 - pt1;
    
    qreal u_norm = sqrt(u.x() * u.x() + u.y() * u.y());
    
    if(u_norm == 0) return pt;
    
    u /= u_norm;
    
    double t = a.x() * u.x() + a.y() * u.y();
    
    if(t < 0.0) return pt1;
    if(t > u_norm) return pt2;
    
    return t * u + pt1;
}

QPointF RulerAssistant::adjustPosition(const QPointF& pt, const QPointF& /*strokeBegin*/, const bool /*snapToAny*/)
{
    return project(pt);
}

void RulerAssistant::drawSubdivisions(QPainter& gc, const KisCoordinatesConverter *converter) {
    if (subdivisions() == 0) {
        return;
    }
  
    // Get handle positions
    QTransform document2widget = converter->documentToWidgetTransform();
  
    QPointF p1 = document2widget.map(*handles()[0]);
    QPointF p2 = document2widget.map(*handles()[1]);
  
    const qreal scale = 32.0 / 2;
    const qreal minorScale = scale / 2;
    const QRectF clipping = QRectF(gc.viewport()).adjusted(-scale, -scale, scale, scale);
    // If the lines would end up closer to each other than this threshold (in
    // screen coordinates), they are not rendered, as they wouldn't be
    // distinguishable anymore.
    const qreal threshold = 3.0;
    
    // Calculate line direction and normal vector
    QPointF delta = p2 - p1;
    qreal length = sqrt(KisPaintingAssistant::norm2(delta));
    qreal stepsize = length / subdivisions();
    
    // Only draw if lines are far enough apart
    if (stepsize >= threshold) {
        QPointF normal = QPointF(delta.y(), -delta.x());
        normal /= length;
  
        QPainterPath path;
        
        // Draw the major subdivisions
        for (int ii = 0; ii <= subdivisions(); ++ii) {
          
            QPointF pos = p1 + delta * ((qreal)ii / subdivisions());
            
            if (clipping.contains(pos)) {
                path.moveTo(pos - normal * scale);
                path.lineTo(pos + normal * scale);
            }
            
            // Draw minor subdivisions, if they exist (implicit check due to
            // the loop bounds)
            // Skip for the last iteration of the outer loop, which would
            // already be beyond the ruler's length
            // Also skip if major subdivisions are too close already
            if (ii == subdivisions() || stepsize / minorSubdivisions() < threshold)
                continue;
            // Draw minor marks in between the major ones
            for (int jj = 1; jj < minorSubdivisions(); ++jj) {
              
                QPointF mpos = pos + delta * ((qreal)jj / (subdivisions() * minorSubdivisions()));
    
                if (clipping.contains(mpos)) {
                    path.moveTo(mpos - normal * minorScale);
                    path.lineTo(mpos + normal * minorScale);
                }
            }
        }
  
        gc.save();
        gc.resetTransform();
        drawPath(gc, path);
        gc.restore();
    }
}

void RulerAssistant::drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool cached, KisCanvas2* canvas, bool assistantVisible, bool previewVisible)
{
    // Draw the subdivisions
    // When the number of subdivisions (or minor subdivisions) is set to
    // 0, the respective feature is turned off and won't be rendered.
    if (assistantVisible && isAssistantComplete() && subdivisions() > 0) {
        drawSubdivisions(gc, converter);
    }
    
    // Draw the ruler itself via drawCache
    KisPaintingAssistant::drawAssistant(gc, updateRect, converter, cached, canvas, assistantVisible, previewVisible);
}

void RulerAssistant::drawCache(QPainter& gc, const KisCoordinatesConverter *converter, bool assistantVisible)
{
    if (!assistantVisible || !isAssistantComplete()){
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

QPointF RulerAssistant::getEditorPosition() const
{
    return (*handles()[0] + *handles()[1]) * 0.5;
}

bool RulerAssistant::isAssistantComplete() const
{
    return handles().size() >= 2;
}

int RulerAssistant::subdivisions() const {
    return m_subdivisions;
}

void RulerAssistant::setSubdivisions(int subdivisions) {
    if (subdivisions < 0) subdivisions = 0;
    m_subdivisions = subdivisions;
}

int RulerAssistant::minorSubdivisions() const {
    return m_minorSubdivisions;
}

void RulerAssistant::setMinorSubdivisions(int subdivisions) {
    if (subdivisions < 0) subdivisions = 0;
    m_minorSubdivisions = subdivisions;
}

bool RulerAssistant::hasFixedLength() const {
    return m_hasFixedLength;
}

void RulerAssistant::enableFixedLength(bool enabled) {
    m_hasFixedLength = enabled;
}

qreal RulerAssistant::fixedLength() const {
    return m_fixedLength;
}

void RulerAssistant::setFixedLength(qreal length) {
    if (length < 0.0) length = 0.0;
    m_fixedLength = length;
}

void RulerAssistant::saveCustomXml(QXmlStreamWriter *xml) {
    if (xml) {
        xml->writeStartElement("subdivisions");
        xml->writeAttribute("value", KisDomUtils::toString(subdivisions()));
        xml->writeEndElement();
        xml->writeStartElement("minorSubdivisions");
        xml->writeAttribute("value", KisDomUtils::toString(minorSubdivisions()));
        xml->writeEndElement();
        xml->writeStartElement("fixedLength");
        xml->writeAttribute("value", KisDomUtils::toString(fixedLength()));
        xml->writeEndElement();
        xml->writeStartElement("fixedLengthEnabled");
        xml->writeAttribute("value", KisDomUtils::toString((int)hasFixedLength()));
        xml->writeEndElement();
    }
}

bool RulerAssistant::loadCustomXml(QXmlStreamReader *xml) {
    if (xml) {
        if (xml->name() == "subdivisions") {
            setSubdivisions(KisDomUtils::toInt(xml->attributes().value("value").toString()));
        }
        else if (xml->name() == "minorSubdivisions") {
            setMinorSubdivisions(KisDomUtils::toInt(xml->attributes().value("value").toString()));
        }
        else if (xml->name() == "fixedLength") {
            setFixedLength(KisDomUtils::toDouble(xml->attributes().value("value").toString()));
        }
        else if (xml->name() == "fixedLengthEnabled") {
            enableFixedLength(0 != KisDomUtils::toInt(xml->attributes().value("value").toString()));
        }
    }
    return true;
}



RulerAssistantFactory::RulerAssistantFactory() = default;

RulerAssistantFactory::~RulerAssistantFactory() = default;

QString RulerAssistantFactory::id() const
{
    return "ruler";
}

QString RulerAssistantFactory::name() const
{
    return i18n("Ruler");
}

KisPaintingAssistant* RulerAssistantFactory::createPaintingAssistant() const
{
    return new RulerAssistant;
}
