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

#include "KoMarkerCollection.h"

#include "KoMarker.h"
#include "KoMarkerSharedLoadingData.h"
#include <KoXmlReader.h>
#include <KoShapeLoadingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoOdfReadStore.h>
#include <kstandarddirs.h>
#include <kdebug.h>

class KoMarkerCollection::Private
{
public:
    ~Private()
    {
        qDeleteAll(markers);
    }

    QList<KoMarker *> markers;
};

KoMarkerCollection::KoMarkerCollection(QObject *parent)
: QObject(parent)
, d(new Private)
{
    // Add no marker so the user can remove a marker from the line.
    d->markers.append(0);
    // Add default markers
    loadDefaultMarkers();
}

KoMarkerCollection::~KoMarkerCollection()
{
    delete d;
}

bool KoMarkerCollection::loadOdf(KoShapeLoadingContext &context)
{
    kDebug(30006);
    QHash<QString, KoMarker*> lookupTable;

    const QHash<QString, KoXmlElement*> markers = context.odfLoadingContext().stylesReader().drawStyles("marker");
    loadOdfMarkers(markers, context, lookupTable);

    KoMarkerSharedLoadingData * sharedMarkerData = new KoMarkerSharedLoadingData(lookupTable);
    context.addSharedData(MARKER_SHARED_LOADING_ID, sharedMarkerData);

    return true;
}

void KoMarkerCollection::loadDefaultMarkers()
{
    // use the same mechanism for loading the markers that are available
    // per default as when loading the normal markers.
    KoOdfStylesReader markerReader;
    KoOdfLoadingContext odfContext(markerReader, 0);
    KoShapeLoadingContext shapeContext(odfContext, 0);
    KoXmlDocument doc;
    QString filePath(KStandardDirs::locate("styles", "markers.xml"));
    if (!filePath.isEmpty()) {
        QFile file(filePath);
        QString errorMessage;
        if (KoOdfReadStore::loadAndParse(&file, doc, errorMessage, filePath)) {
            markerReader.createStyleMap(doc, true);

            QHash<QString, KoMarker*> lookupTable;
            const QHash<QString, KoXmlElement*> defaultMarkers = markerReader.drawStyles("marker");
            loadOdfMarkers(defaultMarkers, shapeContext, lookupTable);
        }
        else {
            kWarning(30006) << "reading of" << filePath << "failed:" << errorMessage;
        }
    }
    else {
        kDebug(30006) << "markers.xml not found";
    }
}

void KoMarkerCollection::loadOdfMarkers(const QHash<QString, KoXmlElement*> &markers, KoShapeLoadingContext &context, QHash<QString, KoMarker*> &lookupTable)
{
    QHash<QString, KoXmlElement*>::const_iterator it(markers.constBegin());
    for (; it != markers.constEnd(); ++it) {
        KoMarker *marker = new KoMarker();
        if (marker->loadOdf(*(it.value()), context)) {
            KoMarker *m = addMarker(marker);
            lookupTable.insert(it.key(), m);
            kDebug(30006) << "loaded marker" << it.key() << marker << m;
            if (m != marker) {
                delete marker;
            }
        }
        else {
            delete marker;
        }
    }
}

QList<KoMarker*> KoMarkerCollection::markers() const
{
    return d->markers;
}

KoMarker * KoMarkerCollection::addMarker(KoMarker *marker)
{
    foreach (KoMarker *m, d->markers) {
        if (marker == m) {
            return marker;
        }
        if (m && *marker == *m) {
            kDebug(30006) << "marker is the same as other";
            return m;
        }
    }
    d->markers.append(marker);
    return marker;
}
