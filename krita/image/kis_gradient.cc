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

#include <qimage.h>
#include <qtextstream.h>
#include <qfile.h>

#include "koColor.h"
#include "kogradientmanager.h"

#include <kdebug.h>
#include <klocale.h>

#include "kis_gradient.h"

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
                KoColor leftRgb((int)(colstop->color1 * 255 + 0.5), (int)(colstop->color2 * 255 + 0.5), (int)(colstop->color3 * 255 + 0.5));
                KoColor rightRgb((int)(colstopNext->color1 * 255 + 0.5), (int)(colstopNext->color2 * 255 + 0.5), (int)(colstopNext->color3 * 255 + 0.5));

                double midp = colstop->midpoint;
                midp = colstop->offset + ((colstopNext->offset - colstop->offset) * midp);

                Color leftColor(leftRgb.color(), colstop->opacity);
                Color rightColor(rightRgb.color(), colstopNext->opacity);

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

void KisGradient::colorAt(double t, QColor *color, quint8 *opacity) const
{
    const KisGradientSegment *segment = segmentAt(t);
    Q_ASSERT(segment != 0);

    if (segment) {
        Color col = segment->colorAt(t);
        *color = col.color();
        *opacity = static_cast<quint8>(col.alpha() * OPACITY_OPAQUE + 0.5);
    }
}

QImage KisGradient::generatePreview(int width, int height) const
{
    QImage img(width, height, QImage::Format_RGB32);

    for (int y = 0; y < img.height(); y++) {
        for (int x = 0; x < img.width(); x++) {

            int backgroundRed = 128 + 63 * ((x / 4 + y / 4) % 2);
            int backgroundGreen = backgroundRed;
            int backgroundBlue = backgroundRed;

            QColor color;
            quint8 opacity;
            double t = static_cast<double>(x) / (img.width() - 1);

            colorAt(t,  &color, &opacity);

            double alpha = static_cast<double>(opacity) / OPACITY_OPAQUE;

            int red = static_cast<int>((1 - alpha) * backgroundRed + alpha * color.red() + 0.5);
            int green = static_cast<int>((1 - alpha) * backgroundGreen + alpha * color.green() + 0.5);
            int blue = static_cast<int>((1 - alpha) * backgroundBlue + alpha * color.blue() + 0.5);

            img.setPixel(x, y, qRgb(red, green, blue));
        }
    }

    return img;
}

KisGradientSegment::KisGradientSegment(int interpolationType, int colorInterpolationType, double startOffset, double middleOffset, double endOffset, const Color& startColor, const Color& endColor)
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

const Color& KisGradientSegment::startColor() const
{
    return m_startColor;
}

const Color& KisGradientSegment::endColor() const
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

Color KisGradientSegment::colorAt(double t) const
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

    Color color = m_colorInterpolator->colorAt(colorT, m_startColor, m_endColor);

    return color;
}

bool KisGradientSegment::isValid() const
{
    if (m_interpolator == 0 || m_colorInterpolator ==0)
        return false;
    return true;
}

KisGradientSegment::RGBColorInterpolationStrategy *KisGradientSegment::RGBColorInterpolationStrategy::instance()
{
    if (m_instance == 0) {
        m_instance = new RGBColorInterpolationStrategy();
        Q_CHECK_PTR(m_instance);
    }

    return m_instance;
}

Color KisGradientSegment::RGBColorInterpolationStrategy::colorAt(double t, Color start, Color end) const
{
    int red = static_cast<int>(start.color().red() + t * (end.color().red() - start.color().red()) + 0.5);
    int green = static_cast<int>(start.color().green() + t * (end.color().green() - start.color().green()) + 0.5);
    int blue = static_cast<int>(start.color().blue() + t * (end.color().blue() - start.color().blue()) + 0.5);
    double alpha = start.alpha() + t * (end.alpha() - start.alpha());

    return Color(QColor(red, green, blue), alpha);
}

KisGradientSegment::HSVCWColorInterpolationStrategy *KisGradientSegment::HSVCWColorInterpolationStrategy::instance()
{
    if (m_instance == 0) {
        m_instance = new HSVCWColorInterpolationStrategy();
        Q_CHECK_PTR(m_instance);
    }

    return m_instance;
}

Color KisGradientSegment::HSVCWColorInterpolationStrategy::colorAt(double t, Color start, Color end) const
{
    KoColor sc = KoColor(start.color());
    KoColor ec = KoColor(end.color());
    
    int s = static_cast<int>(sc.S() + t * (ec.S() - sc.S()) + 0.5);
    int v = static_cast<int>(sc.V() + t * (ec.V() - sc.V()) + 0.5);
    int h;
    
    if (ec.H() < sc.H()) {
        h = static_cast<int>(ec.H() + (1 - t) * (sc.H() - ec.H()) + 0.5);
    }
    else {
        h = static_cast<int>(ec.H() + (1 - t) * (360 - ec.H() + sc.H()) + 0.5);
        
        if (h > 359) {
            h -= 360;
        }
    }
    
    double alpha = start.alpha() + t * (end.alpha() - start.alpha());

    return Color(KoColor(h, s, v, KoColor::csHSV).color(), alpha);
}

KisGradientSegment::HSVCCWColorInterpolationStrategy *KisGradientSegment::HSVCCWColorInterpolationStrategy::instance()
{
    if (m_instance == 0) {
        m_instance = new HSVCCWColorInterpolationStrategy();
        Q_CHECK_PTR(m_instance);
    }

    return m_instance;
}

Color KisGradientSegment::HSVCCWColorInterpolationStrategy::colorAt(double t, Color start, Color end) const
{
    KoColor sc = KoColor(start.color());
    KoColor se = KoColor(end.color());

    int s = static_cast<int>(sc.S() + t * (se.S() - sc.S()) + 0.5);
    int v = static_cast<int>(sc.V() + t * (se.V() - sc.V()) + 0.5);
    int h;

    if (sc.H() < se.H()) {
        h = static_cast<int>(sc.H() + t * (se.H() - sc.H()) + 0.5);
    }
    else {
        h = static_cast<int>(sc.H() + t * (360 - sc.H() + se.H()) + 0.5);

        if (h > 359) {
            h -= 360;
        }
    }

    double alpha = start.alpha() + t * (end.alpha() - start.alpha());

    return Color(KoColor(h, s, v, KoColor::csHSV).color(), alpha);
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

