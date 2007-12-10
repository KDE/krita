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

#include "kis_merge_visitor_test.h"
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_types.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_filter.h"
#include "kis_filter_configuration.h"
#include "kis_filter_registry.h"
#include "kis_merge_visitor.h"
#include "kis_fill_painter.h"
#include "kis_filter_mask.h"
#include "kis_transparency_mask.h"
#include "kis_transformation_mask.h"

#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "kis_clone_layer.h"
#include "kis_adjustment_layer.h"

#include "testutil.h"

void KisMergeVisitorTest::initTestCase()
{
    original = QImage( QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa.png");
    inverted = QImage( QString(FILES_DATA_DIR) + QDir::separator() + "inverted_hakonepa.png" );
    colorSpace = KoColorSpaceRegistry::instance()->colorSpace("RGBA", 0);
    image = new KisImage(0, original.width(), original.height(), colorSpace, "merge test");
    image->lock(); // We don't want the automatic recomposition to kick in
    
}

void KisMergeVisitorTest::testMergePreview()
{
    KisPaintLayerSP layer = new KisPaintLayer(image, "test", OPACITY_OPAQUE);
    layer->paintDevice()->convertFromQImage( original, 0, 0, 0  );

    KisFilterMaskSP mask = new KisFilterMask();
    KisFilterSP f = KisFilterRegistry::instance()->value("invert");
    Q_ASSERT( f );
    KisFilterConfiguration * kfc = f->defaultConfiguration(0);
    Q_ASSERT( kfc );
    mask->setFilter( kfc );
    mask->select( original.rect() );
    layer->setPreviewMask( mask );
    layer->setDirty( original.rect() );
    
    KisPaintDeviceSP projection = new KisPaintDevice(colorSpace);
    KisMergeVisitor v( projection, original.rect() );
    layer->accept( v );
    
    QPoint errpoint;
    if ( !TestUtil::compareQImages( errpoint,
                                    inverted,
                                    projection->convertToQImage(0, 0, 0, original.width(), original.height() ) ) ) {

        projection->convertToQImage(0, 0, 0, original.width(), original.height() ).save("merge_visitor1.png");
        QFAIL( QString( "Failed to create identical image, first different pixel: %1,%2 " )
            .arg( errpoint.x() )
            .arg( errpoint.y() )
            .toAscii() );
    }
    delete kfc;
    
}
void KisMergeVisitorTest::testMergePreviewTwice()
{
    KisPaintLayerSP layer = new KisPaintLayer(image, "test", OPACITY_OPAQUE);
    layer->paintDevice()->convertFromQImage( original, 0, 0, 0  );

    KisFilterMaskSP mask = new KisFilterMask();
    KisFilterSP f = KisFilterRegistry::instance()->value("invert");
    Q_ASSERT( f );
    KisFilterConfiguration * kfc = f->defaultConfiguration(0);
    Q_ASSERT( kfc );
    mask->setFilter( kfc );
    mask->select( original.rect() );
    layer->setPreviewMask( mask );
    layer->setDirty( original.rect() );
    KisPaintDeviceSP projection = new KisPaintDevice(colorSpace);
    KisMergeVisitor v( projection, original.rect() );
    layer->accept( v );
    layer->accept( v );
    
    QPoint errpoint;
    if ( !TestUtil::compareQImages( errpoint,
                                    inverted,
                                    projection->convertToQImage(0, 0, 0, original.width(), original.height() ) ) ) {

        projection->convertToQImage(0, 0, 0, original.width(), original.height() ).save("merge_visitor2.png");
        QFAIL( QString( "Failed to create identical image, first different pixel: %1,%2 " )
            .arg( errpoint.x() )
            .arg( errpoint.y() )
            .toAscii() );
    }
    delete kfc;
}

void KisMergeVisitorTest::visitPaintLayer()
{
    KisPaintLayerSP layer = new KisPaintLayer(image, "test", OPACITY_OPAQUE);
    layer->paintDevice()->convertFromQImage( original, 0, 0, 0  );
    KisPaintDeviceSP projection = new KisPaintDevice(colorSpace);
    KisMergeVisitor v( projection, original.rect() );
    layer->accept( v );
    QPoint errpoint;
    if (!TestUtil::comparePaintDevices(errpoint, projection, layer->paintDevice())) {
        projection->convertToQImage(0, 0, 0, original.width(), original.height() ).save("merge_visitor3.png");
        QFAIL( QString( "Failed to merge layer onto projection, first different pixel: %1,%2 " )
            .arg( errpoint.x() )
            .arg( errpoint.y() )
            .toAscii() );    
    }

}

void KisMergeVisitorTest::visitGroupLayerWithOnlyPaintLayers()
{
    
    KisPaintLayerSP layer1 = new KisPaintLayer(image, "layer 1", OPACITY_OPAQUE);
    layer1->paintDevice()->convertFromQImage( original, 0, 0, 0  );
    layer1->setCompositeOp(colorSpace->compositeOp(COMPOSITE_OVER));
    
    KisPaintLayerSP layer2 = new KisPaintLayer(image, "layer 2", 128);
    KoColor c(Qt::red, colorSpace);
    KisFillPainter gc(layer2->paintDevice());
    gc.fillRect(original.rect(), c);
    gc.end();
    layer2->setCompositeOp(colorSpace->compositeOp(COMPOSITE_OVER));

    KisGroupLayerSP group = new KisGroupLayer(image, "test", OPACITY_OPAQUE);
    image->addNode(group.data());
    image->addNode(layer1.data(), group.data());
    image->addNode(layer2.data(), group.data(), layer1.data());
    
    group->setDirty(original.rect());
    KisPaintDeviceSP projection = new KisPaintDevice(colorSpace);
    KisMergeVisitor v( projection, original.rect() );
    group->accept( v );

    QImage result = group->projection()->convertToQImage(0, 0, 0, original.width(), original.height());
    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, result, QImage(QString(FILES_DATA_DIR) + QDir::separator() + "merge_visitor4.png"))) {
        result.save("merge_visitor4.png");
        QFAIL( QString( "Failed to merge two layers in one group, first different pixel: %1,%2 " )
            .arg( errpoint.x() )
            .arg( errpoint.y() )
            .toAscii() );    
    }
}

void KisMergeVisitorTest::visitGroupLayerWithAnAdjustmentLayer()
{
}

void KisMergeVisitorTest::visitAdjustmentLayer()
{
    KisFilterSP f = KisFilterRegistry::instance()->value("invert");
    KisFilterConfiguration * kfc = f->defaultConfiguration(0);
    KisSelectionSP sel = new KisSelection();
    sel->getOrCreatePixelSelection()->invert(); // Make sure everything is selected
    sel->updateProjection();
    
    KisAdjustmentLayerSP adjust = new KisAdjustmentLayer(image, "test", kfc, sel);
    adjust->setDirty();
    
    // Prepare a projection
    KisPaintDeviceSP projection = new KisPaintDevice(colorSpace);
    projection->convertFromQImage(original, 0, 0, 0);

    KisMergeVisitor v( projection, original.rect() );
    adjust->accept( v );

    projection->convertToQImage(0, 0, 0, original.width(), original.height()).save("xxx.png");

    // Check that the projection is cached in the adjustment layer
}

void KisMergeVisitorTest::visitCloneLayer()
{
}

    

QTEST_KDEMAIN(KisMergeVisitorTest, NoGUI);
#include "kis_merge_visitor_test.moc"

