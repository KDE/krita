/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include <qtest_kde.h>

#include <QTime>

#include "kis_paint_device_test.h"

#include <KoStore.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_layer.h"
#include "kis_paint_layer.h"
#include "kis_selection.h"
#include "kis_datamanager.h"
#include "kis_global.h"
#include "testutil.h"
#include "kis_transaction.h"

void KisPaintDeviceTest::testCreation()
{
    KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice( cs );
    QVERIFY( dev->objectName() == QString() );

    dev = new KisPaintDevice( cs, "test" );
    QVERIFY( dev->objectName() == "test" );
    QVERIFY( dev->colorSpace() == cs );
    QVERIFY( dev->x() == 0 );
    QVERIFY( dev->y() == 0 );
    QVERIFY( dev->pixelSize() == cs->pixelSize() );
    QVERIFY( dev->channelCount() == cs->channelCount() );
    QVERIFY( dev->dataManager() != 0 );
    QVERIFY( dev->paintEngine() != 0 );

    KisImageSP image = new KisImage(0, 1000, 1000, cs, "merge test");
    KisPaintLayerSP layer = new KisPaintLayer( image, "bla", 125 );

    dev = new KisPaintDevice( layer.data(), cs, "test2" );
    QVERIFY( dev->name() == dev->objectName() );
    QVERIFY( dev->objectName() == QString( "test2" ) );
    QVERIFY( dev->colorSpace() == cs );
    QVERIFY( dev->x() == 0 );
    QVERIFY( dev->y() == 0 );
    QVERIFY( dev->pixelSize() == cs->pixelSize() );
    QVERIFY( dev->channelCount() == cs->channelCount() );
    QVERIFY( dev->dataManager() != 0 );
    QVERIFY( dev->paintEngine() != 0 );

    // Let the layer go out of scope and see what happens
    {
        KisPaintLayerSP l2 = new KisPaintLayer( image, "blabla", 250 );
        dev = new KisPaintDevice( l2.data(), cs, "test3" );
    }

}

