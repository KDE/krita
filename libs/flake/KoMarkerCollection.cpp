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

#include <QFile>

#include <klocalizedstring.h>
#include "KoMarker.h"
#include <KoXmlReader.h>
#include <FlakeDebug.h>
#include <KoResourcePaths.h>
#include <SvgParser.h>
#include <QFileInfo>
#include <KoDocumentResourceManager.h>

#include "kis_debug.h"

// WARNING: there is a bug in GCC! It doesn't warn that we are
//          deleting an uninitialized type here!
#include <KoShape.h>


class Q_DECL_HIDDEN KoMarkerCollection::Private
{
public:
    ~Private()
    {
    }

    QList<QExplicitlySharedDataPointer<KoMarker> > markers;
};

KoMarkerCollection::KoMarkerCollection(QObject *parent)
: QObject(parent)
, d(new Private)
{
    // Add no marker so the user can remove a marker from the line.
    d->markers.append(QExplicitlySharedDataPointer<KoMarker>(0));
    // Add default markers
    loadDefaultMarkers();
}

KoMarkerCollection::~KoMarkerCollection()
{
    delete d;
}

void KoMarkerCollection::loadMarkersFromFile(const QString &svgFile)
{
    QFile file(svgFile);
    if (!file.exists()) return;

    if (!file.open(QIODevice::ReadOnly)) return;

    QString errorMsg;
    int errorLine = 0;
    int errorColumn;

    KoXmlDocument doc = SvgParser::createDocumentFromSvg(&file, &errorMsg, &errorLine, &errorColumn);
    if (doc.isNull()) {
        errKrita << "Parsing error in " << svgFile << "! Aborting!" << endl
        << " In line: " << errorLine << ", column: " << errorColumn << endl
        << " Error message: " << errorMsg << endl;
        errKrita << i18n("Parsing error in the main document at line %1, column %2\nError message: %3"
                         , errorLine , errorColumn , errorMsg);
        return;
    }

    KoDocumentResourceManager manager;
    SvgParser parser(&manager);
    parser.setResolution(QRectF(0,0,100,100), 72); // initialize with default values
    parser.setXmlBaseDir(QFileInfo(svgFile).absolutePath());

    parser.setFileFetcher(
        [](const QString &fileName) {
            QFile file(fileName);
            if (!file.exists()) return QByteArray();

            file.open(QIODevice::ReadOnly);
            return file.readAll();
        });

    QSizeF fragmentSize;
    QList<KoShape*> shapes = parser.parseSvg(doc.documentElement(), &fragmentSize);
    qDeleteAll(shapes);

    Q_FOREACH (const QExplicitlySharedDataPointer<KoMarker> &marker, parser.knownMarkers()) {
        addMarker(marker.data());
    }
}

void KoMarkerCollection::loadDefaultMarkers()
{
    QString filePath = KoResourcePaths::findResource("data", "styles/markers.svg");
    loadMarkersFromFile(filePath);
}

QList<KoMarker*> KoMarkerCollection::markers() const
{
    QList<KoMarker*> markerList;
    foreach (const QExplicitlySharedDataPointer<KoMarker>& m, d->markers){
        markerList.append(m.data());
    }
    return markerList;
}

KoMarker * KoMarkerCollection::addMarker(KoMarker *marker)
{
    foreach (const QExplicitlySharedDataPointer<KoMarker>& m, d->markers) {
        if (marker == m.data()) {
            return marker;
        }
        if (m && *marker == *m) {
            debugFlake << "marker is the same as other";
            return m.data();
        }
    }
    d->markers.append(QExplicitlySharedDataPointer<KoMarker>(marker));
    return marker;
}
