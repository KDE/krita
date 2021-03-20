/*
 *  SPDX-FileCopyrightText: 2019 Tusooa Zhu <tusooa@vista.aero>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisDocumentReplaceTest.h"

#include <KisDocument.h>
#include <kis_image.h>
#include <kis_types.h>
#include <KisPart.h>
#include <kis_layer_utils.h>
#include <kis_group_layer.h>
#include <sdk/tests/testui.h>
#include <QScopedPointer>

void KisDocumentReplaceTest::init()
{
    m_doc = KisPart::instance()->createDocument();
    qDebug() << m_doc->newImage("test", 512, 512, KoColorSpaceRegistry::instance()->colorSpace("RGBA", "U8", 0), KoColor(), KisConfig::RASTER_LAYER, 1, "", 96);
}

void KisDocumentReplaceTest::finalize()
{
    delete m_doc;
    m_doc = 0;
}

void KisDocumentReplaceTest::testCopyFromDocument()
{
    init();
    QScopedPointer<KisDocument> clonedDoc(m_doc->lockAndCreateSnapshot());
    KisDocument *anotherDoc = KisPart::instance()->createDocument();
    anotherDoc->newImage("test", 512, 512, KoColorSpaceRegistry::instance()->colorSpace("RGBA", "U8", 0), KoColor(), KisConfig::RASTER_LAYER, 2, "", 96);
    KisImageSP anotherImage(anotherDoc->image());
    KisNodeSP root(anotherImage->rootLayer());
    anotherDoc->copyFromDocument(*(clonedDoc.data()));
    // image pointer should not change
    QCOMPARE(anotherImage.data(), anotherDoc->image().data());
    // root node should change
    QVERIFY(root.data() != anotherDoc->image()->rootLayer().data());
    // node count should be the same
    QList<KisNodeSP> oldNodes, newNodes;
    KisLayerUtils::recursiveApplyNodes(clonedDoc->image()->root(), [&oldNodes](KisNodeSP node) { oldNodes << node; });
    KisLayerUtils::recursiveApplyNodes(anotherDoc->image()->root(), [&newNodes](KisNodeSP node) { newNodes << node; });
    QCOMPARE(oldNodes.size(), newNodes.size());

    KisPart::instance()->removeDocument(anotherDoc);
    finalize();
}

KISTEST_MAIN(KisDocumentReplaceTest)
