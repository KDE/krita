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

#include "kis_kra_loader_test.h"

#include <qtest_kde.h>

#include <KoDocument.h>
#include <KoDocumentInfo.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>

#include "kis_doc2.h"
#include "kis_image.h"

void KisKraLoaderTest::testLoading()
{
    KisDoc2 doc;
    doc.loadNativeFormat(QString(FILES_DATA_DIR) + QDir::separator() + "load_test.kra");
    KisImageWSP image = doc.image();
    image->lock();
    QCOMPARE(image->nlayers(), 12);
    QCOMPARE(doc.documentInfo()->aboutInfo("title"), QString("test image for loading"));
    QCOMPARE(image->height(), 753);
    QCOMPARE(image->width(), 1000);
    QCOMPARE(image->colorSpace()->id(), KoColorSpaceRegistry::instance()->rgb8()->id());

    KisNodeSP node = image->root()->firstChild();
    QVERIFY(node);
    QCOMPARE(node->name(), QString("Background"));
    QVERIFY(node->inherits("KisPaintLayer"));

    node = node->nextSibling();
    QVERIFY(node);
    QCOMPARE(node->name(), QString("Group 1"));
    QVERIFY(node->inherits("KisGroupLayer"));
    QCOMPARE((int) node->childCount(), 2);

}

QTEST_KDEMAIN(KisKraLoaderTest, GUI)
#include "kis_kra_loader_test.moc"
