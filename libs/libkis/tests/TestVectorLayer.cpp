/* SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "TestVectorLayer.h"
#include <simpletest.h>


#include <KritaVersionWrapper.h>


#include <kis_image.h>
#include <kis_shape_layer.h>

#include <KisDocument.h>
#include <KisPart.h>

#include <testui.h>

#include "VectorLayer.h"
#include "Shape.h"



void TestVectorLayer::initTestCase()
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
        <text id="shape0" krita:useRichText="true" krita:textVersion="2" transform="matrix(0.999999960263572 0 0 0.999999960263572 12.921701441042 34.9859557016265)" fill="#0026ff" stroke-opacity="0" stroke="#000000" stroke-width="0" stroke-linecap="square" stroke-linejoin="bevel" font-stretch="normal" letter-spacing="0" word-spacing="0"><tspan x="0">Top Shape0</tspan></text>
        <text id="shape1" krita:useRichText="true" krita:textVersion="2" transform="matrix(0.999999867428964 0 0 0.999999867428964 239.90986885015 35.1714852651231)" fill="#0026ff" stroke-opacity="0" stroke="#000000" stroke-width="0" stroke-linecap="square" stroke-linejoin="bevel" font-stretch="normal" letter-spacing="0" word-spacing="0"><tspan x="0">Top Shape1</tspan></text>
        <text id="shape2" krita:useRichText="true" krita:textVersion="2" transform="matrix(0.999999762283568 0 0 0.999999762283568 36.7133523692404 203.118701819228)" fill="#0026ff" stroke-opacity="0" stroke="#000000" stroke-width="0" stroke-linecap="square" stroke-linejoin="bevel" font-stretch="normal" letter-spacing="0" word-spacing="0"><tspan x="0">Bottom Shape2</tspan></text>
        <text id="shape3" krita:useRichText="true" krita:textVersion="2" transform="matrix(0.999999698132009 0 0 0.999999698132009 214.531891414957 201.125497598433)" fill="#0026ff" stroke-opacity="0" stroke="#000000" stroke-width="0" stroke-linecap="square" stroke-linejoin="bevel" font-stretch="normal" letter-spacing="0" word-spacing="0"><tspan x="0">Bottom Shape3</tspan></text>
        </svg>)quote");
}

void TestVectorLayer::cleanupTestCase()
{
    delete vNode;

    kisdoc->setCurrentImage(0);
    KisPart::instance()->removeDocument(kisdoc);
}

void TestVectorLayer::testAddShapesFromSvg()
{
    QList<Shape*> shapeList = vNode->shapes();

    {
        QStringList test = {"shape0", "shape1", "shape2", "shape3"};

        QVERIFY(test.size() == shapeList.size());
        for (Shape* shape : shapeList) {
            QVERIFY(test.contains(shape->name()) == true);
            delete shape;
        }
    }
}

void TestVectorLayer::testShapeAtPosition()
{
    Shape *shape0 = vNode->shapeAtPosition(QPointF(20,30));

    QVERIFY(shape0->name() == "shape0");

    Shape *shape1 = vNode->shapeAtPosition(QPointF(209,300));

    QVERIFY(!shape1 == true);
}

void TestVectorLayer::testShapesInRect()
{
    QList<Shape*> shapeList = vNode->shapesInRect(QRectF(0,0,200,200));

    {
        QStringList test = {"shape0", "shape2"};

        QVERIFY(test.size() == shapeList.size());
        for (Shape* shape : shapeList) {
            QVERIFY(test.contains(shape->name()) == true);
            delete shape;
        }
    }
}

void TestVectorLayer::testCreateGroupShape()
{
    QList<Shape*> shapeList = vNode->shapes();
    Shape *groupshape = vNode->createGroupShape("group1",shapeList);

    QVERIFY(groupshape->name() == "group1");

    QList<Shape*> topLevelShapeList = vNode->shapes();
    {
        QStringList test = {"group1"};

        QVERIFY(test.size() == topLevelShapeList.size());
        for (Shape* shape : topLevelShapeList) {
            QVERIFY(test.contains(shape->name()) == true);
        }
    }
}

KISTEST_MAIN(TestVectorLayer)