void KisPaintDeviceTest::testPaintEngine()
{
    // Create a paint device and a QImage. Let the paintEngine paint
    // on both, then do a pixel-by-pixel comparison
    KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice( cs );
    quint8* pixel = cs->allocPixelBuffer( 1 );
    cs->fromQColor( Qt::white, pixel );
    dev->fill( 0, 0, 512, 512, pixel);

    QImage img( 512, 512, QImage::Format_ARGB32 );
    QPainter p( &img );
    p.fillRect( QRect( 0, 0, 512, 512 ), Qt::white );
    p.end();


    QPainter gc( dev.data() );
    QPainter gc2( &img );

    // drawArc
    {
        QRectF rectangle(10.0, 20.0, 80.0, 60.0);
        int startAngle = 30 * 16;
        int spanAngle = 120 * 16;
        gc.drawArc( rectangle, startAngle, spanAngle );
        gc2.drawArc( rectangle, startAngle, spanAngle );
    }

    // drawChord
    {
        QRectF rectangle(10.0, 20.0, 80.0, 60.0);
        int startAngle = 30 * 16;
        int spanAngle = 120 * 16;

        gc.drawChord(rectangle, startAngle, spanAngle);
        gc2.drawChord(rectangle, startAngle, spanAngle);
    }

    // drawConvexPolygon
    {
        static const QPointF points[4] = {
            QPointF(10.0, 80.0),
            QPointF(20.0, 10.0),
            QPointF(80.0, 30.0),
            QPointF(90.0, 70.0)
        };


        gc.drawConvexPolygon(points, 4);
        gc2.drawConvexPolygon(points, 4);
    }

    // drawEllipse
    {
         QRectF rectangle(10.0, 20.0, 80.0, 60.0);

         gc.drawEllipse( rectangle );
         gc2.drawEllipse( rectangle );

    }

    // drawImage
    {
        QRectF target(100.0, 200.0, 80.0, 60.0);
        QRectF source(0.0, 0.0, 70.0, 40.0);
        QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa.png");

        gc.drawImage(target, image, source);
        gc2.drawImage(target, image, source );
    }

    // drawLine
    {
           QLineF line(10.0, 80.0, 90.0, 20.0);

           gc.drawLine(line);
           gc2.drawLine( line );
    }

    // drawPainterPath
    {
        QPainterPath path;
        path.moveTo(20, 80);
        path.lineTo(20, 30);
        path.cubicTo(80, 0, 50, 50, 80, 80);


        gc.drawPath(path);
        gc2.drawPath( path );
    }

    // draw pie
    {
        QRectF rectangle(10.0, 20.0, 80.0, 60.0);
        int startAngle = 30 * 16;
        int spanAngle = 120 * 16;

        gc.drawPie(rectangle, startAngle, spanAngle);
        gc2.drawPie(rectangle, startAngle, spanAngle);
    }

    // draw pixmap
    {
         QRectF target(200.0, 200.0, 80.0, 60.0);
         QRectF source(0.0, 0.0, 70.0, 40.0);
         QPixmap pixmap(QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa.png");

         gc.drawPixmap(target, pixmap, source);
         gc2.drawPixmap(target, pixmap, source);
    }

    // draw point
    {
        QPointF pt( 50.0, 45.0 );
        gc.drawPoint( pt );
        gc2.drawPoint( pt );
    }

    // draw points
    {
        static const QPointF points[4] = {
            QPointF(10.0, 80.0),
            QPointF(20.0, 10.0),
            QPointF(80.0, 30.0),
            QPointF(90.0, 70.0)
        };


        gc.drawPoints(points, 4);
        gc2.drawPoints(points, 4);

    }

    // draw polygon
    {
        static const QPointF points[4] = {
            QPointF(10.0, 80.0),
            QPointF(20.0, 10.0),
            QPointF(80.0, 30.0),
            QPointF(90.0, 70.0)
        };

        gc.drawPolygon(points, 4);
        gc2.drawPolygon(points, 4);
    }

    // draw polyline
    {
         static const QPointF points[3] = {
             QPointF(10.0, 80.0),
             QPointF(20.0, 10.0),
             QPointF(80.0, 30.0),
         };

         gc.drawPolyline(points, 3);
         gc2.drawPolyline(points, 3);
    }

    // draw rect
    {
        QRectF rectangle(10.0, 300.0, 80.0, 60.0);

        gc.drawRect(rectangle);
        gc2.drawRect(rectangle);
    }

    // draw round rect
    {
           QRectF rectangle(10.0, 400.0, 80.0, 60.0);
           gc.drawRoundRect(rectangle);
           gc2.drawRoundRect(rectangle);
    }

    // draw text
    {
        QRectF rect ( 250, 250, 100, 10 );
        gc.drawText(rect, Qt::AlignCenter, tr("Krita"));
        gc2.drawText(rect, Qt::AlignCenter, tr("Krita "));
    }

    // draw Tiled pixmap
    {
        QRectF target(300.0, 300.0, 212, 212);
        QPixmap pixmap(QString(FILES_DATA_DIR) + QDir::separator() + "tile.png");

        gc.drawTiledPixmap( target, pixmap, QPointF( 0, 0 ) );
        gc2.drawTiledPixmap( target, pixmap,QPointF( 0, 0 ) );
    }

    gc.end();
    gc2.end();

    QImage result = dev->convertToQImage( 0, 0, 0, 512, 512 );

    QPoint errpoint;

    if ( !TestUtil::compareQImages( errpoint, img, result ) ) {
        img.save( "kis_paint_device_test_test_paintengine_qimage.png" );
        result.save( "kis_paint_device_test_test_paintengine_result.png" );
        QFAIL( QString( "Failed to create identical image, first different pixel: %1,%2 " ).arg( errpoint.x() ).arg( errpoint.y() ).toAscii() );
    }
}

void KisPaintDeviceTest::testStore()
{

    KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice( cs );

    KoStore * readStore =
        KoStore::createStore(QString(FILES_DATA_DIR) + QDir::separator() + "store_test.kra", KoStore::Read);
    readStore->open( "built image/layers/layer0" );
    QVERIFY( dev->read( readStore ) );
    readStore->close();
    delete readStore;

    QVERIFY( dev->exactBounds() == QRect( 0, 0, 100, 100 ) );

    KoStore * writeStore =
        KoStore::createStore(QString(FILES_DATA_DIR) + QDir::separator() + "store_test_out.kra", KoStore::Write);
    writeStore->open( "built image/layers/layer0" );
    QVERIFY( dev->write( writeStore ) );
    writeStore->close();
    delete writeStore;

    KisPaintDeviceSP dev2 = new KisPaintDevice( cs );
    readStore =
        KoStore::createStore(QString(FILES_DATA_DIR) + QDir::separator() + "store_test_out.kra", KoStore::Read);
    readStore->open( "built image/layers/layer0" );
    QVERIFY( dev2->read( readStore ) );
    readStore->close();
    delete readStore;

    QVERIFY( dev2->exactBounds() == QRect( 0, 0, 100, 100 ) );

    QPoint pt;
    if ( !TestUtil::comparePaintDevices( pt, dev, dev2 ) ) {
        QFAIL( QString( "Loading a saved image is not pixel perfect, first different pixel: %1,%2 " ).arg( pt.x() ).arg( pt.y() ).toAscii() );
    }

}

void KisPaintDeviceTest::testGeometry()
{
    KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice( cs );

    quint8* pixel = cs->allocPixelBuffer( 1 );
    cs->fromQColor( Qt::white, pixel );
    dev->fill( 0, 0, 512, 512, pixel);

    QVERIFY( dev->exactBounds() == QRect( 0, 0, 512, 512 ) );
    QVERIFY( dev->extent() == QRect( 0, 0, 512, 512 ) );

    dev->move( 10, 10 );

    QVERIFY( dev->exactBounds() == QRect( 10, 10, 512, 512 ) );
    QVERIFY( dev->extent() == QRect( 10, 10, 512, 512 ) );

    dev->crop( 50, 50, 50, 50 );
    QVERIFY( dev->exactBounds() == QRect( 50, 50, 50, 50 ) );
    QVERIFY( dev->extent() == QRect( 50, 50, 50, 50 ) );

    QColor c;
    quint8 opacity;

    dev->clear( QRect( 50, 50, 50, 50 ) );
    dev->pixel( 80, 80, &c, &opacity );
    QVERIFY( c == Qt::black );
    QVERIFY( opacity == OPACITY_TRANSPARENT );

    dev->fill( 0, 0, 512, 512, pixel);
    dev->pixel( 80, 80, &c, &opacity );
    QVERIFY( c == Qt::white );
    QVERIFY( opacity == OPACITY_OPAQUE );

    dev->clear();
    dev->pixel( 80, 80, &c, &opacity );
    QVERIFY( c == Qt::black );
    QVERIFY( opacity == OPACITY_TRANSPARENT );

    // XXX: No idea why we get this extent and bounds after a clear --
    // but I want to know as soon as possible if this behaviour
    // changes in any way.
    QVERIFY( dev->extent() == QRect( 74, 74, 64, 64 ) );
    QVERIFY( dev->exactBounds() == QRect( 74, 74, 64, 64 ) );

}

void KisPaintDeviceTest::testClear()
{
    KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice( cs );

    QVERIFY( dev->extent() == QRect(2147483647,2147483647, 0, 0) );
    QVERIFY( dev->exactBounds() == QRect(2147483647,2147483647, 0, 0) );

    dev->clear();

    QVERIFY( dev->extent() == QRect(2147483647,2147483647, 0, 0) );
    QVERIFY( dev->exactBounds() == QRect(2147483647,2147483647, 0, 0) );

    dev->clear( QRect( 100, 100, 100, 100 ) );

    // XXX: This is strange!
    QVERIFY( dev->extent() == QRect(64, 64, 192, 192 ) );
    QVERIFY( dev->exactBounds() == QRect(64, 64, 192, 192 ) );

    dev->clear();

    QVERIFY( dev->extent() == QRect(2147483647,2147483647, 0, 0) );
    QVERIFY( dev->exactBounds() == QRect(2147483647,2147483647, 0, 0) );

}

void KisPaintDeviceTest::testCrop()
{
    KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice( cs );
    quint8* pixel = cs->allocPixelBuffer( 1 );
    cs->fromQColor( Qt::white, pixel );
    dev->fill( -14, 8, 433, 512, pixel);

    QVERIFY( dev->exactBounds() == QRect( -14, 8, 433, 512 ) );

    // Crop inside
    dev->crop( 50, 50, 150, 150 );
    QVERIFY( dev->exactBounds() == QRect( 50, 50, 150, 150 ) );

    // Crop outside, pd should not grow
    dev->crop( 0, 0, 1000, 1000 );
    QVERIFY( dev->exactBounds() == QRect( 50, 50, 150, 150 ) );
}

void KisPaintDeviceTest::testRoundtripReadWrite()
{
    KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice( cs );
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "tile.png");
    dev->convertFromQImage( image, "");
    quint8* bytes = cs->allocPixelBuffer( image.width() * image.height() );
    memset( bytes, 0, image.width() * image.height() * dev->pixelSize() );
    dev->readBytes( bytes, image.rect() );

    KisPaintDeviceSP dev2 = new KisPaintDevice( cs );
    dev2->writeBytes( bytes, image.rect() );
    QVERIFY( dev2->exactBounds() == image.rect() );

    dev2->convertToQImage(0, 0, 0, image.width(), image.height()).save( "readwrite.png" );


    QPoint pt;
    if ( !TestUtil::comparePaintDevices( pt, dev, dev2 ) ) {
        QFAIL( QString( "Failed round trip using readBytes and writeBytes, first different pixel: %1,%2 " ).arg( pt.x() ).arg( pt.y() ).toAscii() );
    }
}

