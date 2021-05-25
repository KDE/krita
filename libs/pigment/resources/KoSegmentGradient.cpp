/*
    SPDX-FileCopyrightText: 2000 Matthias Elter <elter@kde.org>
    SPDX-FileCopyrightText: 2001 John Califf
    SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
    SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
    SPDX-FileCopyrightText: 2004, 2007 Sven Langkamp <sven.langkamp@gmail.com>
    SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>

    SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <resources/KoSegmentGradient.h>

#include <array>
#include <cfloat>
#include <cmath>

#include <QImage>
#include <QTextStream>
#include <QFile>
#include <QByteArray>
#include <QDomDocument>
#include <QDomElement>
#include <QBuffer>

#include <DebugPigment.h>
#include <KoCanvasResourcesIds.h>
#include <KoCanvasResourcesInterface.h>
#include <KoColorModelStandardIds.h>
#include <kis_dom_utils.h>
#include <kis_global.h>
#include <klocalizedstring.h>

#include "KoColor.h"
#include "KoColorSpace.h"
#include "KoColorSpaceRegistry.h"
#include "KoMixColorsOp.h"

KoGradientSegment::RGBColorInterpolationStrategy *KoGradientSegment::RGBColorInterpolationStrategy::m_instance = 0;
KoGradientSegment::HSVCWColorInterpolationStrategy *KoGradientSegment::HSVCWColorInterpolationStrategy::m_instance = 0;
KoGradientSegment::HSVCCWColorInterpolationStrategy *KoGradientSegment::HSVCCWColorInterpolationStrategy::m_instance = 0;

KoGradientSegment::LinearInterpolationStrategy *KoGradientSegment::LinearInterpolationStrategy::m_instance = 0;
KoGradientSegment::CurvedInterpolationStrategy *KoGradientSegment::CurvedInterpolationStrategy::m_instance = 0;
KoGradientSegment::SineInterpolationStrategy *KoGradientSegment::SineInterpolationStrategy::m_instance = 0;
KoGradientSegment::SphereIncreasingInterpolationStrategy *KoGradientSegment::SphereIncreasingInterpolationStrategy::m_instance = 0;
KoGradientSegment::SphereDecreasingInterpolationStrategy *KoGradientSegment::SphereDecreasingInterpolationStrategy::m_instance = 0;

KoSegmentGradient::KoSegmentGradient(const QString& file)
    : KoAbstractGradient(file)
{
}

KoSegmentGradient::~KoSegmentGradient()
{
    for (int i = 0; i < m_segments.count(); i++) {
        delete m_segments[i];
        m_segments[i] = 0;
    }
}

KoSegmentGradient::KoSegmentGradient(const KoSegmentGradient &rhs)
    : KoAbstractGradient(rhs)
{
    Q_FOREACH (KoGradientSegment *segment, rhs.m_segments) {
        pushSegment(new KoGradientSegment(*segment));
    }
}

KoResourceSP KoSegmentGradient::clone() const
{
    return KoResourceSP(new KoSegmentGradient(*this));
}

bool KoSegmentGradient::loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface)
{
    Q_UNUSED(resourcesInterface);

    QByteArray data = dev->readAll();

    QTextStream fileContent(data, QIODevice::ReadOnly);
    fileContent.setAutoDetectUnicode(true);

    QString header = fileContent.readLine();

    if (header != "GIMP Gradient") {
        return false;
    }

    QString nameDefinition = fileContent.readLine();
    QString numSegmentsText;

    if (nameDefinition.startsWith("Name: ")) {
        QString nameText = nameDefinition.right(nameDefinition.length() - 6);
        setName(nameText);

        numSegmentsText = fileContent.readLine();
    } else {
        // Older format without name.

        numSegmentsText = nameDefinition;
    }

    dbgPigment << "Loading gradient: " << name();

    int numSegments;
    bool ok;

    numSegments = numSegmentsText.toInt(&ok);

    if (!ok || numSegments < 1) {
        return false;
    }

    dbgPigment << "Number of segments = " << numSegments;

    const KoColorSpace *rgbColorSpace = KoColorSpaceRegistry::instance()->rgb16(KoColorSpaceRegistry::instance()->p709SRGBProfile());

    for (int i = 0; i < numSegments; i++) {

        QString segmentText = fileContent.readLine();
        QTextStream segmentFields(&segmentText);
        QStringList values = segmentText.split(' ');

        qreal leftOffset = values[0].toDouble();
        qreal middleOffset = values[1].toDouble();
        qreal rightOffset = values[2].toDouble();

        qreal leftRed = values[3].toDouble();
        qreal leftGreen = values[4].toDouble();
        qreal leftBlue = values[5].toDouble();
        qreal leftAlpha = values[6].toDouble();

        qreal rightRed = values[7].toDouble();
        qreal rightGreen = values[8].toDouble();
        qreal rightBlue = values[9].toDouble();
        qreal rightAlpha = values[10].toDouble();

        int interpolationType = values[11].toInt();
        int colorInterpolationType = values[12].toInt();

        KoGradientSegmentEndpointType startType, endType;
        if (values.count() >= 15) { //file supports FG/BG colors
            startType = static_cast<KoGradientSegmentEndpointType>(values[13].toInt());
            endType = static_cast<KoGradientSegmentEndpointType>(values[14].toInt());
        }
        else {
            startType = endType = COLOR_ENDPOINT;
        }
        std::array<quint16, 4> data;
        data[2] = static_cast<quint16>(leftRed * quint16_MAX + 0.5);
        data[1] = static_cast<quint16>(leftGreen * quint16_MAX + 0.5);
        data[0] = static_cast<quint16>(leftBlue * quint16_MAX + 0.5);
        data[3] = static_cast<quint16>(leftAlpha * quint16_MAX + 0.5);

        KoColor leftColor(reinterpret_cast<quint8 *>(data.data()), rgbColorSpace);

        data[2] = static_cast<quint16>(rightRed * quint16_MAX + 0.5);
        data[1] = static_cast<quint16>(rightGreen * quint16_MAX + 0.5);
        data[0] = static_cast<quint16>(rightBlue * quint16_MAX + 0.5);
        data[3] = static_cast<quint16>(rightAlpha * quint16_MAX + 0.5);

        KoColor rightColor(reinterpret_cast<quint8 *>(data.data()), rgbColorSpace);
        KoGradientSegmentEndpoint left(leftOffset, leftColor, startType);
        KoGradientSegmentEndpoint right(rightOffset, rightColor, endType);

        KoGradientSegment *segment = new KoGradientSegment(interpolationType, colorInterpolationType, left, right, middleOffset);
        Q_CHECK_PTR(segment);

        if (!segment -> isValid()) {
            delete segment;
            return false;
        }

        m_segments.push_back(segment);
    }

    if (!m_segments.isEmpty()) {
        updatePreview();
        setValid(true);
        return true;
    } else {
        return false;
    }

}

bool KoSegmentGradient::saveToDevice(QIODevice *dev) const
{
    QTextStream fileContent(dev);
    fileContent << "GIMP Gradient\n";
    fileContent << "Name: " << name() << "\n";
    fileContent << m_segments.count() << "\n";

    Q_FOREACH (KoGradientSegment* segment, m_segments) {
        fileContent << QString::number(segment->startOffset(), 'f') << " " << QString::number(segment->middleOffset(), 'f') << " "
                    << QString::number(segment->endOffset(), 'f') << " ";

        QColor startColor = segment->startColor().toQColor();
        QColor endColor = segment->endColor().toQColor();
        fileContent << QString::number(startColor.redF(), 'f') << " " << QString::number(startColor.greenF(), 'f') << " "
                    << QString::number(startColor.blueF(), 'f') << " " << QString::number(startColor.alphaF(), 'f') << " ";
        fileContent << QString::number(endColor.redF(), 'f') << " " << QString::number(endColor.greenF(), 'f') << " "
                    << QString::number(endColor.blueF(), 'f') << " " << QString::number(endColor.alphaF(), 'f') << " ";

        fileContent << (int)segment->interpolation() << " " << (int)segment->colorInterpolation() << " ";

        fileContent << (int)segment->startType() << " " << (int)segment->endType() << "\n";

    }

    return true;
}

KoGradientSegment *KoSegmentGradient::segmentAt(qreal t) const
{
    if (t < 0.0) return 0;
    if (t > 1.0) return 0;
    if (m_segments.isEmpty()) return 0;

    for (QList<KoGradientSegment *>::const_iterator it = m_segments.begin(); it != m_segments.end(); ++it) {
        if (t > (*it)->startOffset() - DBL_EPSILON && t < (*it)->endOffset() + DBL_EPSILON) {
            return *it;
        }
    }

    return 0;
}

void KoSegmentGradient::colorAt(KoColor& dst, qreal t) const
{
    const KoGradientSegment *segment = segmentAt(t);
    if (segment) {
        segment->colorAt(dst, t);
    }
}

QGradient* KoSegmentGradient::toQGradient() const
{
    QGradient* gradient = new QLinearGradient();

    QColor color;
    Q_FOREACH (KoGradientSegment* segment, m_segments) {
        segment->startColor().toQColor(&color);
        gradient->setColorAt(segment->startOffset() , color);
        segment->endColor().toQColor(&color);
        gradient->setColorAt(segment->endOffset() , color);
    }
    return gradient;
}

QString KoSegmentGradient::defaultFileExtension() const
{
    return QString(".ggr");
}

void KoSegmentGradient::toXML(QDomDocument &doc, QDomElement &gradientElt) const
{
    gradientElt.setAttribute("type", "segment");
    Q_FOREACH(KoGradientSegment *segment, this->segments()) {
        QDomElement segmentElt = doc.createElement("segment");
        QDomElement start = doc.createElement("start");
        QDomElement end = doc.createElement("end");
        segmentElt.setAttribute("start-offset", KisDomUtils::toString(segment->startOffset()));
        const KoColor startColor = segment->startColor();
        segmentElt.setAttribute("start-bitdepth", startColor.colorSpace()->colorDepthId().id());
        segmentElt.setAttribute("start-alpha", KisDomUtils::toString(startColor.opacityF()));
        segmentElt.setAttribute("start-type", KisDomUtils::toString(segment->startType()));
        startColor.toXML(doc, start);
        segmentElt.setAttribute("middle-offset", KisDomUtils::toString(segment->middleOffset()));
        segmentElt.setAttribute("end-offset", KisDomUtils::toString(segment->endOffset()));
        const KoColor endColor = segment->endColor();
        segmentElt.setAttribute("end-bitdepth", endColor.colorSpace()->colorDepthId().id());
        segmentElt.setAttribute("end-alpha", KisDomUtils::toString(endColor.opacityF()));
        segmentElt.setAttribute("end-type", KisDomUtils::toString(segment->endType()));
        endColor.toXML(doc, end);
        segmentElt.setAttribute("interpolation", KisDomUtils::toString(segment->interpolation()));
        segmentElt.setAttribute("color-interpolation", KisDomUtils::toString(segment->colorInterpolation()));
        segmentElt.appendChild(start);
        segmentElt.appendChild(end);
        gradientElt.appendChild(segmentElt);
    }
}

KoSegmentGradient KoSegmentGradient::fromXML(const QDomElement &elt)
{
    KoSegmentGradient gradient;
    QDomElement segmentElt = elt.firstChildElement("segment");
    while (!segmentElt.isNull()) {
        int interpolation = KisDomUtils::toInt(segmentElt.attribute("interpolation", "0.0"));
        int colorInterpolation = KisDomUtils::toInt(segmentElt.attribute("color-interpolation", "0.0"));
        double startOffset = KisDomUtils::toDouble(segmentElt.attribute("start-offset", "0.0"));
        qreal middleOffset = KisDomUtils::toDouble(segmentElt.attribute("middle-offset", "0.0"));
        qreal endOffset = KisDomUtils::toDouble(segmentElt.attribute("end-offset", "0.0"));
        QDomElement start = segmentElt.firstChildElement("start");
        QString startBitdepth = segmentElt.attribute("start-bitdepth", Integer8BitsColorDepthID.id());
        QColor left = KoColor::fromXML(start.firstChildElement(), startBitdepth).toQColor();
        left.setAlphaF(KisDomUtils::toDouble(segmentElt.attribute("start-alpha", "1.0")));
        QString endBitdepth = segmentElt.attribute("end-bitdepth", Integer8BitsColorDepthID.id());
        QDomElement end = segmentElt.firstChildElement("end");
        QColor right = KoColor::fromXML(end.firstChildElement(), endBitdepth).toQColor();
        right.setAlphaF(KisDomUtils::toDouble(segmentElt.attribute("end-alpha", "1.0")));
        KoGradientSegmentEndpointType leftType = static_cast<KoGradientSegmentEndpointType>(KisDomUtils::toInt(segmentElt.attribute("start-type", "0")));
        KoGradientSegmentEndpointType rightType = static_cast<KoGradientSegmentEndpointType>(KisDomUtils::toInt(segmentElt.attribute("end-type", "0")));
        gradient.createSegment(interpolation, colorInterpolation, startOffset, endOffset, middleOffset, left, right, leftType, rightType);
        segmentElt = segmentElt.nextSiblingElement("segment");
    }
    return gradient;
}

KoGradientSegment::KoGradientSegment(int interpolationType, int colorInterpolationType, KoGradientSegmentEndpoint start, KoGradientSegmentEndpoint end, qreal middleOffset)
    : m_start(start), m_end(end)
{
    m_interpolator = 0;

    switch (interpolationType) {
    case INTERP_LINEAR:
        m_interpolator = LinearInterpolationStrategy::instance();
        break;
    case INTERP_CURVED:
        m_interpolator = CurvedInterpolationStrategy::instance();
        break;
    case INTERP_SINE:
        m_interpolator = SineInterpolationStrategy::instance();
        break;
    case INTERP_SPHERE_INCREASING:
        m_interpolator = SphereIncreasingInterpolationStrategy::instance();
        break;
    case INTERP_SPHERE_DECREASING:
        m_interpolator = SphereDecreasingInterpolationStrategy::instance();
        break;
    }

    m_colorInterpolator = 0;

    switch (colorInterpolationType) {
    case COLOR_INTERP_RGB:
        m_colorInterpolator = RGBColorInterpolationStrategy::instance();
        break;
    case COLOR_INTERP_HSV_CCW:
        m_colorInterpolator = HSVCCWColorInterpolationStrategy::instance();
        break;
    case COLOR_INTERP_HSV_CW:
        m_colorInterpolator = HSVCWColorInterpolationStrategy::instance();
        break;
    }



    if (m_start.offset < DBL_EPSILON) {
        m_start.offset = 0;
    } else if (m_start.offset > 1 - DBL_EPSILON) {
        m_start.offset = 1;
    }

    if (middleOffset < m_start.offset + DBL_EPSILON) {
        m_middleOffset = m_start.offset;
    } else if (middleOffset > 1 - DBL_EPSILON) {
        m_middleOffset = 1;
    } else {
        m_middleOffset = middleOffset;
    }

    if (m_end.offset < m_middleOffset + DBL_EPSILON) {
        m_end.offset = m_middleOffset;
    } else if (m_end.offset > 1 - DBL_EPSILON) {
        m_end.offset = 1;
    }

    m_length = m_end.offset - m_start.offset;

    if (m_length < DBL_EPSILON) {
        m_middleT = 0.5;
    } else {
        m_middleT = (m_middleOffset - m_start.offset) / m_length;
    }

    m_hasVariableColors = m_start.type != COLOR_ENDPOINT || m_end.type != COLOR_ENDPOINT;

}

const KoColor& KoGradientSegment::startColor() const
{
    return m_start.color;
}

const KoColor& KoGradientSegment::endColor() const
{
    return m_end.color;
}

qreal KoGradientSegment::startOffset() const
{
    return m_start.offset;
}

qreal KoGradientSegment::middleOffset() const
{
    return m_middleOffset;
}

qreal KoGradientSegment::endOffset() const
{
    return m_end.offset;
}

KoGradientSegmentEndpointType KoGradientSegment::startType() const
{
    return m_start.type;
}

KoGradientSegmentEndpointType KoGradientSegment::endType() const
{
    return m_end.type;
}

void KoGradientSegment::setStartType(KoGradientSegmentEndpointType type) {
    m_start.type = type;
    if (type != COLOR_ENDPOINT) {
        m_hasVariableColors = true;
    }
    else if (m_end.type == COLOR_ENDPOINT) {
        m_hasVariableColors = false;
    }
}

void KoGradientSegment::setEndType(KoGradientSegmentEndpointType type) {
    m_end.type = type;
    if (type != COLOR_ENDPOINT) {
        m_hasVariableColors = true;
    }
    else if (m_start.type == COLOR_ENDPOINT) {
        m_hasVariableColors = false;
    }
}

void KoGradientSegment::setStartOffset(qreal t)
{
    m_start.offset = t;
    m_length = m_end.offset - m_start.offset;

    if (m_length < DBL_EPSILON) {
        m_middleT = 0.5;
    } else {
        m_middleT = (m_middleOffset - m_start.offset) / m_length;
    }
}
void KoGradientSegment::setMiddleOffset(qreal t)
{
    m_middleOffset = t;

    if (m_length < DBL_EPSILON) {
        m_middleT = 0.5;
    } else {
        m_middleT = (m_middleOffset - m_start.offset) / m_length;
    }
}

void KoGradientSegment::setEndOffset(qreal t)
{
    m_end.offset = t;
    m_length = m_end.offset - m_start.offset;

    if (m_length < DBL_EPSILON) {
        m_middleT = 0.5;
    } else {
        m_middleT = (m_middleOffset - m_start.offset) / m_length;
    }
}

void KoGradientSegment::setVariableColors(const KoColor& foreground, const KoColor& background) {
    switch (m_start.type) {
    case COLOR_ENDPOINT:
        break;
    case FOREGROUND_ENDPOINT:
        m_start.color = foreground;
        break;
    case FOREGROUND_TRANSPARENT_ENDPOINT: //TODO: add Transparent options to gradient editor...
        m_start.color = foreground;
        m_start.color.setOpacity(quint8(0));
        break;
    case BACKGROUND_ENDPOINT:
        m_start.color = background;
        break;
    case BACKGROUND_TRANSPARENT_ENDPOINT:
        m_start.color = background;
        m_start.color.setOpacity(quint8(0));
        break;
    }

    switch (m_end.type) {
    case COLOR_ENDPOINT:
        break;
    case FOREGROUND_ENDPOINT:
        m_end.color = foreground;
        break;
    case FOREGROUND_TRANSPARENT_ENDPOINT:
        m_end.color = foreground;
        m_end.color.setOpacity(quint8(0));
        break;
    case BACKGROUND_ENDPOINT:
        m_end.color = background;
        break;
    case BACKGROUND_TRANSPARENT_ENDPOINT:
        m_end.color = background;
        m_end.color.setOpacity(quint8(0));
        break;
    }
}

bool KoGradientSegment::hasVariableColors() {
    return m_hasVariableColors;
}

int KoGradientSegment::interpolation() const
{
    return m_interpolator->type();
}

void KoGradientSegment::setInterpolation(int interpolationType)
{
    switch (interpolationType) {
    case INTERP_LINEAR:
        m_interpolator = LinearInterpolationStrategy::instance();
        break;
    case INTERP_CURVED:
        m_interpolator = CurvedInterpolationStrategy::instance();
        break;
    case INTERP_SINE:
        m_interpolator = SineInterpolationStrategy::instance();
        break;
    case INTERP_SPHERE_INCREASING:
        m_interpolator = SphereIncreasingInterpolationStrategy::instance();
        break;
    case INTERP_SPHERE_DECREASING:
        m_interpolator = SphereDecreasingInterpolationStrategy::instance();
        break;
    }
}

int KoGradientSegment::colorInterpolation() const
{
    return m_colorInterpolator->type();
}

void KoGradientSegment::setColorInterpolation(int colorInterpolationType)
{
    switch (colorInterpolationType) {
    case COLOR_INTERP_RGB:
        m_colorInterpolator = RGBColorInterpolationStrategy::instance();
        break;
    case COLOR_INTERP_HSV_CCW:
        m_colorInterpolator = HSVCCWColorInterpolationStrategy::instance();
        break;
    case COLOR_INTERP_HSV_CW:
        m_colorInterpolator = HSVCWColorInterpolationStrategy::instance();
        break;
    }
}

void KoGradientSegment::colorAt(KoColor& dst, qreal t) const
{
    Q_ASSERT(t > m_start.offset - DBL_EPSILON && t < m_end.offset + DBL_EPSILON);

    qreal segmentT;

    if (m_length < DBL_EPSILON) {
        segmentT = 0.5;
    } else {
        segmentT = (t - m_start.offset) / m_length;
    }

    qreal colorT = m_interpolator->valueAt(segmentT, m_middleT);

    m_colorInterpolator->colorAt(dst, colorT, m_start.color, m_end.color);

}

void KoGradientSegment::mirrorSegment()
{
    KoColor tmpColor = startColor();
    setStartColor(endColor());
    setEndColor(tmpColor);
    KoGradientSegmentEndpointType tmpType = startType();
    setStartType(endType());
    setEndType(tmpType);

    setMiddleOffset(endOffset() - (middleOffset() - startOffset()));

    if (interpolation() == INTERP_SPHERE_INCREASING) {
        setInterpolation(INTERP_SPHERE_DECREASING);
    }
    else if (interpolation() == INTERP_SPHERE_DECREASING) {
        setInterpolation(INTERP_SPHERE_INCREASING);
    }
    if (colorInterpolation() == COLOR_INTERP_HSV_CW) {
        setColorInterpolation(COLOR_INTERP_HSV_CCW);
    }
    else if (colorInterpolation() == COLOR_INTERP_HSV_CCW) {
        setColorInterpolation(COLOR_INTERP_HSV_CW);
    }
}

bool KoGradientSegment::isValid() const
{
    if (m_interpolator == 0 || m_colorInterpolator == 0)
        return false;
    return true;
}

KoGradientSegment::RGBColorInterpolationStrategy::RGBColorInterpolationStrategy()
    : m_colorSpace(KoColorSpaceRegistry::instance()->rgb16(KoColorSpaceRegistry::instance()->p709SRGBProfile()))
{
}

KoGradientSegment::RGBColorInterpolationStrategy *KoGradientSegment::RGBColorInterpolationStrategy::instance()
{
    if (m_instance == 0) {
        m_instance = new RGBColorInterpolationStrategy();
        Q_CHECK_PTR(m_instance);
    }

    return m_instance;
}

void KoGradientSegment::RGBColorInterpolationStrategy::colorAt(KoColor& dst, qreal t, const KoColor& _start, const KoColor& _end) const
{
    const KoColorSpace *mixSpace = dst.colorSpace();

    KoColor startDummy(_start, mixSpace);
    KoColor endDummy(_end, mixSpace);

    const std::array<quint8*, 2> colors = {{startDummy.data(), endDummy.data()}};

    std::array<qint16, 2> colorWeights{};
    colorWeights[0] = std::lround((1.0 - t) * qint16_MAX);
    colorWeights[1] = qint16_MAX - colorWeights[0];

    mixSpace->mixColorsOp()->mixColors(colors.data(), colorWeights.data(), 2, dst.data(), qint16_MAX);
}

KoGradientSegment::HSVCWColorInterpolationStrategy::HSVCWColorInterpolationStrategy()
    : m_colorSpace(KoColorSpaceRegistry::instance()->rgb16(KoColorSpaceRegistry::instance()->p709SRGBProfile()))
{
}

KoGradientSegment::HSVCWColorInterpolationStrategy *KoGradientSegment::HSVCWColorInterpolationStrategy::instance()
{
    if (m_instance == 0) {
        m_instance = new HSVCWColorInterpolationStrategy();
        Q_CHECK_PTR(m_instance);
    }

    return m_instance;
}

void KoGradientSegment::HSVCWColorInterpolationStrategy::colorAt(KoColor& dst, qreal t, const KoColor& start, const KoColor& end) const
{
    QColor sc;
    QColor ec;

    start.toQColor(&sc);
    end.toQColor(&ec);

    int s = static_cast<int>(sc.saturation() + t * (ec.saturation() - sc.saturation()) + 0.5);
    int v = static_cast<int>(sc.value() + t * (ec.value() - sc.value()) + 0.5);
    int h;

    if (ec.hue() < sc.hue()) {
        h = static_cast<int>(ec.hue() + (1 - t) * (sc.hue() - ec.hue()) + 0.5);
    } else {
        h = static_cast<int>(ec.hue() + (1 - t) * (360 - ec.hue() + sc.hue()) + 0.5);

        if (h > 359) {
            h -= 360;
        }
    }

    qreal opacity{sc.alphaF() + t * (ec.alphaF() - sc.alphaF())};

    QColor result;
    result.setHsv(h, s, v);
    result.setAlphaF(opacity);
    dst.fromQColor(result);
}

KoGradientSegment::HSVCCWColorInterpolationStrategy::HSVCCWColorInterpolationStrategy()
    : m_colorSpace(KoColorSpaceRegistry::instance()->rgb16(KoColorSpaceRegistry::instance()->p709SRGBProfile()))
{
}


KoGradientSegment::HSVCCWColorInterpolationStrategy *KoGradientSegment::HSVCCWColorInterpolationStrategy::instance()
{
    if (m_instance == 0) {
        m_instance = new HSVCCWColorInterpolationStrategy();
        Q_CHECK_PTR(m_instance);
    }

    return m_instance;
}

void KoGradientSegment::HSVCCWColorInterpolationStrategy::colorAt(KoColor& dst, qreal t, const KoColor& start, const KoColor& end) const
{
    QColor sc;
    QColor se;

    start.toQColor(&sc);
    end.toQColor(&se);

    int s = static_cast<int>(sc.saturation() + t * (se.saturation() - sc.saturation()) + 0.5);
    int v = static_cast<int>(sc.value() + t * (se.value() - sc.value()) + 0.5);
    int h;

    if (sc.hue() < se.hue()) {
        h = static_cast<int>(sc.hue() + t * (se.hue() - sc.hue()) + 0.5);
    } else {
        h = static_cast<int>(sc.hue() + t * (360 - sc.hue() + se.hue()) + 0.5);

        if (h > 359) {
            h -= 360;
        }
    }

    qreal opacity = sc.alphaF() + t * (se.alphaF() - sc.alphaF());

    QColor result;
    result.setHsv(h, s, v);
    result.setAlphaF(opacity);
    dst.fromQColor(result);
}

KoGradientSegment::LinearInterpolationStrategy *KoGradientSegment::LinearInterpolationStrategy::instance()
{
    if (m_instance == 0) {
        m_instance = new LinearInterpolationStrategy();
        Q_CHECK_PTR(m_instance);
    }

    return m_instance;
}

qreal KoGradientSegment::LinearInterpolationStrategy::calcValueAt(qreal t, qreal middle)
{
    Q_ASSERT(t > -DBL_EPSILON && t < 1 + DBL_EPSILON);
    Q_ASSERT(middle > -DBL_EPSILON && middle < 1 + DBL_EPSILON);

    qreal value = 0;

    if (t <= middle) {
        if (middle < DBL_EPSILON) {
            value = 0;
        } else {
            value = (t / middle) * 0.5;
        }
    } else {
        if (middle > 1 - DBL_EPSILON) {
            value = 1;
        } else {
            value = ((t - middle) / (1 - middle)) * 0.5 + 0.5;
        }
    }

    return value;
}

qreal KoGradientSegment::LinearInterpolationStrategy::valueAt(qreal t, qreal middle) const
{
    return calcValueAt(t, middle);
}

KoGradientSegment::CurvedInterpolationStrategy::CurvedInterpolationStrategy()
{
    m_logHalf = log(0.5);
}

KoGradientSegment::CurvedInterpolationStrategy *KoGradientSegment::CurvedInterpolationStrategy::instance()
{
    if (m_instance == 0) {
        m_instance = new CurvedInterpolationStrategy();
        Q_CHECK_PTR(m_instance);
    }

    return m_instance;
}

qreal KoGradientSegment::CurvedInterpolationStrategy::valueAt(qreal t, qreal middle) const
{
    Q_ASSERT(t > -DBL_EPSILON && t < 1 + DBL_EPSILON);
    Q_ASSERT(middle > -DBL_EPSILON && middle < 1 + DBL_EPSILON);

    qreal value = 0;

    if (middle < DBL_EPSILON) {
        middle = DBL_EPSILON;
    }

    value = pow(t, m_logHalf / log(middle));

    return value;
}

KoGradientSegment::SineInterpolationStrategy *KoGradientSegment::SineInterpolationStrategy::instance()
{
    if (m_instance == 0) {
        m_instance = new SineInterpolationStrategy();
        Q_CHECK_PTR(m_instance);
    }

    return m_instance;
}

qreal KoGradientSegment::SineInterpolationStrategy::valueAt(qreal t, qreal middle) const
{
    qreal lt = LinearInterpolationStrategy::calcValueAt(t, middle);
    qreal value = (sin(-M_PI_2 + M_PI * lt) + 1.0) / 2.0;

    return value;
}

KoGradientSegment::SphereIncreasingInterpolationStrategy *KoGradientSegment::SphereIncreasingInterpolationStrategy::instance()
{
    if (m_instance == 0) {
        m_instance = new SphereIncreasingInterpolationStrategy();
        Q_CHECK_PTR(m_instance);
    }

    return m_instance;
}

qreal KoGradientSegment::SphereIncreasingInterpolationStrategy::valueAt(qreal t, qreal middle) const
{
    qreal lt = LinearInterpolationStrategy::calcValueAt(t, middle) - 1;
    qreal value = sqrt(1 - lt * lt);

    return value;
}

KoGradientSegment::SphereDecreasingInterpolationStrategy *KoGradientSegment::SphereDecreasingInterpolationStrategy::instance()
{
    if (m_instance == 0) {
        m_instance = new SphereDecreasingInterpolationStrategy();
        Q_CHECK_PTR(m_instance);
    }

    return m_instance;
}

qreal KoGradientSegment::SphereDecreasingInterpolationStrategy::valueAt(qreal t, qreal middle) const
{
    qreal lt = LinearInterpolationStrategy::calcValueAt(t, middle);
    qreal value = 1 - sqrt(1 - lt * lt);

    return value;
}

void KoSegmentGradient::createSegment(int interpolation, int colorInterpolation, double startOffset, double endOffset, double middleOffset, const QColor & leftColor, const QColor & rightColor,
                                      KoGradientSegmentEndpointType leftType, KoGradientSegmentEndpointType rightType)
{
    KoGradientSegmentEndpoint left(startOffset, KoColor(leftColor, colorSpace()), leftType);
    KoGradientSegmentEndpoint right(endOffset, KoColor(rightColor, colorSpace()), rightType);
    pushSegment(new KoGradientSegment(interpolation, colorInterpolation, left, right, middleOffset));

}

const QList<double> KoSegmentGradient::getHandlePositions() const
{
    QList<double> handlePositions;

    handlePositions.push_back(m_segments[0]->startOffset());
    for (int i = 0; i < m_segments.count(); i++) {
        handlePositions.push_back(m_segments[i]->endOffset());
    }
    return handlePositions;
}

const QList<double> KoSegmentGradient::getMiddleHandlePositions() const
{
    QList<double> middleHandlePositions;

    for (int i = 0; i < m_segments.count(); i++) {
        middleHandlePositions.push_back(m_segments[i]->middleOffset());
    }
    return middleHandlePositions;
}

void KoSegmentGradient::moveSegmentStartOffset(KoGradientSegment* segment, double t)
{
    QList<KoGradientSegment*>::iterator it = std::find(m_segments.begin(), m_segments.end(), segment);
    if (it != m_segments.end()) {
        if (it == m_segments.begin()) {
            segment->setStartOffset(0.0);
            return;
        }
        KoGradientSegment* previousSegment = (*(it - 1));
        if (t > segment->startOffset()) {
            if (t > segment->middleOffset())
                t = segment->middleOffset();
        } else {
            if (t < previousSegment->middleOffset())
                t = previousSegment->middleOffset();
        }
        previousSegment->setEndOffset(t);
        segment->setStartOffset(t);
    }
}

void KoSegmentGradient::moveSegmentEndOffset(KoGradientSegment* segment, double t)
{
    QList<KoGradientSegment*>::iterator it = std::find(m_segments.begin(), m_segments.end(), segment);
    if (it != m_segments.end()) {
        if (it + 1 == m_segments.end()) {
            segment->setEndOffset(1.0);
            return;
        }
        KoGradientSegment* followingSegment = (*(it + 1));
        if (t < segment->endOffset()) {
            if (t < segment->middleOffset())
                t = segment->middleOffset();
        } else {
            if (t > followingSegment->middleOffset())
                t = followingSegment->middleOffset();
        }
        followingSegment->setStartOffset(t);
        segment->setEndOffset(t);
    }
}

void KoSegmentGradient::moveSegmentMiddleOffset(KoGradientSegment* segment, double t)
{
    if (segment) {
        if (t > segment->endOffset())
            segment->setMiddleOffset(segment->endOffset());
        else if (t < segment->startOffset())
            segment->setMiddleOffset(segment->startOffset());
        else
            segment->setMiddleOffset(t);
    }
}

void KoSegmentGradient::splitSegment(KoGradientSegment* segment)
{
    Q_ASSERT(segment != 0);
    QList<KoGradientSegment*>::iterator it = std::find(m_segments.begin(), m_segments.end(), segment);
    if (it != m_segments.end()) {
        KoColor midleoffsetColor(segment->endColor().colorSpace());
        segment->colorAt(midleoffsetColor, segment->middleOffset());
        KoGradientSegmentEndpoint left(segment->startOffset(), segment->startColor(), segment->startType());
        KoGradientSegmentEndpoint right(segment->middleOffset(), midleoffsetColor, COLOR_ENDPOINT);
        KoGradientSegment* newSegment = new KoGradientSegment(
                    segment->interpolation(), segment->colorInterpolation(),
                    left, right,
                    (segment->middleOffset() - segment->startOffset()) / 2 + segment->startOffset());
        m_segments.insert(it, newSegment);
        segment->setStartColor(midleoffsetColor);
        segment->setStartOffset(segment->middleOffset());
        segment->setMiddleOffset((segment->endOffset() - segment->startOffset()) / 2 + segment->startOffset());
    }
}

void KoSegmentGradient::duplicateSegment(KoGradientSegment* segment)
{
    Q_ASSERT(segment != 0);
    QList<KoGradientSegment*>::iterator it = std::find(m_segments.begin(), m_segments.end(), segment);
    if (it != m_segments.end()) {
        double middlePostionPercentage = (segment->middleOffset() - segment->startOffset()) / segment->length();
        double center = segment->startOffset() + segment->length() / 2;
        KoGradientSegmentEndpoint left(segment->startOffset(), segment->startColor(), segment->startType());
        KoGradientSegmentEndpoint right(center, segment->endColor(), segment->endType());
        KoGradientSegment* newSegment = new KoGradientSegment(
                    segment->interpolation(), segment->colorInterpolation(),
                    left, right,
                    segment->length() / 2 * middlePostionPercentage + segment->startOffset());
        m_segments.insert(it, newSegment);
        segment->setStartOffset(center);
        segment->setMiddleOffset(segment->length() * middlePostionPercentage  + segment->startOffset());
    }
}

void KoSegmentGradient::mirrorSegment(KoGradientSegment* segment)
{
    Q_ASSERT(segment != 0);

    segment->mirrorSegment();
}

KoGradientSegment* KoSegmentGradient::removeSegment(KoGradientSegment* segment)
{
    Q_ASSERT(segment != 0);
    if (m_segments.count() < 2)
        return 0;
    QList<KoGradientSegment*>::iterator it = std::find(m_segments.begin(), m_segments.end(), segment);
    if (it != m_segments.end()) {
        double middlePostionPercentage;
        KoGradientSegment* nextSegment;
        if (it == m_segments.begin()) {
            nextSegment = (*(it + 1));
            middlePostionPercentage = (nextSegment->middleOffset() - nextSegment->startOffset()) / nextSegment->length();
            nextSegment->setStartOffset(segment->startOffset());
            nextSegment->setMiddleOffset(middlePostionPercentage * nextSegment->length() + nextSegment->startOffset());
        } else {
            nextSegment = (*(it - 1));
            middlePostionPercentage = (nextSegment->middleOffset() - nextSegment->startOffset()) / nextSegment->length();
            nextSegment->setEndOffset(segment->endOffset());
            nextSegment->setMiddleOffset(middlePostionPercentage * nextSegment->length() + nextSegment->startOffset());
        }

        delete segment;
        m_segments.erase(it);
        return nextSegment;
    }
    return 0;
}

bool KoSegmentGradient::removeSegmentPossible() const
{
    if (m_segments.count() < 2)
        return false;
    return true;
}

const QList<KoGradientSegment *>& KoSegmentGradient::segments() const
{
    return m_segments;
}

void KoSegmentGradient::setSegments(const QList<KoGradientSegment*> &segments)
{
    for (int i = 0; i < m_segments.count(); i++) {
        delete m_segments[i];
    }
    m_segments.clear();
    KoColor color;
    for (const KoGradientSegment *segment : segments) {
        KoGradientSegment *newSegment =
            new KoGradientSegment(
                segment->interpolation(),
                segment->colorInterpolation(),
                KoGradientSegmentEndpoint(
                    segment->startOffset(),
                    segment->startColor().convertedTo(colorSpace()),
                    segment->startType()
                ),
                KoGradientSegmentEndpoint(
                    segment->endOffset(),
                    segment->endColor().convertedTo(colorSpace()),
                    segment->endType()
                ),
                segment->middleOffset()
            );
        m_segments.append(newSegment);
    }
    updatePreview();
}

QList<int> KoSegmentGradient::requiredCanvasResources() const
{
    bool hasVariableColors = false;
    for (int i = 0; i < m_segments.count(); i++) {
        if (m_segments[i]->hasVariableColors()) {
            hasVariableColors = true;
            break;
        }
    }

    QList<int> result;
    if (hasVariableColors) {
        result << KoCanvasResource::ForegroundColor << KoCanvasResource::BackgroundColor;
    }

    return result;
}

void KoSegmentGradient::bakeVariableColors(KoCanvasResourcesInterfaceSP canvasResourcesInterface)
{
    const KoColor fgColor = canvasResourcesInterface->resource(KoCanvasResource::ForegroundColor).value<KoColor>().convertedTo(colorSpace());
    const KoColor bgColor = canvasResourcesInterface->resource(KoCanvasResource::BackgroundColor).value<KoColor>().convertedTo(colorSpace());

    for (auto it = m_segments.begin(); it != m_segments.end(); ++it) {
        if ((*it)->hasVariableColors()) {
            (*it)->setVariableColors(fgColor, bgColor);
            (*it)->setStartType(COLOR_ENDPOINT);
            (*it)->setEndType(COLOR_ENDPOINT);
        }
    }
}

void KoSegmentGradient::updateVariableColors(KoCanvasResourcesInterfaceSP canvasResourcesInterface)
{
    const KoColor fgColor = canvasResourcesInterface->resource(KoCanvasResource::ForegroundColor).value<KoColor>().convertedTo(colorSpace());
    const KoColor bgColor = canvasResourcesInterface->resource(KoCanvasResource::BackgroundColor).value<KoColor>().convertedTo(colorSpace());

    for (auto it = m_segments.begin(); it != m_segments.end(); ++it) {
        if ((*it)->hasVariableColors()) {
            (*it)->setVariableColors(fgColor, bgColor);
        }
    }
}
