/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_scaling_size_brush.h"

KisScalingSizeBrush::KisScalingSizeBrush()
    : KisBrush()
{
}

KisScalingSizeBrush::KisScalingSizeBrush(const QString &filename)
    : KisBrush(filename)
{
}

KisScalingSizeBrush::KisScalingSizeBrush(const KisScalingSizeBrush &rhs)
    : KisBrush(rhs),
      m_useColorAsMask(rhs.m_useColorAsMask),
      m_brightnessAdjustment(rhs.m_brightnessAdjustment),
      m_contrastAdjustment(rhs.m_contrastAdjustment)
{
    setName(rhs.name());
    setValid(rhs.valid());
}

qreal KisScalingSizeBrush::userEffectiveSize() const
{
    return this->width() * this->scale();
}

void KisScalingSizeBrush::setUserEffectiveSize(qreal value)
{
    this->setScale(value / this->width());
}

void KisScalingSizeBrush::setUseColorAsMask(bool useColorAsMask)
{
    /**
     * WARNING: There is a problem in the brush server, since it
     * returns not copies of brushes, but direct pointers to them. It
     * means that the brushes are shared among all the currently
     * present paintops, which might be a problem for e.g. Multihand
     * Brush Tool.
     *
     * Right now, all the instances of Multihand Brush Tool share the
     * same brush, so there is no problem in this sharing, unless we
     * reset the internal state of the brush on our way.
     */

    if (useColorAsMask != m_useColorAsMask) {
        m_useColorAsMask = useColorAsMask;
        resetBoundary();
        clearBrushPyramid();
    }
}

bool KisScalingSizeBrush::useColorAsMask() const
{
    return m_useColorAsMask;
}

#include <KoColorSpaceMaths.h>

QImage KisScalingSizeBrush::brushTipImage() const
{
    QImage image = KisBrush::brushTipImage();
    if (hasColor() && useColorAsMask()) {
        if (!qFuzzyIsNull(m_brightnessAdjustment) || !qFuzzyIsNull(m_contrastAdjustment)) {

            const int half = KoColorSpaceMathsTraits<quint8>::halfValue;
            const int unit = KoColorSpaceMathsTraits<quint8>::unitValue;

            const qreal midPoint = half * (1.0 + 0.5 * m_brightnessAdjustment);
            const qreal loA = 2 * midPoint / (1.0 - m_contrastAdjustment) / unit;
            const qreal hiA = 2 * (unit - midPoint) / (1.0 - m_contrastAdjustment) / unit;

            const qreal loB = midPoint - half * loA;
            const qreal hiB = midPoint - half * hiA;

            for (int y = 0; y < image.height(); y++) {
                QRgb *pixel = reinterpret_cast<QRgb *>(image.scanLine(y));
                for (int x = 0; x < image.width(); x++) {
                    QRgb c = pixel[x];

                    int v = qGray(c);

                    if (v >= half) {
                        v = qMin(unit, qRound(hiA * v + hiB));
                    } else {
                        v = qMax(0, qRound(loA * v + loB));
                    }

                    pixel[x] = qRgba(v, v, v, qAlpha(c));
                }
            }
        } else {
            for (int y = 0; y < image.height(); y++) {
                QRgb *pixel = reinterpret_cast<QRgb *>(image.scanLine(y));
                for (int x = 0; x < image.width(); x++) {
                    QRgb c = pixel[x];

                    int v = qGray(c);
                    pixel[x] = qRgba(v, v, v, qAlpha(c));
                }
            }
        }
    }
    return image;
}

void KisScalingSizeBrush::setBrightnessAdjustment(qreal value)
{
    m_brightnessAdjustment = value;
}

void KisScalingSizeBrush::setContrastAdjustment(qreal value)
{
    m_contrastAdjustment = value;
}

qreal KisScalingSizeBrush::brightnessAdjustment() const
{
    return m_brightnessAdjustment;
}

qreal KisScalingSizeBrush::contrastAdjustment() const
{
    return m_contrastAdjustment;
}

#include <QDomElement>

void KisScalingSizeBrush::toXML(QDomDocument& d, QDomElement& e) const
{
    e.setAttribute("ColorAsMask", QString::number((int)useColorAsMask()));
    e.setAttribute("BrightnessAdjustment", QString::number(m_brightnessAdjustment));
    e.setAttribute("ContrastAdjustment", QString::number(m_contrastAdjustment));
    KisBrush::toXML(d, e);
}
