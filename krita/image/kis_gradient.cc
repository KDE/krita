/*
 *  kis_gradient.cc - part of Krayon
 *
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *                2001 John Califf
 *                2004 Boudewijn Rempt <boud@valdyas.org>
 *                2004 Adrian Page <adrian@pagenet.plus.com>
 *                2004 Sven Langkamp <longamp@reallygood.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <cfloat>
#include <cmath>

#include <QImage>
#include <QTextStream>
#include <QFile>

#include "KoColorSpaceRegistry.h"
#include "KoColorSpace.h"
#include "kogradientmanager.h"

#include <kdebug.h>
#include <klocale.h>

#include "kis_gradient.h"
#include "kis_meta_registry.h"

#define PREVIEW_WIDTH 64
#define PREVIEW_HEIGHT 64

KisGradientSegment::RGBColorInterpolationStrategy *KisGradientSegment::RGBColorInterpolationStrategy::m_instance = 0;
KisGradientSegment::HSVCWColorInterpolationStrategy *KisGradientSegment::HSVCWColorInterpolationStrategy::m_instance = 0;
KisGradientSegment::HSVCCWColorInterpolationStrategy *KisGradientSegment::HSVCCWColorInterpolationStrategy::m_instance = 0;

KisGradientSegment::LinearInterpolationStrategy *KisGradientSegment::LinearInterpolationStrategy::m_instance = 0;
KisGradientSegment::CurvedInterpolationStrategy *KisGradientSegment::CurvedInterpolationStrategy::m_instance = 0;
KisGradientSegment::SineInterpolationStrategy *KisGradientSegment::SineInterpolationStrategy::m_instance = 0;
KisGradientSegment::SphereIncreasingInterpolationStrategy *KisGradientSegment::SphereIncreasingInterpolationStrategy::m_instance = 0;
KisGradientSegment::SphereDecreasingInterpolationStrategy *KisGradientSegment::SphereDecreasingInterpolationStrategy::m_instance = 0;

KisGradient::KisGradient(const QString& file) : super(file)
{
    m_colorSpace = KisMetaRegistry::instance()->csRegistry()->getRGB8();
}

KisGradient::~KisGradient()
{
    for (int i = 0; i < m_segments.count(); i++) {
        delete m_segments[i];
        m_segments[i] = 0;
    }
}

bool KisGradient::load()
{
    return init();
}

bool KisGradient::save()
{
    return false;
}

QImage KisGradient::img()
{
    return m_img;
}

bool KisGradient::init()
{
    KoGradientManager gradLoader;
    KoGradient* grad = gradLoader.loadGradient(filename());

    if( !grad )
        return false;

    m_segments.clear();

    if( grad->colorStops.count() > 1 ) {
        KoColorStop *colstop;
        for(colstop = grad->colorStops.first(); colstop; colstop = grad->colorStops.next()) {
            KoColorStop *colstopNext = grad->colorStops.next();

            if(colstopNext) {
                double midp = colstop->midpoint;
                midp = colstop->offset + ((colstopNext->offset - colstop->offset) * midp);

                quint8 data[4];
                data[2] = static_cast<quint8>(colstop->color1 * 255 + 0.5);
                data[1] = static_cast<quint8>(colstop->color2 * 255 + 0.5);
                data[0] = static_cast<quint8>(colstop->color3 * 255 + 0.5);
                data[3] = static_cast<quint8>(colstop->opacity * OPACITY_OPAQUE + 0.5);

                KoColorSpace * cs = KisMetaRegistry::instance()->csRegistry()->getRGB8();
                KoColor leftColor(data, cs);

                data[2] = static_cast<quint8>(colstopNext->color1 * 255 + 0.5);
                data[1] = static_cast<quint8>(colstopNext->color2 * 255 + 0.5);
                data[0] = static_cast<quint8>(colstopNext->color3 * 255 + 0.5);
                data[3] = static_cast<quint8>(colstopNext->opacity * OPACITY_OPAQUE + 0.5);

                KoColor rightColor(data, cs);

                KisGradientSegment *segment = new KisGradientSegment(colstop->interpolation, colstop->colorType, colstop->offset, midp, colstopNext->offset, leftColor, rightColor);
                Q_CHECK_PTR(segment);

                if ( !segment->isValid() ) {
                    delete segment;
                    return false;
                }

                m_segments.push_back(segment);
                grad->colorStops.prev();
            }
            else {
                grad->colorStops.prev();
                break;
            }
        }
    }
    else
        return false;

    if (!m_segments.isEmpty()) {
        m_img = generatePreview(PREVIEW_WIDTH, PREVIEW_HEIGHT);
        setValid(true);
        return true;
    }
    else {
        return false;
    }
}

void KisGradient::setImage(const QImage& img)
{
    m_img = img;
    m_img.detach();

    setValid(true);
}

KisGradientSegment *KisGradient::segmentAt(double t) const
{
    if (t < DBL_EPSILON) {
        t = 0;
    }
    else
    if (t > 1 - DBL_EPSILON) {
        t = 1;
    }

    Q_ASSERT(m_segments.count() != 0);

    KisGradientSegment *segment = 0;

    for (int i = 0; i < m_segments.count(); i++) {
        if (t > m_segments[i]->startOffset() - DBL_EPSILON && t < m_segments[i]->endOffset() + DBL_EPSILON) {
            segment = m_segments[i];
            break;
        }
    }

    return segment;
}

KoColor KisGradient::colorAt(double t) const
{
    const KisGradientSegment *segment = segmentAt(t);
    Q_ASSERT(segment != 0);

    KoColor color = segment->colorAt(t);
    color.convertTo(m_colorSpace);
    return segment->colorAt(t);
}

QImage KisGradient::generatePreview(int width, int height) const
{
    QImage img(width, height, QImage::Format_RGB32);

    for (int y = 0; y < img.height(); y++) {
        for (int x = 0; x < img.width(); x++) {

            int backgroundRed = 128 + 63 * ((x / 4 + y / 4) % 2);
            int backgroundGreen = backgroundRed;
            int backgroundBlue = backgroundRed;

            KoColor c;
            QColor color;
            quint8 opacity;
            double t = static_cast<double>(x) / (img.width() - 1);

            c = colorAt(t);
            c.toQColor( &color, &opacity );

            double alpha = static_cast<double>(opacity) / OPACITY_OPAQUE;

            int red = static_cast<int>((1 - alpha) * backgroundRed + alpha * color.red() + 0.5);
            int green = static_cast<int>((1 - alpha) * backgroundGreen + alpha * color.green() + 0.5);
            int blue = static_cast<int>((1 - alpha) * backgroundBlue + alpha * color.blue() + 0.5);

            img.setPixel(x, y, qRgb(red, green, blue));
        }
    }

    return img;
}

KisGradientSegment::KisGradientSegment(int interpolationType, int colorInterpolationType, double startOffset, double middleOffset, double endOffset, const KoColor& startColor, const KoColor& endColor)
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
    }
    else
    if (startOffset > 1 - DBL_EPSILON) {
        m_startOffset = 1;
    }
    else {
        m_startOffset = startOffset;
    }

    if (middleOffset < m_startOffset + DBL_EPSILON) {
        m_middleOffset = m_startOffset;
    }
    else
    if (middleOffset > 1 - DBL_EPSILON) {
        m_middleOffset = 1;
    }
    else {
        m_middleOffset = middleOffset;
    }

    if (endOffset < m_middleOffset + DBL_EPSILON) {
        m_endOffset = m_middleOffset;
    }
    else
    if (endOffset > 1 - DBL_EPSILON) {
        m_endOffset = 1;
    }
    else {
        m_endOffset = endOffset;
    }

    m_length = m_endOffset - m_startOffset;

    if (m_length < DBL_EPSILON) {
        m_middleT = 0.5;
    }
    else {
        m_middleT = (m_middleOffset - m_startOffset) / m_length;
    }

    m_startColor = startColor;
    m_endColor = endColor;
}

const KoColor& KisGradientSegment::startColor() const
{
    return m_startColor;
}

const KoColor& KisGradientSegment::endColor() const
{
    return m_endColor;
}

double KisGradientSegment::startOffset() const
{
    return m_startOffset;
}

double KisGradientSegment::middleOffset() const
{
    return m_middleOffset;
}

double KisGradientSegment::endOffset() const
{
    return m_endOffset;
}

void KisGradientSegment::setStartOffset(double t)
{
    m_startOffset = t;
    m_length = m_endOffset - m_startOffset;

    if (m_length < DBL_EPSILON) {
        m_middleT = 0.5;
    }
    else {
        m_middleT = (m_middleOffset - m_startOffset) / m_length;
    }
}
void KisGradientSegment::setMiddleOffset(double t)
{
    m_middleOffset = t;

    if (m_length < DBL_EPSILON) {
        m_middleT = 0.5;
    }
    else {
        m_middleT = (m_middleOffset - m_startOffset) / m_length;
    }
}

void KisGradientSegment::setEndOffset(double t)
{
    m_endOffset = t;
    m_length = m_endOffset - m_startOffset;

    if (m_length < DBL_EPSILON) {
        m_middleT = 0.5;
    }
    else {
        m_middleT = (m_middleOffset - m_startOffset) / m_length;
    }
}

int KisGradientSegment::interpolation() const
{
    return m_interpolator->type();
}

void KisGradientSegment::setInterpolation(int interpolationType)
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

int KisGradientSegment::colorInterpolation() const
{
    return m_colorInterpolator->type();
}

void KisGradientSegment::setColorInterpolation(int colorInterpolationType)
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

KoColor KisGradientSegment::colorAt(double t) const
{
    Q_ASSERT(t > m_startOffset - DBL_EPSILON && t < m_endOffset + DBL_EPSILON);

    double segmentT;

    if (m_length < DBL_EPSILON) {
        segmentT = 0.5;
    }
    else {
        segmentT = (t - m_startOffset) / m_length;
    }

    double colorT = m_interpolator->valueAt(segmentT, m_middleT);

    KoColor color = m_colorInterpolator->colorAt(colorT, m_startColor, m_endColor);

    return color;
}

bool KisGradientSegment::isValid() const
{
    if (m_interpolator == 0 || m_colorInterpolator ==0)
        return false;
    return true;
}

KisGradientSegment::RGBColorInterpolationStrategy::RGBColorInterpolationStrategy()
{
    m_colorSpace = KisMetaRegistry::instance()->csRegistry()->getRGB8();
}

KisGradientSegment::RGBColorInterpolationStrategy *KisGradientSegment::RGBColorInterpolationStrategy::instance()
{
    if (m_instance == 0) {
        m_instance = new RGBColorInterpolationStrategy();
        Q_CHECK_PTR(m_instance);
    }

    return m_instance;
}

KoColor KisGradientSegment::RGBColorInterpolationStrategy::colorAt(double t, KoColor start, KoColor end) const
{
    KoColor result = start;

    start.convertTo(m_colorSpace);
    end.convertTo(m_colorSpace);

    const quint8 *colors[2];
    colors[0] = start.data();
    colors[1] = end.data();

    quint8 colorWeights[2];
    colorWeights[0] = static_cast<quint8>((1.0 - t) * 255 + 0.5);
    colorWeights[1] = 255 - colorWeights[0];

    m_colorSpace->mixColors(colors, colorWeights, 2, result.data());

    return result;
}

KisGradientSegment::HSVCWColorInterpolationStrategy::HSVCWColorInterpolationStrategy()
{
    m_colorSpace = KisMetaRegistry::instance()->csRegistry()->getRGB8();
}

KisGradientSegment::HSVCWColorInterpolationStrategy *KisGradientSegment::HSVCWColorInterpolationStrategy::instance()
{
    if (m_instance == 0) {
        m_instance = new HSVCWColorInterpolationStrategy();
        Q_CHECK_PTR(m_instance);
    }

    return m_instance;
}

KoColor KisGradientSegment::HSVCWColorInterpolationStrategy::colorAt(double t, KoColor start, KoColor end) const
{
    QColor sc;
    QColor ec;
    quint8 startOpacity;
    quint8 endOpacity;

    start.toQColor( &sc, &startOpacity);
    end.toQColor( &ec, &endOpacity);

    int s = static_cast<int>(sc.saturation() + t * (ec.saturation() - sc.saturation()) + 0.5);
    int v = static_cast<int>(sc.value() + t * (ec.value() - sc.value()) + 0.5);
    int h;

    if (ec.hue() < sc.hue()) {
        h = static_cast<int>(ec.hue() + (1 - t) * (sc.hue() - ec.hue()) + 0.5);
    }
    else {
        h = static_cast<int>(ec.hue() + (1 - t) * (360 - ec.hue() + sc.hue()) + 0.5);

        if (h > 359) {
            h -= 360;
        }
    }

    quint8 opacity = startOpacity + t * (endOpacity - startOpacity);

    QColor result;
    result.setHsv( h, s, v);
    KoColorSpace * cs = KisMetaRegistry::instance()->csRegistry()->getRGB8();
    return KoColor( result, opacity, cs);
}

KisGradientSegment::HSVCCWColorInterpolationStrategy::HSVCCWColorInterpolationStrategy()
{
    m_colorSpace = KisMetaRegistry::instance()->csRegistry()->getRGB8();
}


KisGradientSegment::HSVCCWColorInterpolationStrategy *KisGradientSegment::HSVCCWColorInterpolationStrategy::instance()
{
    if (m_instance == 0) {
        m_instance = new HSVCCWColorInterpolationStrategy();
        Q_CHECK_PTR(m_instance);
    }

    return m_instance;
}

KoColor KisGradientSegment::HSVCCWColorInterpolationStrategy::colorAt(double t, KoColor start, KoColor end) const
{
    QColor sc;
    QColor se;
    quint8 startOpacity;
    quint8 endOpacity;

    start.toQColor( &sc, &startOpacity);
    end.toQColor( &se, &endOpacity);

    int s = static_cast<int>(sc.saturation() + t * (se.saturation() - sc.saturation()) + 0.5);
    int v = static_cast<int>(sc.value() + t * (se.value() - sc.value()) + 0.5);
    int h;

    if (sc.hue() < se.hue()) {
        h = static_cast<int>(sc.hue() + t * (se.hue() - sc.hue()) + 0.5);
    }
    else {
        h = static_cast<int>(sc.hue() + t * (360 - sc.hue() + se.hue()) + 0.5);

        if (h > 359) {
            h -= 360;
        }
    }

    quint8 opacity = startOpacity + t * (endOpacity - startOpacity);

    QColor result;
    result.setHsv( h, s, v);
    return KoColor( result, opacity, m_colorSpace);
}

KisGradientSegment::LinearInterpolationStrategy *KisGradientSegment::LinearInterpolationStrategy::instance()
{
    if (m_instance == 0) {
        m_instance = new LinearInterpolationStrategy();
        Q_CHECK_PTR(m_instance);
    }

    return m_instance;
}

double KisGradientSegment::LinearInterpolationStrategy::calcValueAt(double t, double middle)
{
    Q_ASSERT(t > -DBL_EPSILON && t < 1 + DBL_EPSILON);
    Q_ASSERT(middle > -DBL_EPSILON && middle < 1 + DBL_EPSILON);

    double value = 0;

    if (t <= middle) {
        if (middle < DBL_EPSILON) {
            value = 0;
        }
        else {
            value = (t / middle) * 0.5;
        }
    }
    else {
        if (middle > 1 - DBL_EPSILON) {
            value = 1;
        }
        else {
            value = ((t - middle) / (1 - middle)) * 0.5 + 0.5;
        }
    }

    return value;
}

double KisGradientSegment::LinearInterpolationStrategy::valueAt(double t, double middle) const
{
    return calcValueAt(t, middle);
}

KisGradientSegment::CurvedInterpolationStrategy::CurvedInterpolationStrategy()
{
    m_logHalf = log(0.5);
}

KisGradientSegment::CurvedInterpolationStrategy *KisGradientSegment::CurvedInterpolationStrategy::instance()
{
    if (m_instance == 0) {
        m_instance = new CurvedInterpolationStrategy();
        Q_CHECK_PTR(m_instance);
    }

    return m_instance;
}

double KisGradientSegment::CurvedInterpolationStrategy::valueAt(double t, double middle) const
{
    Q_ASSERT(t > -DBL_EPSILON && t < 1 + DBL_EPSILON);
    Q_ASSERT(middle > -DBL_EPSILON && middle < 1 + DBL_EPSILON);

    double value = 0;

    if (middle < DBL_EPSILON) {
        middle = DBL_EPSILON;
    }

    value = pow(t, m_logHalf / log(middle));

    return value;
}

KisGradientSegment::SineInterpolationStrategy *KisGradientSegment::SineInterpolationStrategy::instance()
{
    if (m_instance == 0) {
        m_instance = new SineInterpolationStrategy();
        Q_CHECK_PTR(m_instance);
    }

    return m_instance;
}

double KisGradientSegment::SineInterpolationStrategy::valueAt(double t, double middle) const
{
    double lt = LinearInterpolationStrategy::calcValueAt(t, middle);
    double value = (sin(-M_PI_2 + M_PI * lt) + 1.0) / 2.0;

    return value;
}

KisGradientSegment::SphereIncreasingInterpolationStrategy *KisGradientSegment::SphereIncreasingInterpolationStrategy::instance()
{
    if (m_instance == 0) {
        m_instance = new SphereIncreasingInterpolationStrategy();
        Q_CHECK_PTR(m_instance);
    }

    return m_instance;
}

double KisGradientSegment::SphereIncreasingInterpolationStrategy::valueAt(double t, double middle) const
{
    double lt = LinearInterpolationStrategy::calcValueAt(t, middle) - 1;
    double value = sqrt(1 - lt * lt);

    return value;
}

KisGradientSegment::SphereDecreasingInterpolationStrategy *KisGradientSegment::SphereDecreasingInterpolationStrategy::instance()
{
    if (m_instance == 0) {
        m_instance = new SphereDecreasingInterpolationStrategy();
        Q_CHECK_PTR(m_instance);
    }

    return m_instance;
}

double KisGradientSegment::SphereDecreasingInterpolationStrategy::valueAt(double t, double middle) const
{
    double lt = LinearInterpolationStrategy::calcValueAt(t, middle);
    double value = 1 - sqrt(1 - lt * lt);

    return value;
}

#include "kis_gradient.moc"

