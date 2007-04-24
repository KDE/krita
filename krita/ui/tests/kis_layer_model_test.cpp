 /*
 * Copyright (C) Adrian Page <adrian@pagenet.plus.com>, (C) 2007
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

#include <QTest>
#include <QCoreApplication>

#include <qtest_kde.h>
#include <kactioncollection.h>
#include <kdebug.h>

#include "kis_image.h"
#include "kis_types.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "kis_layer_model.h"
#include "KoColorSpace.h"
#include "KoCompositeOp.h"
#include "KoColorSpaceRegistry.h"

#include "kis_layer_model_test.h"

void kislayermodel_test::testRowcount()
{
    KisImage img( 0, 100, 100,  KoColorSpaceRegistry::instance()->rgb8(), "testimage" );

    KisLayerModel model(0);
    model.setImage( &img );

    KisLayerSP l1 = img.newLayer("first", 200, COMPOSITE_OVER, img.colorSpace());
    QVERIFY( model.rowCount() == 1 );

    KisLayerSP l2 = img.newLayer("second", 200, COMPOSITE_OVER, img.colorSpace());
    QVERIFY( model.rowCount() == 2 );

    KisGroupLayerSP parent = new KisGroupLayer(&img, "group 1", 200);
    img.addLayer( parent, img.rootLayer() );

    QVERIFY( model.rowCount() == 3 );

    KisPaintLayerSP child = new KisPaintLayer(&img, "child", 200);
    img.addLayer( child, parent );

    QVERIFY( model.rowCount() == 3 );

    // Is placed on top of the stack, internal idx = 2, qt idx = 0
    QModelIndex idx = model.index( 0, 0 );
    QVERIFY( idx.isValid() );
    QVERIFY( !idx.parent().isValid() );
    QVERIFY( model.hasChildren( idx ) );
    QVERIFY( model.rowCount(idx) == 1 );
}

void kislayermodel_test::testModelIndex()
{
    KisImage img( 0, 100, 100,  KoColorSpaceRegistry::instance()->rgb8(), "testimage" );

    KisLayerModel model(0);
    model.setImage( &img );

    KisLayerSP first = img.newLayer("first", 200, COMPOSITE_OVER, img.colorSpace());
    QModelIndex idx = model.index( 0, 0 );

    QVERIFY( idx.isValid() );
    QVERIFY( !idx.parent().isValid() );
    QVERIFY( idx.internalPointer() == first.data() );
    QVERIFY( model.hasChildren( idx ) == false );

    KisLayerSP second = img.newLayer( "second", 200, COMPOSITE_OVER, img.colorSpace() );
    QModelIndex idx2 = model.index( 1, 0 );

    QVERIFY( idx2.isValid() );
    QVERIFY( !idx2.parent().isValid() );
    QVERIFY( idx2.internalPointer() == second.data() );
    QVERIFY( idx2.sibling(0, 0) == idx );
    QVERIFY( idx.sibling(1, 0) == idx2 );
    QVERIFY( idx.model() == &model );

    KisGroupLayerSP parent = new KisGroupLayer (&img, "group 1", 200);
    img.addLayer( parent, img.rootLayer() );

    KisPaintLayerSP child = new KisPaintLayer(&img, "child", 200);
    img.addLayer( child, parent );

    QModelIndex parentIdx = model.index( 0, 0 );

    QVERIFY( parentIdx.isValid() );
    QVERIFY( !parentIdx.parent().isValid() );
    QVERIFY( parentIdx.internalPointer() == parent.data() );
    QVERIFY( model.hasChildren( parentIdx ) );

    QModelIndex childIdx = parentIdx.child( 0, 0 );
    QVERIFY( childIdx.isValid() );
    QVERIFY( childIdx.parent().isValid() );
    QVERIFY( childIdx.parent() == parentIdx );
    QVERIFY( childIdx.internalPointer() == child.data() );
    QVERIFY( childIdx.parent().internalPointer() == parent.data() );

    QModelIndex childIdx2 = model.index( 0, 0, parentIdx );
    QVERIFY( childIdx2.isValid() );
    QVERIFY( childIdx2.parent().isValid() );
    QVERIFY( childIdx2.parent() == parentIdx );
    QVERIFY( childIdx2.internalPointer() == child.data() );
    QVERIFY( childIdx2.parent().internalPointer() == parent.data() );

}

void kislayermodel_test::testGroupLayers()
{
    KisImage img( 0, 100, 100,  KoColorSpaceRegistry::instance()->rgb8(), "testimage" );

    KisLayerModel model(0);
    model.setImage( &img );

    KisLayerSP first = img.newLayer("first", 200, COMPOSITE_OVER, img.colorSpace());
    KisLayerSP second = img.newLayer( "second", 200, COMPOSITE_OVER, img.colorSpace() );


    KisGroupLayerSP parent = new KisGroupLayer (&img, "group 1", 200);
    img.addLayer( parent, img.rootLayer() );

    KisPaintLayerSP child = new KisPaintLayer(&img, "child", 200);
    img.addLayer( child, parent );

    QModelIndex parentIdx = model.index( 0, 0 );
    QVERIFY( model.hasChildren( parentIdx ) );

    QModelIndex childIdx2 = model.index( 0, 0, parentIdx );
    QVERIFY( childIdx2.isValid() );
    QVERIFY( childIdx2.parent().isValid() );
    QVERIFY( childIdx2.parent() == parentIdx );
    QVERIFY( childIdx2.internalPointer() == child.data() );
    QVERIFY( childIdx2.parent().internalPointer() == parent.data() );

}

QTEST_KDEMAIN(kislayermodel_test, GUI)

#include "kis_layer_model_test.moc"

