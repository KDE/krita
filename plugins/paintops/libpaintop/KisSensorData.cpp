/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisSensorData.h"

#include <KisDynamicSensorIds.h>

#include <QDomDocument>
#include <QDomElement>

KisSensorData::KisSensorData(const KoID &sensorId)
    : id(sensorId),
      curve(DEFAULT_CURVE_STRING)
{
}

KisSensorData::~KisSensorData()
{
}

void KisSensorData::setBaseCurveRange(const QRectF &rect)
{
    Q_UNUSED(rect);
    KIS_SAFE_ASSERT_RECOVER_NOOP(0 && "setBaseCurveRange is not implemented for standard Krita sensors");
}

QRectF KisSensorData::baseCurveRange() const
{
    return QRectF(0.0,0.0,1.0,1.0);
}

void KisSensorData::write(QDomDocument& doc, QDomElement &e) const
{
    e.setAttribute("id", id.id());
    if (curve != DEFAULT_CURVE_STRING) {
        QDomElement curve_elt = doc.createElement("curve");
        QDomText text = doc.createTextNode(curve);
        curve_elt.appendChild(text);
        e.appendChild(curve_elt);
    }
}

void KisSensorData::read(const QDomElement& e)
{
    KIS_ASSERT(e.attribute("id", "") == id.id());
    QDomElement curve_elt = e.firstChildElement("curve");
    if (!curve_elt.isNull()) {
        curve = curve_elt.text();
    } else {
        curve = DEFAULT_CURVE_STRING;
    }
}

void KisSensorData::reset()
{
    *this = KisSensorData(id);
}

KisSensorWithLengthData::KisSensorWithLengthData(const KoID &sensorId, const QLatin1String &lengthTag)
    : KisSensorData(sensorId)
    , m_lengthTag(lengthTag.isNull() ? QLatin1Literal("length") : lengthTag)
{
    if (sensorId == FadeId) {
        isPeriodic = false;
        length = 1000;
    } else if (sensorId == DistanceId) {
        isPeriodic = false;
        length = 30;
    } else if (sensorId == TimeId) {
        isPeriodic = false;
        length = 30;
    } else {
        qFatal("This sensor type \"%s\" has no length associated!", sensorId.id().toLatin1().data());
    }
}

void KisSensorWithLengthData::write(QDomDocument &doc, QDomElement &e) const
{
    KisSensorData::write(doc, e);
    e.setAttribute("periodic", isPeriodic);
    e.setAttribute(m_lengthTag, length);
}

void KisSensorWithLengthData::read(const QDomElement &e)
{
    reset();
    KisSensorData::read(e);

    if (e.hasAttribute("periodic")) {
        isPeriodic = e.attribute("periodic").toInt();
    }

    if (e.hasAttribute(m_lengthTag)) {
        length = e.attribute(m_lengthTag).toInt();
    }
}

void KisSensorWithLengthData::reset()
{
    *this = KisSensorWithLengthData(id, m_lengthTag);
}

KisDrawingAngleSensorData::KisDrawingAngleSensorData()
    : KisSensorData(DrawingAngleId)
{
}

void KisDrawingAngleSensorData::write(QDomDocument &doc, QDomElement &e) const
{
    KisSensorData::write(doc, e);
    e.setAttribute("fanCornersEnabled", fanCornersEnabled);
    e.setAttribute("fanCornersStep", fanCornersStep);
    e.setAttribute("angleOffset", angleOffset);
    e.setAttribute("lockedAngleMode", lockedAngleMode);
}

void KisDrawingAngleSensorData::read(const QDomElement &e)
{
    reset();
    KisSensorData::read(e);

    if (e.hasAttribute("fanCornersEnabled")) {
        fanCornersEnabled = e.attribute("fanCornersEnabled").toInt();
    }
    if (e.hasAttribute("fanCornersStep")) {
        fanCornersStep = e.attribute("fanCornersStep").toInt();
    }
    if (e.hasAttribute("angleOffset")) {
        angleOffset = e.attribute("angleOffset").toInt();
    }
    if (e.hasAttribute("lockedAngleMode")) {
        lockedAngleMode = e.attribute("lockedAngleMode").toInt();
    }
}

void KisDrawingAngleSensorData::reset()
{
    *this = KisDrawingAngleSensorData();
}
