/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "TestKoMarkerCollection.h"

#include <QTest>
#include <QFileInfo>
#include <QPainter>
#include <KoMarker.h>
#include <KoMarkerCollection.h>
#include <KoPathShape.h>
#include <KoColorBackground.h>

#include "kis_debug.h"
#include "../../sdk/tests/qimage_test_util.h"
#include <sdk/tests/kistest.h>

#include <cmath>


void initMarkerCollection(KoMarkerCollection *collection)
{
    QCOMPARE(collection->markers().size(), 1);

    const QString fileName = TestUtil::fetchDataFileLazy("test_markers.svg");
    QVERIFY(QFileInfo(fileName).exists());

    collection->loadMarkersFromFile(fileName);
    QCOMPARE(collection->markers().size(), 10);
}

void TestKoMarkerCollection::testLoadMarkersFromFile()
{
    KoMarkerCollection collection;
    initMarkerCollection(&collection);
}

void TestKoMarkerCollection::testDeduplication()
{
    QPainterPath path1;
    path1.addRect(QRect(5,5,15,15));

    KoPathShape *shape1(KoPathShape::createShapeFromPainterPath(path1));
    shape1->setBackground(QSharedPointer<KoColorBackground>(new KoColorBackground(Qt::blue)));

    KoMarker *marker(new KoMarker());
    marker->setAutoOrientation(true);
    marker->setShapes({shape1});

    KoMarkerCollection collection;
    QCOMPARE(collection.markers().size(), 1);

    KoMarker *clonedMarker = new KoMarker(*marker);

    collection.addMarker(marker);
    QCOMPARE(collection.markers().size(), 2);

    collection.addMarker(marker);
    QCOMPARE(collection.markers().size(), 2);

    collection.addMarker(clonedMarker);
    QCOMPARE(collection.markers().size(), 2);
}

void testOneMarkerPosition(KoMarker *marker, KoFlake::MarkerPosition position, const QString &testName)
{
    QImage image(30,30, QImage::Format_ARGB32);
    image.fill(0);
    QPainter painter(&image);

    QPen pen(Qt::black, 2);
    marker->drawPreview(&painter, image.rect(), pen, position);

    QVERIFY(TestUtil::checkQImage(image, "marker_collection", "preview", testName));
}

void TestKoMarkerCollection::testMarkerBounds()
{
    KoMarkerCollection collection;
    initMarkerCollection(&collection);

    QList<KoMarker*> allMarkers = collection.markers();
    KoMarker *marker = allMarkers[3];

    QCOMPARE(marker->boundingRect(1, 0).toAlignedRect(), QRect(-8,-4,11,8));
    QCOMPARE(marker->boundingRect(1, M_PI).toAlignedRect(), QRect(-3,-4,11,8));

    QCOMPARE(marker->outline(1, 0).boundingRect().toAlignedRect(), QRect(-6,-2,7,4));
    QCOMPARE(marker->outline(1, M_PI).boundingRect().toAlignedRect(), QRect(-1,-2,7,4));

    testOneMarkerPosition(marker, KoFlake::StartMarker, "start_marker");
    testOneMarkerPosition(marker, KoFlake::MidMarker, "mid_marker");
    testOneMarkerPosition(marker, KoFlake::EndMarker, "end_marker");
}

KISTEST_MAIN(TestKoMarkerCollection)
