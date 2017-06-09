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

#ifndef KOMARKERCOLLECTION_H
#define KOMARKERCOLLECTION_H

#include "kritaflake_export.h"

#include <QObject>
#include <QList>
#include <QHash>
#include <QMetaType>

class KoMarker;
#include <KoXmlReaderForward.h>
class KoShapeLoadingContext;

class KRITAFLAKE_EXPORT KoMarkerCollection : public QObject
{
    Q_OBJECT
public:
    explicit KoMarkerCollection(QObject *parent = 0);
    ~KoMarkerCollection() override;

    QList<KoMarker*> markers() const;

    /**
     * Add marker to collection
     *
     * The collection checks if a marker with the same content exists and if so deletes the
     * passed marker and returns a pointer to an existing marker. If no such marker exists it
     * adds the marker and return the same pointer as passed.
     * Calling that function passes ownership of the marker to this class.
     *
     * @param marker Marker to add
     * @return pointer to marker that should be used. This might be different to the marker passed
     */
    KoMarker * addMarker(KoMarker *marker);

    void loadMarkersFromFile(const QString &svgFile);

private:
    /// load the markers that are available per default.
    void loadDefaultMarkers();

    class Private;
    Private * const d;
};

Q_DECLARE_METATYPE(KoMarkerCollection *)

#endif /* KOMARKERCOLLECTION_H */
