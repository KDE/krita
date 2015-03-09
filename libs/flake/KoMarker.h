/* This file is part of the KDE project
   Copyright (C) 2011 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOMARKER_H
#define KOMARKER_H

#include <QMetaType>
#include <QSharedData>

#include "flake_export.h"

class KoXmlElement;
class KoShapeLoadingContext;
class KoShapeSavingContext;
class QString;
class QPainterPath;

class  FLAKE_EXPORT KoMarker : public QSharedData
{
public:
    KoMarker();
    ~KoMarker();

    /**
     * Load the marker
     *
     * @param element The xml element containing the marker
     * @param context The shape loading context
     */
    bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context);

    /**
     * Save the marker
     *
     * @return The reference of the marker.
     */
    QString saveOdf(KoShapeSavingContext &context) const;

    /**
     * Display name of the marker
     *
     * @return Display name of the marker
     */
    QString name() const;

    /**
     * Get the path of the marker
     *
     * It calculates the offset depending on the line width
     *
     * @param The width of the line the marker is attached to.
     * @return the path of the marker
     */
    QPainterPath path(qreal width) const;

    bool operator==(const KoMarker &other) const;

private:
    class Private;
    Private * const d;
};

Q_DECLARE_METATYPE(KoMarker*)

#endif /* KOMARKER_H */
