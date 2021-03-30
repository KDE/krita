/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2011 Thorsten Zachmann <zachmann@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOMARKERCOLLECTION_H
#define KOMARKERCOLLECTION_H

#include "kritaflake_export.h"

#include <QObject>
#include <QList>
#include <QHash>
#include <QMetaType>

class KoMarker;
#include <QDomDocument>
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
