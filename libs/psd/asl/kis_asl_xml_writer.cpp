/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_asl_xml_writer.h"

#include <QDomDocument>
#include <QColor>
#include <QPointF>
#include <QUuid>
#include <QBuffer>

#include <resources/KoPattern.h>
#include <resources/KoSegmentGradient.h>
#include <resources/KoStopGradient.h>

#include <cfloat>

#include "kis_dom_utils.h"
#include "kis_asl_writer_utils.h"

struct KisAslXmlWriter::Private
{
    QDomDocument document;
    QDomElement currentElement;
};


KisAslXmlWriter::KisAslXmlWriter()
    : m_d(new Private)
{
    QDomElement el = m_d->document.createElement("asl");
    m_d->document.appendChild(el);
    m_d->currentElement = el;
}

KisAslXmlWriter::~KisAslXmlWriter()
{
}

QDomDocument KisAslXmlWriter::document() const
{
    if (m_d->document.documentElement() != m_d->currentElement) {
        warnKrita << "KisAslXmlWriter::document(): unbalanced enter/leave descriptor/array";
    }

    return m_d->document;
}

void KisAslXmlWriter::enterDescriptor(const QString &key, const QString &name, const QString &classId)
{
    QDomElement el = m_d->document.createElement("node");

    if (!key.isEmpty()) {
        el.setAttribute("key", key);
    }

    el.setAttribute("type", "Descriptor");
    el.setAttribute("name", name);
    el.setAttribute("classId", classId);

    m_d->currentElement.appendChild(el);
    m_d->currentElement = el;
}

void KisAslXmlWriter::leaveDescriptor()
{
    if (!m_d->currentElement.parentNode().toElement().isNull()) {
        m_d->currentElement = m_d->currentElement.parentNode().toElement();
    } else {
        warnKrita << "KisAslXmlWriter::leaveDescriptor(): unbalanced enter/leave descriptor";
    }
}

void KisAslXmlWriter::enterList(const QString &key)
{
    QDomElement el = m_d->document.createElement("node");

    if (!key.isEmpty()) {
        el.setAttribute("key", key);
    }

    el.setAttribute("type", "List");

    m_d->currentElement.appendChild(el);
    m_d->currentElement = el;
}

void KisAslXmlWriter::leaveList()
{
    if (!m_d->currentElement.parentNode().toElement().isNull()) {
        m_d->currentElement = m_d->currentElement.parentNode().toElement();
    } else {
        warnKrita << "KisAslXmlWriter::leaveList(): unbalanced enter/leave list";
    }
}

void KisAslXmlWriter::writeDouble(const QString &key, double value)
{
    QDomElement el = m_d->document.createElement("node");

    if (!key.isEmpty()) {
        el.setAttribute("key", key);
    }

    el.setAttribute("type", "Double");
    el.setAttribute("value", KisDomUtils::toString(value));

    m_d->currentElement.appendChild(el);
}

void KisAslXmlWriter::writeInteger(const QString &key, int value)
{
    QDomElement el = m_d->document.createElement("node");

    if (!key.isEmpty()) {
        el.setAttribute("key", key);
    }

    el.setAttribute("type", "Integer");
    el.setAttribute("value", KisDomUtils::toString(value));

    m_d->currentElement.appendChild(el);
}

void KisAslXmlWriter::writeEnum(const QString &key, const QString &typeId, const QString &value)
{
    QDomElement el = m_d->document.createElement("node");

    if (!key.isEmpty()) {
        el.setAttribute("key", key);
    }

    el.setAttribute("type", "Enum");
    el.setAttribute("typeId", typeId);
    el.setAttribute("value", value);

    m_d->currentElement.appendChild(el);
}

void KisAslXmlWriter::writeUnitFloat(const QString &key, const QString &unit, double value)
{
    QDomElement el = m_d->document.createElement("node");

    if (!key.isEmpty()) {
        el.setAttribute("key", key);
    }

    el.setAttribute("type", "UnitFloat");
    el.setAttribute("unit", unit);
    el.setAttribute("value", KisDomUtils::toString(value));

    m_d->currentElement.appendChild(el);
}