void logFailure( const QString & reason, const KoColorSpace * srcCs, const KoColorSpace * dstCs )
{
    QString profile1( "no profile" );
    QString profile2( "no profile" );
    if ( srcCs->profile() )
        profile1 = srcCs->profile()->name();
    if ( dstCs->profile() )
        profile2 = dstCs->profile()->name();

    QWARN(  QString("Failed %1 %2 -> %3 %4 %5" )
            .arg( srcCs->name() )
            .arg( profile1 )
            .arg( dstCs->name() )
            .arg( profile2 )
            .arg( reason )
            .toAscii() );
}

void KisPaintDeviceTest::testColorSpaceConversion()
{
    QTime t;
    t.start();

    QList<KoColorSpace*> colorSpaces = TestUtil::allColorSpaces();
    int failedColorSpaces = 0;

    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "tile.png");

    foreach ( KoColorSpace * srcCs, colorSpaces ) {
        foreach( KoColorSpace * dstCs,  colorSpaces ) {

            KisPaintDeviceSP dev  = new KisPaintDevice( srcCs );
            dev->convertFromQImage( image, "");
            dev->move( 10, 10 ); // Unalign with tile boundaries
            dev->convertTo( dstCs );

            if ( dev->exactBounds() != QRect( 10, 10, image.width(), image.height() ) ) {
                logFailure( "bounds", srcCs, dstCs );
                failedColorSpaces++;
            }
            if ( dev->pixelSize() != dstCs->pixelSize() ) {
                logFailure( "pixelsize", srcCs, dstCs );
                failedColorSpaces++;
            }
            if ( dev->colorSpace()->name() != dstCs->name() ) {
                logFailure( "dest cs", srcCs, dstCs );
                failedColorSpaces++;
            }
        }
    }
    qDebug() << colorSpaces.size() * colorSpaces.size()
             << "conversions"
             << " done in "
             << t.elapsed()
             << "ms";

    if ( failedColorSpaces > 0 ) {
        QFAIL( QString( "Failed conversions %1, see log for details." ).arg( failedColorSpaces ).toAscii() );
    }
}


