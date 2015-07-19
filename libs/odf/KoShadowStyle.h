/* This file is part of the KDE project
 *
 * Copyright (C) 2011 Pierre Ducroquet <pinaraf@pinaraf.info>
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

#ifndef KOSHADOWSTYLE_H
#define KOSHADOWSTYLE_H

#include "koodf_export.h"

#include <QColor>
#include <QMetaType>
#include <QPointF>
#include <QSharedData>

class KoShadowStylePrivate;

/**
 * A container and parser for shadows as defined in the
 * OpenDocument specification.
 * Applies to at least :
 * - graphic elements,
 * - headers-footers,
 * - pages,
 * - paragraphs,
 * - tables and table cells.
 */
class KOODF_EXPORT KoShadowStyle
{
public:
    /// Default constructor, constructs an empty shadow
    KoShadowStyle();
    /// Copy constructor
    KoShadowStyle(const KoShadowStyle &other);
    ~KoShadowStyle();


    // Holds data about one of the shadow this shadow contains
    struct KOODF_EXPORT ShadowData {
        ShadowData();
        bool operator==(const ShadowData &other) const;
        QColor color;
        QPointF offset;
        qreal radius;
    };


    bool operator==(const KoShadowStyle &other) const;
    bool operator!=(const KoShadowStyle &other) const;

    /**
     * Loads the given OpenDocument-defined shadow
     * in this KoShadow object.
     * @param shadow the shadow to parse
     * @return true when the parsing was successful
     */
    bool loadOdf(const QString &shadow);

    /**
     * Returns this shadow as a string formatted like an
     * OpenDocument-defined shadow.
     */
    QString saveOdf() const;

    /**
     * Returns the number of shadows that are contained in this shadow
     */
    int shadowCount() const;


private:
    QSharedDataPointer<KoShadowStylePrivate> d;
};

Q_DECLARE_TYPEINFO(KoShadowStyle::ShadowData, Q_MOVABLE_TYPE);

Q_DECLARE_METATYPE(KoShadowStyle)

#endif

