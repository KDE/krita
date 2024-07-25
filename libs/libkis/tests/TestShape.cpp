/* SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "TestShape.h"
#include <simpletest.h>

#include <KritaVersionWrapper.h>

#include <kis_image.h>
#include <kis_shape_layer.h>

#include <KisDocument.h>
#include <KisPart.h>

#include <testui.h>

#include "VectorLayer.h"
#include "GroupShape.h"


void TestShape::initTestCase()
{

    kisdoc = KisPart::instance()->createDocument();
    image = new KisImage(0, 100, 100, KoColorSpaceRegistry::instance()->rgb8(), "test");
    vLayer1 = new KisShapeLayer(kisdoc->shapeController(), image, "vLayer1", OPACITY_OPAQUE_U8);

    image->addNode(vLayer1);
    kisdoc->setCurrentImage(image);
    KisPart::instance()->addDocument(kisdoc, false);

    vNode = new VectorLayer(vLayer1);

    QList<Shape*> shapeList = vNode->addShapesFromSvg(R"quote(<?xml version="1.0" standalone="no"?>
        <!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 20010904//EN" "http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd">
        <!-- Created using Krita: https://krita.org -->
        <svg xmlns="http://www.w3.org/2000/svg"
            xmlns:xlink="http://www.w3.org/1999/xlink"
            xmlns:krita="http://krita.org/namespaces/svg/krita"
            xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
            width="307.2pt"
            height="307.2pt"
            viewBox="0 0 307.2 307.2">
        <defs/>
        <g id="group0">
        <text id="shape0" krita:useRichText="true" krita:textVersion="2" transform="matrix(0.999999960263572 0 0 0.999999960263572 12.921701441042 34.9859557016265)" fill="#0026ff" stroke-opacity="0" stroke="#000000" stroke-width="0" stroke-linecap="square" stroke-linejoin="bevel" font-stretch="normal" letter-spacing="0" word-spacing="0"><tspan x="0">Top Shape0</tspan></text>
        </g>
        </svg>)quote");
}

void TestShape::cleanupTestCase()
{
    delete vNode;

    kisdoc->setCurrentImage(0);
    KisPart::instance()->removeDocument(kisdoc);
}

void TestShape::testParentShape()
{
    QList<Shape*> groupShapeList = vNode->shapes();
    GroupShape *groupShape = qobject_cast<GroupShape*>(groupShapeList[0]);
    QList<Shape*> shapeList = groupShape->children();
    Shape *childShape = shapeList[0];

    QVERIFY(groupShape->name() == childShape->parentShape()->name());
}

KISTEST_MAIN(TestShape)