void KisPaintDeviceTest::testRoundtripConversion()
{
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa.png");
    KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage( image, "" );
    QImage result = dev->convertToQImage(0, 0, 0, 640, 441);

    QPoint errpoint;

    if ( !TestUtil::compareQImages( errpoint, image, result ) ) {
        image.save( "kis_paint_device_test_test_roundtrip_qimage.png" );
        result.save( "kis_paint_device_test_test_roundtrip_result.png" );
        QFAIL( QString( "Failed to create identical image, first different pixel: %1,%2 \n" ).arg( errpoint.x() ).arg( errpoint.y() ).toAscii() );
    }
}

void KisPaintDeviceTest::testThumbnail()
{
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa.png");
    KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage( image, "" );
    {
        KisPaintDeviceSP thumb = dev->createThumbnailDevice( 50, 50 );
        QRect rc = thumb->exactBounds();
        QVERIFY( rc.width() <= 50 );
        QVERIFY( rc.height() <= 50 );
    }
    {
        QImage thumb = dev->createThumbnail( 50, 50 );
        QVERIFY( thumb.width() <= 50 );
        QVERIFY( thumb.height() <= 50 );
    }
}

void KisPaintDeviceTest::testPixel()
{
    KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    QColor c = Qt::red;
    quint8 opacity = 125;

    dev->setPixel( 5, 5, c, opacity );

    QColor c2;
    quint8 opacity2;

    dev->pixel( 5, 5, &c2, &opacity2 );

    QVERIFY( c == c2 );
    QVERIFY( opacity2 == opacity2 );

}


