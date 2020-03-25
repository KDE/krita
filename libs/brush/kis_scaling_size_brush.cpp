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
      m_useColorAsMask(rhs.m_useColorAsMask)
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

QImage KisScalingSizeBrush::brushTipImage() const
{
    QImage image = KisBrush::brushTipImage();
    if (hasColor() && useColorAsMask()) {
        for (int y = 0; y < image.height(); y++) {
            QRgb *pixel = reinterpret_cast<QRgb *>(image.scanLine(y));
            for (int x = 0; x < image.width(); x++) {
                QRgb c = pixel[x];
                int a = qGray(c);
                pixel[x] = qRgba(a, a, a, qAlpha(c));
            }
        }
    }
    return image;
}
