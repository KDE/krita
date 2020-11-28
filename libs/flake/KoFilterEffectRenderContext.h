/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2009 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KOFILTEREFFECTRENDERCONTEXT_H
#define KOFILTEREFFECTRENDERCONTEXT_H

#include "kritaflake_export.h"

#include <QtGlobal>

class QRectF;
class QPointF;
class KoViewConverter;

/// This class provides the render context for filter effects
class KRITAFLAKE_EXPORT KoFilterEffectRenderContext
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
