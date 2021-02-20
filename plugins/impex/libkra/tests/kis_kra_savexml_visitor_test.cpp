/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_kra_savexml_visitor_test.h"

#include <QTest>

#include <QBitArray>
#include <QDomDocument>

#include <KisDocument.h>
#include <KoDocumentInfo.h>
#include <KoShapeContainer.h>
#include <KoPathShape.h>

#include "kis_count_visitor.h"
#include "filter/kis_filter_registry.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter.h"
#include "kis_image.h"
#include "kis_pixel_selection.h"
#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include "kis_clone_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_shape_layer.h"
#include "kis_filter_mask.h"
#include "kis_transparency_mask.h"
#include "kis_selection_mask.h"
#include "kis_selection.h"
#include "kis_fill_painter.h"
#include "kis_shape_selection.h"
#include "util.h"

#include "../kis_kra_savexml_visitor.h"

#include <generator/kis_generator_registry.h>

void KisKraSaveXmlVisitorTest::initTestCase()
{
    KisFilterRegistry::instance();
    KisGeneratorRegistry::instance();
}


void KisKraSaveXmlVisitorTest::testCreateDomDocument()
{
    KisDocument* doc = createCompleteDocument();

    quint32 count = 0;

    QDomDocument dom;
    QDomElement image = dom.createElement("IMAGE"); // Legacy!
    KisSaveXmlVisitor visitor(dom, image, count, "", true);

    Q_ASSERT(doc->image());

    QStringList list;

    doc->image()->lock();
    
    KisCountVisitor cv(list, KoProperties());
    doc->image()->rootLayer()->accept(cv);

    doc->image()->rootLayer()->accept(visitor);

    QCOMPARE((int)visitor.m_count, (int)cv.count());

    //delete doc;
}

QTEST_MAIN(KisKraSaveXmlVisitorTest)
