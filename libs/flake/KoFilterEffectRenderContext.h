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

#ifndef KOFILTEREFFECTRENDERCONTEXT_H
#define KOFILTEREFFECTRENDERCONTEXT_H

#include "flake_export.h"

#include <QtGlobal>

class QRectF;
class QPointF;
class KoViewConverter;

/// This class provides the render context for filter effects
class FLAKE_EXPORT KoFilterEffectRenderContext
{
public:
    explicit KoFilterEffectRenderContext(const KoViewConverter &converter);
    ~KoFilterEffectRenderContext();

    /// Returns the filter region the filter is applied to
    QRectF filterRegion() const;

    /// Sets the filter region the filter is applied to
    void setFilterRegion(const QRectF &filterRegion);

    /// Sets the shape bounding box used to convert to user space coordinates
    void setShapeBoundingBox(const QRectF &bound);

    /// Converts point from bounding box coordinates to user space coordinates
    QPointF toUserSpace(const QPointF &value) const;

    /// Converts x-coordinate from bounding box coordinates to user space coordinates
    qreal toUserSpaceX(qreal value) const;

    /// Converts y-coordinate from bounding box coordinates to user space coordinates
    qreal toUserSpaceY(qreal value) const;

    /// Returns the view converter
    const KoViewConverter *viewConverter() const;

private:
    class Private;
    Private * const d;
};

#endif // KOFILTEREFFECTRENDERCONTEXT_H
