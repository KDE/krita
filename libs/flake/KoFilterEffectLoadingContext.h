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

#ifndef KOFILTEREFFECTLOADINGCONTEXT_H
#define KOFILTEREFFECTLOADINGCONTEXT_H

#include "kritaflake_export.h"

class QRectF;
class QPointF;

#include <QtGlobal>
#include <QString>

/// This class provides a loading context for filter effects
class KRITAFLAKE_EXPORT KoFilterEffectLoadingContext
{
public:
    /**
    * Constructs a new filter effect loading context
    * @param basePath the xml document base path
    */
    explicit KoFilterEffectLoadingContext(const QString &basePath = QString());

    /// Destructor
    virtual ~KoFilterEffectLoadingContext();

    /**
     * Sets the bounding box of the shape a filter is loaded for.
     * The shapes bounding box is used to convert from user space
     * coordinates to bounding box coordinates for filter attributes.
     * @param shapeBound the shapes bounding box
     */
    void setShapeBoundingBox(const QRectF &shapeBound);

    /// Enables conversion of filter units
    void enableFilterUnitsConversion(bool enable);

    /// Enables conversion of filter primitive units
    void enableFilterPrimitiveUnitsConversion(bool enable);

    /// Converts a point value from user space to bounding box coordinates
    QPointF convertFilterUnits(const QPointF &value) const;

    /// Converts an x value from user space to bounding box coordinates
    qreal convertFilterUnitsX(qreal value) const;

    /// Converts an y value from user space to bounding box coordinates
    qreal convertFilterUnitsY(qreal value) const;

    QPointF convertFilterPrimitiveUnits(const QPointF &value) const;

    /// Converts an x value from user space to bounding box coordinates
    qreal convertFilterPrimitiveUnitsX(qreal value) const;

    /// Converts an y value from user space to bounding box coordinates
    qreal convertFilterPrimitiveUnitsY(qreal value) const;

    /// Converts a href to an absolute path name
    QString pathFromHref(const QString &href) const;

private:
    class Private;
    Private * const d;
};

#endif // KOFILTEREFFECTLOADINGCONTEXT_H