void KisAslXmlWriter::writeText(const QString &key, const QString &value)
{
    QDomElement el = m_d->document.createElement("node");

    if (!key.isEmpty()) {
        el.setAttribute("key", key);
    }

    el.setAttribute("type", "Text");
    el.setAttribute("value", value);

    m_d->currentElement.appendChild(el);
}

void KisAslXmlWriter::writeBoolean(const QString &key, bool value)
{
    QDomElement el = m_d->document.createElement("node");

    if (!key.isEmpty()) {
        el.setAttribute("key", key);
    }

    el.setAttribute("type", "Boolean");
    el.setAttribute("value", KisDomUtils::toString(value));

    m_d->currentElement.appendChild(el);
}

void KisAslXmlWriter::writeColor(const QString &key, const QColor &value)
{
    enterDescriptor(key, "", "RGBC");

    writeDouble("Rd  ", value.red());
    writeDouble("Grn ", value.green());
    writeDouble("Bl  ", value.blue());

    leaveDescriptor();
}

void KisAslXmlWriter::writePoint(const QString &key, const QPointF &value)
{
    enterDescriptor(key, "", "CrPt");

    writeDouble("Hrzn", value.x());
    writeDouble("Vrtc", value.y());

    leaveDescriptor();
}

void KisAslXmlWriter::writePhasePoint(const QString &key, const QPointF &value)
{
    enterDescriptor(key, "", "Pnt ");

    writeDouble("Hrzn", value.x());
    writeDouble("Vrtc", value.y());

    leaveDescriptor();
}

void KisAslXmlWriter::writeOffsetPoint(const QString &key, const QPointF &value)
{
    enterDescriptor(key, "", "Pnt ");

    writeUnitFloat("Hrzn", "#Prc", value.x());
    writeUnitFloat("Vrtc", "#Prc", value.y());

    leaveDescriptor();
}

void KisAslXmlWriter::writeCurve(const QString &key, const QString &name, const QVector<QPointF> &points)
{
    enterDescriptor(key, "", "ShpC");

    writeText("Nm  ", name);

    enterList("Crv ");

    Q_FOREACH (const QPointF &pt, points) {
        writePoint("", pt);
    }

    leaveList();
    leaveDescriptor();
}

QString KisAslXmlWriter::writePattern(const QString &key, const KoPatternSP pattern)
{
    enterDescriptor(key, "", "KisPattern");

    writeText("Nm  ", pattern->name());

    QString uuid = KisAslWriterUtils::getPatternUuidLazy(pattern);
    writeText("Idnt", uuid);

    // Write pattern data

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    pattern->savePatToDevice(&buffer);

    QDomCDATASection dataSection = m_d->document.createCDATASection(qCompress(buffer.buffer()).toBase64());

    QDomElement dataElement = m_d->document.createElement("node");
    dataElement.setAttribute("type", "KisPatternData");
    dataElement.setAttribute("key", "Data");
    dataElement.appendChild(dataSection);

    m_d->currentElement.appendChild(dataElement);

    leaveDescriptor();

    return uuid;
}

void KisAslXmlWriter::writePatternRef(const QString &key, const KoPatternSP pattern, const QString &uuid)
{
    enterDescriptor(key, "", "Ptrn");

    writeText("Nm  ", pattern->name());
    writeText("Idnt", uuid);

    leaveDescriptor();
}

