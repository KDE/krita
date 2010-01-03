/*
   Copyright (c) 2000 Matthias Elter <elter@kde.org>
                 2001 John Califf
                 2004 Boudewijn Rempt <boud@valdyas.org>
                 2004 Adrian Page <adrian@pagenet.plus.com>
                 2004, 2007 Sven Langkamp <sven.langkamp@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "KoSegmentGradient.h"

#include <cfloat>
#include <cmath>

#include <QImage>
#include <QTextStream>
#include <QFile>

#include "KoColorSpaceRegistry.h"
#include "KoColorSpace.h"

#include <kdebug.h>
#include <klocale.h>

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

bool KoSegmentGradient::load()
{
    return init();
}

bool KoSegmentGradient::save()
{
    return false;
}

bool KoSegmentGradient::init()
{
    QFile file(filename());

    if (!file.open(QIODevice::ReadOnly))
        return false;

    QByteArray m_data = file.readAll();
    file.close();

    QTextStream fileContent(m_data, QIODevice::ReadOnly);
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

    kDebug(30009) << "Loading gradient: " << name();

    int numSegments;
    bool ok;

    numSegments = numSegmentsText.toInt(&ok);

    if (!ok || numSegments < 1) {
        return false;
    }

    kDebug(30009) << "Number of segments = " << numSegments;

    const KoColorSpace* rgbColorSpace = KoColorSpaceRegistry::instance()->rgb8();

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

        quint8 data[4];
        data[2] = static_cast<quint8>(leftRed * 255 + 0.5);
        data[1] = static_cast<quint8>(leftGreen * 255 + 0.5);
        data[0] = static_cast<quint8>(leftBlue * 255 + 0.5);
        data[3] = static_cast<quint8>(leftAlpha * OPACITY_OPAQUE + 0.5);

        KoColor leftColor(data, rgbColorSpace);

        data[2] = static_cast<quint8>(rightRed * 255 + 0.5);
        data[1] = static_cast<quint8>(rightGreen * 255 + 0.5);
        data[0] = static_cast<quint8>(rightBlue * 255 + 0.5);
        data[3] = static_cast<quint8>(rightAlpha * OPACITY_OPAQUE + 0.5);

        KoColor rightColor(data, rgbColorSpace);

        KoGradientSegment *segment = new KoGradientSegment(interpolationType, colorInterpolationType, leftOffset, middleOffset, rightOffset, leftColor, rightColor);
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

KoGradientSegment *KoSegmentGradient::segmentAt(qreal t) const
{
    Q_ASSERT(t >= 0 || t <= 1);
    Q_ASSERT(!m_segments.empty());

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
    Q_ASSERT(segment != 0);

    if (segment) {
        segment->colorAt(dst, t);
    }
}

QGradient* KoSegmentGradient::toQGradient() const
{
    QGradient* gradient = new QLinearGradient();;

    QColor color;
    foreach(KoGradientSegment* segment, m_segments) {
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

KoGradientSegment::KoGradientSegment(int interpolationType, int colorInterpolationType, qreal startOffset, qreal middleOffset, qreal endOffset, const KoColor& startColor, const KoColor& endColor)
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

    if (startOffset < DBL_EPSILON) {
        m_startOffset = 0;
    } else
        if (startOffset > 1 - DBL_EPSILON) {
            m_startOffset = 1;
        } else {
            m_startOffset = startOffset;
        }

    if (middleOffset < m_startOffset + DBL_EPSILON) {
        m_middleOffset = m_startOffset;
    } else
        if (middleOffset > 1 - DBL_EPSILON) {
            m_middleOffset = 1;
        } else {
            m_middleOffset = middleOffset;
        }

    if (endOffset < m_middleOffset + DBL_EPSILON) {
        m_endOffset = m_middleOffset;
    } else
        if (endOffset > 1 - DBL_EPSILON) {
            m_endOffset = 1;
        } else {
            m_endOffset = endOffset;
        }

    m_length = m_endOffset - m_startOffset;

    if (m_length < DBL_EPSILON) {
        m_middleT = 0.5;
    } else {
        m_middleT = (m_middleOffset - m_startOffset) / m_length;
    }

    m_startColor = startColor;
    m_endColor = endColor;
}

const KoColor& KoGradientSegment::startColor() const
{
    return m_startColor;
}

const KoColor& KoGradientSegment::endColor() const
{
    return m_endColor;
}

qreal KoGradientSegment::startOffset() const
{
    return m_startOffset;
}

qreal KoGradientSegment::middleOffset() const
{
    return m_middleOffset;
}

qreal KoGradientSegment::endOffset() const
{
    return m_endOffset;
}

void KoGradientSegment::setStartOffset(qreal t)
{
    m_startOffset = t;
    m_length = m_endOffset - m_startOffset;

    if (m_length < DBL_EPSILON) {
        m_middleT = 0.5;
    } else {
        m_middleT = (m_middleOffset - m_startOffset) / m_length;
    }
}
void KoGradientSegment::setMiddleOffset(qreal t)
{
    m_middleOffset = t;

    if (m_length < DBL_EPSILON) {
        m_middleT = 0.5;
    } else {
        m_middleT = (m_middleOffset - m_startOffset) / m_length;
    }
}

void KoGradientSegment::setEndOffset(qreal t)
{
    m_endOffset = t;
    m_length = m_endOffset - m_startOffset;

    if (m_length < DBL_EPSILON) {
        m_middleT = 0.5;
    } else {
        m_middleT = (m_middleOffset - m_startOffset) / m_length;
    }
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
    Q_ASSERT(t > m_startOffset - DBL_EPSILON && t < m_endOffset + DBL_EPSILON);

    qreal segmentT;

    if (m_length < DBL_EPSILON) {
        segmentT = 0.5;
    } else {
        segmentT = (t - m_startOffset) / m_length;
    }

    qreal colorT = m_interpolator->valueAt(segmentT, m_middleT);

    m_colorInterpolator->colorAt(dst, colorT, m_startColor, m_endColor);

}

bool KoGradientSegment::isValid() const
{
    if (m_interpolator == 0 || m_colorInterpolator == 0)
        return false;
    return true;
}

KoGradientSegment::RGBColorInterpolationStrategy::RGBColorInterpolationStrategy()
        : m_colorSpace(KoColorSpaceRegistry::instance()->rgb8()), buffer(m_colorSpace), m_start(m_colorSpace), m_end(m_colorSpace)
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

void KoGradientSegment::RGBColorInterpolationStrategy::colorAt(KoColor& dst, qreal t, const KoColor& start, const KoColor& end) const
{
    m_start.fromKoColor(start);
    m_end.fromKoColor(end);

    const quint8 *colors[2];
    colors[0] = start.data();
    colors[1] = end.data();

    qint16 colorWeights[2];
    colorWeights[0] = static_cast<quint8>((1.0 - t) * 255 + 0.5);
    colorWeights[1] = 255 - colorWeights[0];

    m_colorSpace->mixColorsOp()->mixColors(colors, colorWeights, 2, buffer.data());

    dst.fromKoColor(buffer);
}

KoGradientSegment::HSVCWColorInterpolationStrategy::HSVCWColorInterpolationStrategy()
        : m_colorSpace(KoColorSpaceRegistry::instance()->rgb8())
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
    // XXX: added an explicit cast. Is this correct?
    quint8 opacity = static_cast<quint8>(sc.alpha() + t * (ec.alpha() - sc.alpha()));

    QColor result;
    result.setHsv(h, s, v);
    result.setAlpha(opacity);
    dst.fromQColor(result);
}

KoGradientSegment::HSVCCWColorInterpolationStrategy::HSVCCWColorInterpolationStrategy() :
        m_colorSpace(KoColorSpaceRegistry::instance()->rgb8())
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
    // XXX: Added an explicit static cast
    quint8 opacity = static_cast<quint8>(sc.alpha() + t * (se.alpha() - sc.alpha()));

    QColor result;
    result.setHsv(h, s, v);
    result.setAlpha(opacity);
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