void KisPaintDeviceTest::testDirty()
{
    KoColorSpace * cs = KoColorSpaceRegistry::instance()->colorSpace("RGBA", 0);
    KisImageSP image = new KisImage(0, 512, 512, cs, "merge test");
    KisPaintLayerSP layer = new KisPaintLayer(image, "test", OPACITY_OPAQUE);
    KisPaintDeviceSP dev = layer->paintDevice();
    QVERIFY( dev != 0 );

    quint8* pixel = cs->allocPixelBuffer( 1 );
    cs->fromQColor( Qt::white, pixel );
    dev->fill( 0, 0, 512, 512, pixel);

    dev->setDirty( QRect( 10, 10, 10, 10 ) );
    QVERIFY( layer->isDirty() );
    QVERIFY( layer->isDirty( QRect( 12, 12, 20, 20 ) ) );
    QVERIFY( !layer->isDirty( QRect( 50, 50, 100, 100 ) ) );

    QRegion r;
    r += QRect( 30, 10, 10, 10 );
    r += QRect( 40, 10, 10, 10 );
    dev->setDirty( r );

    QVERIFY( layer->isDirty() );
    QVERIFY( layer->isDirty( QRect( 12, 12, 20, 20 ) ) );
    QVERIFY( layer->isDirty( QRect( 32, 12, 20, 20 ) ) );
    QVERIFY( layer->isDirty( QRect( 32, 12, 20, 20 ) ) );
    QVERIFY( !layer->isDirty( QRect( 100, 100, 100, 100 ) ) );
    layer->setClean();

    dev->setDirty();
    QVERIFY( layer->isDirty() );
    QVERIFY( !layer->isDirty( QRect( -10, -10, 5, 5 ) ) );
    QVERIFY( layer->isDirty( QRect( 0, 0, 512, 512 ) ) );
}

void KisPaintDeviceTest::testMirror()
{
    KoColorSpace * cs = KoColorSpaceRegistry::instance()->colorSpace("RGBA", 0);
    KisPaintDeviceSP dev = new KisPaintDevice( cs );


    quint8* pixel = cs->allocPixelBuffer( 1 );
    cs->fromQColor( Qt::white, pixel );
    dev->fill( 0, 0, 512, 512, pixel);

    cs->fromQColor( Qt::black, pixel );
    dev->fill( 512, 0, 512, 512, pixel );

    QColor c1;
    quint8 opacity1;
    dev->pixel( 5, 5, &c1, &opacity1 );

    QColor c2;
    quint8 opacity2;
    dev->pixel( 517, 5, &c2, &opacity2 );

    QVERIFY( c1 == Qt::white );
    QVERIFY( c2 == Qt::black );

    dev->mirrorX();

    dev->pixel( 5, 5, &c1, &opacity1 );
    dev->pixel( 517, 5, &c2, &opacity2 );

    QVERIFY( c1 == Qt::black );
    QVERIFY( c2 == Qt::white );

    cs->fromQColor( Qt::white, pixel );
    dev->fill( 0, 0, 512, 512, pixel);

    cs->fromQColor( Qt::black, pixel );
    dev->fill( 0, 512, 512, 512, pixel );

    dev->pixel( 5, 5, &c1, &opacity1 );
    dev->pixel( 5, 517, &c2, &opacity2 );

    QVERIFY( c1 == Qt::white );
    QVERIFY( c2 == Qt::black );

    dev->mirrorY();
    dev->convertToQImage(0, 0, 0, 1024, 512).save( "mirror.png" );

    dev->pixel( 5, 5, &c1, &opacity1 );
    dev->pixel( 5, 517, &c2, &opacity2 );

    QVERIFY( c1 == Qt::black );
    QVERIFY( c2 == Qt::white );

    {
        QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
        KisPaintDeviceSP dev2 = new KisPaintDevice( cs );
        dev2->convertFromQImage( image, "" );
        dev2->mirrorX();
        dev2->mirrorX();
        dev2->mirrorX();
        dev2->convertToQImage(0, 0, 0, image.width(), image.height()).save( "mirror_test2.png" );
    }
}

