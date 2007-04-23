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

void kislayermodel_test::testModel()
{
    KisImage img( 0, 100, 100,  KoColorSpaceRegistry::instance()->rgb8(), "testimage" );
    KisLayerModel model(0);
    model.setImage( &img );

    img.newLayer("first", 200, COMPOSITE_OVER, img.colorSpace());
    QVERIFY( model.rowCount() == 1 );
    img.newLayer("second", 200, COMPOSITE_OVER, img.colorSpace());
    QVERIFY( model.rowCount() == 2 );

    QModelIndex idx = model.index( 0, 0 );
    kDebug() << img.rootLayer().data() << ", " << idx.internalPointer() << endl;
    kDebug() << idx.model() << ", " << idx.row() << ", " << idx.column() << endl;
    QVERIFY( idx.isValid() );
    QVERIFY( !idx.parent().isValid() );
}

QTEST_KDEMAIN(kislayermodel_test, GUI)

#include "kis_layer_model_test.moc"

