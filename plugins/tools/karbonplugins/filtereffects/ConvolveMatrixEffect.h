/* This file is part of the KDE project
 * Copyright (c) 2010 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef CONVOLVEMATRIXEFFECT_H
#define CONVOLVEMATRIXEFFECT_H

#include "KoFilterEffect.h"
#include <QPointF>
#include <QVector>

#define ConvolveMatrixEffectId "feConvolveMatrix"

class KoFilterEffectLoadingContext;

/// A convolve matrix effect
class ConvolveMatrixEffect : public KoFilterEffect
{
public:
    /// Edge mode, i.e. how the kernel behaves at image edges
    enum EdgeMode {
        Duplicate, ///< duplicates colors at the edges
        Wrap,      ///< takes the colors at the opposite edge
        None       ///< uses values of zero for each color channel
    };

    ConvolveMatrixEffect();

    /// Returns the order of the kernel matrix
    QPoint order() const;

    /// Sets the order of the kernel matrix
    void setOrder(const QPoint &order);

    /// Returns the kernel matrix
    QVector<qreal> kernel() const;

    /// Sets the kernel matrix
    void setKernel(const QVector<qreal> &kernel);

    /// Returns the divisor
    qreal divisor() const;

    /// Sets the divisor
    void setDivisor(qreal divisor);

    /// Returns the bias
    qreal bias() const;

    /// Sets the bias
    void setBias(qreal bias);

    /// Returns the target cell within the kernel
    QPoint target() const;

    /// Sets the target cell within the kernel
    void setTarget(const QPoint &target);

    /// Returns edge mode
    EdgeMode edgeMode() const;

    /// Sets the edge mode
    void setEdgeMode(EdgeMode edgeMode);

    /// Returns if alpha values are preserved
    bool isPreserveAlphaEnabled() const;

    /// Enables/disables preserving alpha values
    void enablePreserveAlpha(bool on);

    /// reimplemented from KoFilterEffect
    virtual QImage processImage(const QImage &image, const KoFilterEffectRenderContext &context) const;
    /// reimplemented from KoFilterEffect
    virtual bool load(const KoXmlElement &element, const KoFilterEffectLoadingContext &context);
    /// reimplemented from KoFilterEffect
    virtual void save(KoXmlWriter &writer);

private:
    void setDefaults();

    QPoint m_order;          ///< the dimension of the kernel
    QVector<qreal> m_kernel; ///< the kernel
    qreal m_divisor;         ///< the divisor
    qreal m_bias;            ///< the bias
    QPoint m_target;         ///< target cell within the kernel
    EdgeMode m_edgeMode;     ///< the edge mode
    QPointF m_kernelUnitLength; ///< the kernel unit length
    bool m_preserveAlpha;    ///< indicates if original alpha values are left intact
};

#endif // CONVOLVEMATRIXEFFECT_H
