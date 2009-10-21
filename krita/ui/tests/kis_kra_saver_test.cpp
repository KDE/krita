/*
 *  Copyright (c) 2007 Boudewijn Rempt boud@valdyas.org
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

#include "kis_kra_saver_test.h"

#include <qtest_kde.h>

#include <QBitArray>

#include <KoDocument.h>
#include <KoDocumentInfo.h>
#include <KoColorSpaceRegistry.h>
#include <KoShapeContainer.h>
#include <KoColorSpace.h>
#include <KoPathShape.h>

#include "filter/kis_filter_registry.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter.h"
#include "kis_doc2.h"
#include "kis_image.h"
#include "kis_pixel_selection.h"
#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include "kis_clone_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_shape_layer.h"
#include "kis_filter_mask.h"
#include "kis_transparency_mask.h"
#include "kis_transformation_mask.h"
#include "kis_selection_mask.h"
#include "kis_selection.h"
#include "kis_fill_painter.h"
#include "kis_shape_selection.h"
#include "util.h"

void KisKraSaverTest::testRoundTrip()
{
    KisDoc2* doc = createCompleteDocument();
    doc->saveNativeFormat("roundtriptest.kra");
    QStringList list;
    KisCountVisitor cv1(list, KoProperties());
    doc->image()->rootLayer()->accept(cv1);

    delete doc;

    KisDoc2 doc2;
    doc2.loadNativeFormat("roundtriptest.kra");

    KisCountVisitor cv2(list, KoProperties());
    doc2.image()->rootLayer()->accept(cv2);
    QCOMPARE(cv1.count(), cv2.count());
}

QTEST_KDEMAIN(KisKraSaverTest, GUI)
#include "kis_kra_saver_test.moc"
