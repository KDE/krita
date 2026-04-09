/*
 *  SPDX-FileCopyrightText: 2026 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "CutThroughShapeStrategyTest.h"

#include "CutThroughShapeStrategy.h"
#include "KisToolKnife.h"
#include <KoSelection.h>
#include <KoCanvasBase.h>
#include <tests/MockShapes.h>
#include <KoSvgTextShape.h>


#include <testutil.h>
#include "kistest.h"
#include <KoPathShape.h>
#include <KoShape.h>

#include <QPainterPath>
#include <QRectF>
#include <QLineF>
#include <QPolygonF>
#include <QSharedPointer>
#include <kis_algebra_2d.h>
#include <QRandomGenerator>

Q_DECLARE_METATYPE(QPainterPath)
Q_DECLARE_METATYPE(QSharedPointer<KoShape>)


void CutThroughShapeStrategyTest::addRandom() {

    QRandomGenerator gen;
    gen.seed(1024);
    int range = 1000;

    gen.bounded(range);


    QPainterPath shapeLeft;
    shapeLeft.addPolygon(QPolygonF({QPointF(gen.bounded(range), gen.bounded(range)), QPointF(gen.bounded(range), gen.bounded(range)), QPointF(gen.bounded(range), gen.bounded(range)), QPointF(gen.bounded(range), gen.bounded(range)), QPointF(gen.bounded(range), gen.bounded(range))}));

    QPainterPath shapeRight;
    shapeRight.addPolygon(QPolygonF({QPointF(300, 100), QPointF(800, 100), QPointF(800, 500), QPointF(400, 500), QPointF(300, 100)}));

    QRectF outlineRect = shapeLeft.boundingRect() | shapeRight.boundingRect();

    QLineF gapLine1 = QLineF(QPointF(0, 300), QPointF(150, 600));
    qreal width = 20;

    QList<QLineF> gapLines = KisAlgebra2D::getParallelLines(gapLine1, width/2);

    QPainterPath left, right;
    QRectF gapLineRect;
    QPolygonF gapLinePolygon;

    CutThroughShapeStrategy::initializeGapShapes(outlineRect, gapLines[0], gapLines[1], left, right, gapLineRect, gapLinePolygon);

    ENTER_FUNCTION() << ppVar(gapLineRect);

    QTest::addRow("check 2: cannot be outside of rect of the gap") << QSharedPointer<KoShape>(KoPathShape::createShapeFromPainterPath(shapeLeft)) << QPainterPath() << !gapLineRect.isEmpty()
                                             << gapLineRect << gapLine1 << gapLines[0] << gapLines[1] << gapLinePolygon  << false << false;



    QTest::addRow("check 3: cannot be outside of rect of the gap") << QSharedPointer<KoShape>(KoPathShape::createShapeFromPainterPath(shapeLeft)) << QPainterPath() << !gapLineRect.isEmpty()
                                             << gapLineRect << gapLine1 << gapLines[0] << gapLines[1] << gapLinePolygon  << false << false;
}

void CutThroughShapeStrategyTest::WillShapeBeCutTest_data()
{

    QTest::addColumn<QSharedPointer<KoShape>>("referenceShape");
    QTest::addColumn<QPainterPath>("srcOutline");
    QTest::addColumn<bool>("checkGapLineRect");
    QTest::addColumn<QRectF>("gapLineRect");
    QTest::addColumn<QLineF>("gapLine");
    QTest::addColumn<QLineF>("leftLine");
    QTest::addColumn<QLineF>("rightLine");
    QTest::addColumn<QPolygonF>("gapLinePolygon");
    QTest::addColumn<bool>("expectedGeneral");
    QTest::addColumn<bool>("expectedPrecise");

    //QTest::addRow() << referenceShape << srcOutline << leftOppositeRect << rightOppositeRect << checkGapLineRect << gapLineRect << gapLine << leftLine << rightLine << gapLinePolygon << expectedGeneral << expectedPrecise;

    QPainterPath path;
    path.moveTo(20, 40);
    path.lineTo(200, 400);
    path.lineTo(40, 50);


    QSharedPointer<KoShape> referenceShape(KoPathShape::createShapeFromPainterPath(path));

    //
    //QTest::addRow("a") << referenceShape << QPainterPath() << QRectF() << QRectF() << false << QRectF() << QLineF() << QLineF() << QLineF() << QPolygonF() << true << true;




    //
    KoSvgTextShape* text = new KoSvgTextShape();
    ENTER_FUNCTION() << text->outline() << text->outlineRect();
    text->insertText(0, "Test case 1");
    ENTER_FUNCTION() << text->outline() << text->outlineRect();
    QSharedPointer<KoShape> textP(text);
    QTest::addRow("check 1: cannot be text") << textP << QPainterPath() << false << QRectF() << QLineF() << QLineF() << QLineF() << QPolygonF() << false << false;


    QPainterPath shapeLeft;
    shapeLeft.addPolygon(QPolygonF({QPointF(100, 100), QPointF(200, 100), QPointF(300, 500), QPointF(100, 500), QPointF(100, 100)}));

    QPainterPath shapeRight;
    shapeRight.addPolygon(QPolygonF({QPointF(300, 100), QPointF(800, 100), QPointF(800, 500), QPointF(400, 500), QPointF(300, 100)}));

    QRectF outlineRect = shapeLeft.boundingRect() | shapeRight.boundingRect();

    QLineF gapLine1 = QLineF(QPointF(0, 300), QPointF(50, 900));
    qreal width = 20;

    QList<QLineF> gapLines1 = KisAlgebra2D::getParallelLines(gapLine1, width/2);

    QPainterPath left, right;
    QRectF gapLineRect;
    QPolygonF gapLinePolygon;

    CutThroughShapeStrategy::initializeGapShapes(outlineRect, gapLines1[0], gapLines1[1], left, right, gapLineRect, gapLinePolygon);

    ENTER_FUNCTION() << ppVar(gapLineRect);

    QTest::addRow("check 2 case 1: cannot be outside of rect of the gap") << QSharedPointer<KoShape>(KoPathShape::createShapeFromPainterPath(shapeLeft)) << shapeLeft << !gapLineRect.isEmpty()
                                             << gapLineRect << gapLine1 << gapLines1[0] << gapLines1[1] << gapLinePolygon  << false << false;



    QTest::addRow("check 2 case 2: cannot be outside of rect of the gap") << QSharedPointer<KoShape>(KoPathShape::createShapeFromPainterPath(shapeRight)) << shapeRight << !gapLineRect.isEmpty()
                                             << gapLineRect << gapLine1 << gapLines1[0] << gapLines1[1] << gapLinePolygon  << false << false;


    gapLine1 = QLineF(QPointF(0, 300), QPointF(150, 300));
    gapLines1 = KisAlgebra2D::getParallelLines(gapLine1, width/2);
    CutThroughShapeStrategy::initializeGapShapes(outlineRect, gapLines1[0], gapLines1[1], left, right, gapLineRect, gapLinePolygon);


    QTest::addRow("check 3: cannot have exactly one line point within the shape") << QSharedPointer<KoShape>(KoPathShape::createShapeFromPainterPath(shapeLeft)) << shapeLeft << !gapLineRect.isEmpty()
                                             << gapLineRect << gapLine1 << gapLines1[0] << gapLines1[1] << gapLinePolygon  << true << false;




    gapLine1 = QLineF(QPointF(150, 300), QPointF(200, 300));
    gapLines1 = KisAlgebra2D::getParallelLines(gapLine1, width/2);
    CutThroughShapeStrategy::initializeGapShapes(outlineRect, gapLines1[0], gapLines1[1], left, right, gapLineRect, gapLinePolygon);


    QTest::addRow("check 4a: can have two points within the shape") << QSharedPointer<KoShape>(KoPathShape::createShapeFromPainterPath(shapeLeft)) << shapeLeft << !gapLineRect.isEmpty()
                                             << gapLineRect << gapLine1 << gapLines1[0] << gapLines1[1] << gapLinePolygon  << true << true;


    gapLine1 = QLineF(QPointF(150, 300), QPointF(200, 300));
    gapLines1 = KisAlgebra2D::getParallelLines(gapLine1, width/2);
    CutThroughShapeStrategy::initializeGapShapes(outlineRect, gapLines1[0], gapLines1[1], left, right, gapLineRect, gapLinePolygon);


    QTest::addRow("check 4b: can cross either gap line") << QSharedPointer<KoShape>(KoPathShape::createShapeFromPainterPath(shapeLeft)) << shapeLeft << !gapLineRect.isEmpty()
                                             << gapLineRect << gapLine1 << gapLines1[0] << gapLines1[1] << gapLinePolygon  << true << true;


    gapLine1 = QLineF(QPointF(80, 80), QPointF(310, 510));
    gapLines1 = KisAlgebra2D::getParallelLines(gapLine1, 200);
    CutThroughShapeStrategy::initializeGapShapes(outlineRect, gapLines1[0], gapLines1[1], left, right, gapLineRect, gapLinePolygon);

    ENTER_FUNCTION() << ppVar(gapLinePolygon);


    QTest::addRow("check 4c: can be inside of the gap") << QSharedPointer<KoShape>(KoPathShape::createShapeFromPainterPath(shapeLeft)) << shapeLeft << !gapLineRect.isEmpty()
                                             << gapLineRect << gapLine1 << gapLines1[0] << gapLines1[1] << gapLinePolygon  << true << true;





}



void CutThroughShapeStrategyTest::WillShapeBeCutTest()
{

    QFETCH(QSharedPointer<KoShape>, referenceShape);
    QFETCH(QPainterPath, srcOutline);
    QFETCH(bool, checkGapLineRect);
    QFETCH(QRectF, gapLineRect);
    QFETCH(QLineF, gapLine);
    QFETCH(QLineF, leftLine);
    QFETCH(QLineF, rightLine);
    QFETCH(QPolygonF, gapLinePolygon);
    QFETCH(bool, expectedGeneral);
    QFETCH(bool, expectedPrecise);

    ENTER_FUNCTION() << referenceShape->absoluteOutlineRect();


    //KoSelection koSelection;
    //KoCanvasBase koCanvasBase;
    //MockCanvas mockCanvas;
    //KisToolKnife toolKnife = KisToolKnife(&mockCanvas);
    //CutThroughShapeStrategy strategy(&toolKnife, &koSelection, {}, QPointF(), GutterWidthsConfig(KoUnit(), 1.0, 1.0, 1.0));
    //ENTER_FUNCTION() << strategy.decorationThickness();

    bool resultGeneral = CutThroughShapeStrategy::willShapeBeCutGeneral(referenceShape.data(), srcOutline, checkGapLineRect, gapLineRect);
    bool resultPrecise = CutThroughShapeStrategy::willShapeBeCutPrecise(srcOutline, gapLine, leftLine, rightLine, gapLinePolygon);

    QCOMPARE(resultGeneral, expectedGeneral);
    QCOMPARE(resultPrecise, expectedPrecise);



}

KISTEST_MAIN(CutThroughShapeStrategyTest)
