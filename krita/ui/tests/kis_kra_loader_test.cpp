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

#include <KisDocument.h>
#include <KoDocumentInfo.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>

#include "KisDocument.h"
#include "kis_image.h"
#include "testutil.h"
#include "KisPart.h"

#include <filter/kis_filter_registry.h>
#include <generator/kis_generator_registry.h>


void KisKraLoaderTest::initTestCase()
{
    KisFilterRegistry::instance();
    KisGeneratorRegistry::instance();
}

void KisKraLoaderTest::testLoading()
{
    KisDocument *doc = KisPart::instance()->createDocument();
    doc->loadNativeFormat(QString(FILES_DATA_DIR) + QDir::separator() + "load_test.kra");
    KisImageWSP image = doc->image();
    image->lock();
    QCOMPARE(image->nlayers(), 12);
    QCOMPARE(doc->documentInfo()->aboutInfo("title"), QString("test image for loading"));
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

    delete doc;
}



void KisKraLoaderTest::testObligeSingleChild()
{
    QString fileName = TestUtil::fetchDataFileLazy("single_layer_no_channel_flags.kra");

    KisDocument *doc = KisPart::instance()->createDocument();
    doc->loadNativeFormat(fileName);
    KisImageWSP image = doc->image();

    QVERIFY(image);
    QCOMPARE(image->nlayers(), 2);

    KisNodeSP root = image->root();
    KisNodeSP child = root->firstChild();

    QVERIFY(child);

    QCOMPARE(root->original(), root->projection());
    QCOMPARE(root->original(), child->projection());

    delete doc;
}


QTEST_KDEMAIN(KisKraLoaderTest, GUI)
#include "kis_kra_loader_test.moc"