void KisAslXmlWriter::writeGradientImpl(const QString &key,
                                        const QString &name,
                                        QVector<QColor> colors,
                                        QVector<qreal> transparencies,
                                        QVector<qreal> positions,
                                        QVector<QString> types,
                                        QVector<qreal> middleOffsets)
{
    enterDescriptor(key, "Gradient", "Grdn");

    writeText("Nm  ", name);
    writeEnum("GrdF", "GrdF", "CstS");
    writeDouble("Intr", 4096);

    enterList("Clrs");

    for (int i = 0; i < colors.size(); i++) {
        enterDescriptor("", "", "Clrt");

        writeColor("Clr ", colors[i]);
        writeEnum("Type", "Clry", types[i]);
        writeInteger("Lctn", positions[i] * 4096.0);
        writeInteger("Mdpn", middleOffsets[i] * 100.0);

        leaveDescriptor();
    };

    leaveList();

    enterList("Trns");

    for (int i = 0; i < colors.size(); i++) {
        enterDescriptor("", "", "TrnS");
        writeUnitFloat("Opct", "#Prc", transparencies[i] * 100.0);
        writeInteger("Lctn", positions[i] * 4096.0);
        writeInteger("Mdpn", middleOffsets[i] * 100.0);
        leaveDescriptor();
    };

    leaveList();

    leaveDescriptor();
}

QString KisAslXmlWriter::getSegmentEndpointTypeString(KoGradientSegmentEndpointType segtype) {
    switch (segtype) {
    case COLOR_ENDPOINT:
        return "UsrS";
        break;
    case FOREGROUND_ENDPOINT:
    case FOREGROUND_TRANSPARENT_ENDPOINT:
        return "FrgC";
        break;
    case BACKGROUND_ENDPOINT:
    case BACKGROUND_TRANSPARENT_ENDPOINT:
        return "BckC";
        break;
    default:
        return "UsrS";
    }
}

void KisAslXmlWriter::writeSegmentGradient(const QString &key, const KoSegmentGradient *gradient)
{
    const QList<KoGradientSegment *>&segments = gradient->segments();
    KIS_SAFE_ASSERT_RECOVER_RETURN(!segments.isEmpty());

    QVector<QColor> colors;
    QVector<qreal> transparencies;
    QVector<qreal> positions;
    QVector<QString> types;
    QVector<qreal> middleOffsets;

    Q_FOREACH (const KoGradientSegment *seg, segments) {
        const qreal start = seg->startOffset();
        const qreal end = seg->endOffset();
        const qreal mid = (end - start) > DBL_EPSILON ? (seg->middleOffset() - start) / (end - start) : 0.5;

        QColor color = seg->startColor().toQColor();
        qreal transparency = color.alphaF();
        color.setAlphaF(1.0);

        QString type = getSegmentEndpointTypeString(seg->startType());

        colors << color;
        transparencies << transparency;
        positions << start;
        types << type;
        middleOffsets << mid;
    }

    // last segment

    if (!segments.isEmpty()) {
        const KoGradientSegment *lastSeg = segments.last();

        QColor color = lastSeg->endColor().toQColor();
        qreal transparency = color.alphaF();
        color.setAlphaF(1.0);
        QString type = getSegmentEndpointTypeString(lastSeg->endType());

        colors << color;
        transparencies << transparency;
        positions << lastSeg->endOffset();
        types << type;
        middleOffsets << 0.5;
    }

    writeGradientImpl(key, gradient->name(), colors, transparencies, positions, types, middleOffsets);
}

void KisAslXmlWriter::writeStopGradient(const QString &key, const KoStopGradient *gradient)
{
    QVector<QColor> colors;
    QVector<qreal> transparencies;
    QVector<qreal> positions;
    QVector<QString> types;
    QVector<qreal> middleOffsets;

    Q_FOREACH (const KoGradientStop &stop, gradient->stops()) {
        QColor color = stop.color.toQColor();
        qreal transparency = color.alphaF();
        color.setAlphaF(1.0);

        QString type;
        switch (stop.type) {
        case COLORSTOP:
            type = "UsrS";
            break;
        case FOREGROUNDSTOP:
            type = "FrgC";
            break;
        case BACKGROUNDSTOP:
            type = "BckC";
            break;
        }

        colors << color;
        transparencies << transparency;
        positions << stop.position;
        types << type;
        middleOffsets << 0.5;
    }

    writeGradientImpl(key, gradient->name(), colors, transparencies, positions, types, middleOffsets);
}
