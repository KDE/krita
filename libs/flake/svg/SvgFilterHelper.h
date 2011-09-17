/* This file is part of the KDE project
 * Copyright (C) 2009 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef SVGFILTERHELPER_H
#define SVGFILTERHELPER_H

#include <KoXmlReader.h>
#include <QtCore/QRectF>

class SvgFilterHelper
{
public:
    enum Units { UserSpaceOnUse, ObjectBoundingBox };

    SvgFilterHelper();
    ~SvgFilterHelper();

    /// Set the filter units type
    void setFilterUnits(Units filterUnits);
    /// Returns the filter units type
    Units filterUnits() const;

    /// Set the filter primitive units type
    void setPrimitiveUnits(Units primitiveUnits);
    /// Returns the filter primitive units type
    Units primitiveUnits() const;

    /// Sets filter position
    void setPosition(const QPointF & position);
    /// Returns filter position (objectBound is used when filterUnits == ObjectBoundingBox)
    QPointF position(const QRectF & objectBound) const;

    /// Sets filter size
    void setSize(const QSizeF & size);
    /// Returns filter size (objectBound is used when filterUnits == ObjectBoundingBox)
    QSizeF size(const QRectF & objectBound) const;

    /// Sets the dom element containing the filter
    void setContent(const KoXmlElement &content);
    /// Return the filer element
    KoXmlElement content() const;

    static QPointF toUserSpace(const QPointF &position, const QRectF &objectBound);
    static QSizeF toUserSpace(const QSizeF &size, const QRectF &objectBound);
private:
    Units m_filterUnits;
    Units m_primitiveUnits;
    QPointF m_position;
    QSizeF m_size;
    KoXmlElement m_filterContent;
};

#endif // SVGFILTERHELPER_H
