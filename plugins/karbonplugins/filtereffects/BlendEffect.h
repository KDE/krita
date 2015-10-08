/* This file is part of the KDE project
 * Copyright (c) 2009 Jan Hambrecht <jaham@gmx.net>
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

#ifndef BLENDEFFECT_H
#define BLENDEFFECT_H

#include "KoFilterEffect.h"

#define BlendEffectId "feBlend"

/// A color matrix effect
class BlendEffect : public KoFilterEffect
{
public:
    enum BlendMode {
        Normal,
        Multiply,
        Screen,
        Darken,
        Lighten
    };

    BlendEffect();

    /// Returns the type of the color matrix
    BlendMode blendMode() const;

    /// Sets the blend mode
    void setBlendMode(BlendMode blendMode);

    /// reimplemented from KoFilterEffect
    virtual QImage processImage(const QImage &image, const KoFilterEffectRenderContext &context) const;
    /// reimplemented from KoFilterEffect
    virtual QImage processImages(const QList<QImage> &images, const KoFilterEffectRenderContext &context) const;
    /// reimplemented from KoFilterEffect
    virtual bool load(const KoXmlElement &element, const KoFilterEffectLoadingContext &context);
    /// reimplemented from KoFilterEffect
    virtual void save(KoXmlWriter &writer);

private:

    BlendMode m_blendMode; ///< the blend mode
};

#endif // BLENDEFFECT_H