void KisPaintDeviceTest::testMirrorTransaction()
{
    KoColorSpace * cs = KoColorSpaceRegistry::instance()->colorSpace("RGBA", 0);
    KisPaintDeviceSP dev = new KisPaintDevice( cs );

    quint8* pixel = cs->allocPixelBuffer( 1 );
    cs->fromQColor( Qt::white, pixel );
    dev->fill( 0, 0, 512, 512, pixel);

    cs->fromQColor( Qt::black, pixel );
    dev->fill( 512, 0, 512, 512, pixel );

    QColor c1;
    quint8 opacity1;
    dev->pixel( 5, 5, &c1, &opacity1 );

    QColor c2;
    quint8 opacity2;
    dev->pixel( 517, 5, &c2, &opacity2 );

    QVERIFY( c1 == Qt::white );
    QVERIFY( c2 == Qt::black );
    dev->convertToQImage( 0, 0, 0, 1024, 512 ).save( "before.png" );

    KisTransaction t( "mirror", dev, 0 );
    dev->mirrorX();

    dev->pixel( 5, 5, &c1, &opacity1 );
    dev->pixel( 517, 5, &c2, &opacity2 );

    dev->convertToQImage( 0, 0, 0, 1024, 512 ).save( "mirror.png" );
    QVERIFY( c1 == Qt::black );
    QVERIFY( c2 == Qt::white );

    cs->fromQColor( Qt::white, pixel );
    dev->fill( 0, 0, 512, 512, pixel);

    cs->fromQColor( Qt::black, pixel );
    dev->fill( 0, 512, 512, 512, pixel );

    dev->pixel( 5, 5, &c1, &opacity1 );
    dev->pixel( 5, 517, &c2, &opacity2 );

    QVERIFY( c1 == Qt::white );
    QVERIFY( c2 == Qt::black );

    dev->mirrorY();

    dev->pixel( 5, 5, &c1, &opacity1 );
    dev->pixel( 5, 517, &c2, &opacity2 );

    QVERIFY( c1 == Qt::black );
    QVERIFY( c2 == Qt::white );

    {
        QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
        KisPaintDeviceSP dev2 = new KisPaintDevice( cs );
        dev2->convertFromQImage( image, "" );
        KisTransaction t( "mirror", dev2, 0 );
        dev2->mirrorX();
        dev2->convertToQImage(0, 0, 0, image.width(), image.height()).save( "mirror_test_t_2.png" );
    }
}

void KisPaintDeviceTest::testSelection()
{
    QFAIL( "Implement unittests for the selection API of KisPaintDevice" );
}

void KisPaintDeviceTest::testPlanarReadWrite()
{
    KoColorSpace * cs = KoColorSpaceRegistry::instance()->colorSpace("RGBA", 0);
    KisPaintDeviceSP dev = new KisPaintDevice( cs );

    quint8* pixel = cs->allocPixelBuffer( 1 );
    cs->fromQColor( QColor( 255, 200, 155), 100, pixel );
    dev->fill( 0, 0, 5000, 5000, pixel);

    QColor c1;
    quint8 opacity1;
    dev->pixel( 5, 5, &c1, &opacity1 );

    QVector<quint8*> planes = dev->readPlanarBytes( 500, 500, 100, 100 );
    QVector<quint8*> swappedPlanes;

    QCOMPARE( ( int )planes.size(), ( int )dev->channelCount() );

    for (int i = 0; i < 100*100; i++) {
        // BGRA encoded
        QVERIFY(planes.at(2)[i] == 255);
        QVERIFY(planes.at(1)[i] == 200);
        QVERIFY(planes.at(0)[i] == 155);
        QVERIFY(planes.at(3)[i] == 100);
    }

    for ( uint i = 1; i < dev->channelCount() + 1; ++i ) {
        swappedPlanes.append( planes[dev->channelCount() - i] );
    }

    dev->writePlanarBytes( swappedPlanes, 0, 0, 100, 100 );

    dev->convertToQImage(0, 0, 0, 5000, 5000).save( "planar.png" );

    dev->pixel( 5, 5, &c1, &opacity1 );

    QVERIFY( c1.red() == 200 );
    QVERIFY( c1.green() == 255 );
    QVERIFY( c1.blue() == 100);
    QVERIFY( opacity1 == 155 );

    dev->pixel( 75, 50, &c1, &opacity1 );

    QVERIFY( c1.red() == 200 );
    QVERIFY( c1.green() == 255 );
    QVERIFY( c1.blue() == 100);
    QVERIFY( opacity1 == 155 );
}

QTEST_KDEMAIN(KisPaintDeviceTest, GUI)
#include "kis_paint_device_test.moc"


