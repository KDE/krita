/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "TestSvgTextShape.h"

#include <kistest.h>
#include <KoSvgTextShape.h>
#include <KoShape.h>
#include <KoPathShape.h>
#include <KoShapeGroup.h>
#include <KoShapeManager.h>

#include <KoShapeRegistry.h>

#include <commands/KoSvgChangeTextContoursCommand.h>
#include <commands/KoSvgTextAddRemoveShapeCommands.h>
#include <commands/KoSvgTextFlipShapeContourTypeCommand.h>
#include <commands/KoSvgTextReorderShapeInsideCommand.h>
#include <commands/KoShapeSizeCommand.h>

// Quick function to get a rectangle.
KoShape *createRectangles() {
    KoShapeFactoryBase *base = KoShapeRegistry::instance()->get("RectangleShape");
    return base->createDefaultShape();
}

KoPathShape *createPolygon() {
    KoPathShape *p = new KoPathShape;
    p->moveTo(QPointF(0,0));
    p->lineTo(QPointF(0,100));
    p->lineTo(QPointF(100,100));
    p->lineTo(QPointF(100, 0));
    p->lineTo(QPointF(0,0));
    p->close();
    return p;
}

void TestSvgTextShape::testSetTextOnShape_data()
{
    QTest::addColumn<QTransform>("textTransform");
    QTest::addColumn<bool>("textHasParent");
    QTest::addColumn<int>("contourCount");
    QTest::addColumn<QTransform>("contourTransform");
    QTest::addColumn<bool>("contourHasParent");

    QTest::addRow("text and 1 contour") << QTransform() << false << 1 << QTransform() << false;
    QTest::addRow("text and 2 contours") << QTransform() << false << 2 << QTransform() << false;
    QTest::addRow("text and 1 contour + local transform") << QTransform() << false << 1 << QTransform::fromTranslate(20, 20) << false;
    QTest::addRow("text + local transform and 1 contour") << QTransform::fromTranslate(20, 20) << false << 1 << QTransform() << false;
    QTest::addRow("text + absolute transform and 1 contour") << QTransform() << true << 1 << QTransform() << false;
    QTest::addRow("text and 1 contour + absolute transform") << QTransform() << false << 1 << QTransform() << true;
}

void TestSvgTextShape::testSetTextOnShape()
{
    KoSvgTextShape *textShape = new KoSvgTextShape();
    textShape->insertText(0, "The quick brown fox jumps over the lazy dog.");
    QList<KoShape*> shapes;

    QFETCH(QTransform, textTransform);
    QFETCH(bool, textHasParent);
    QFETCH(int, contourCount);
    QFETCH(QTransform, contourTransform);
    QFETCH(bool, contourHasParent);

    KoShapeGroup *contourParentGroup = nullptr;
    if (contourHasParent) {
        contourParentGroup = new KoShapeGroup();
        contourParentGroup->setTransformation(QTransform::fromTranslate(10, 10));
    }

    textShape->setTransformation(textTransform);

    for (int i = 0; i < contourCount; i++) {
        KoPathShape *p = createPolygon();
        p->setTransformation(contourTransform);
        if (contourParentGroup) {
            p->setParent(contourParentGroup);
        }
        shapes.append(p);
    }

    bool inside = true;

    if (textHasParent) {
        KoShapeGroup *parentGroup = new KoShapeGroup();
        parentGroup->setTransformation(QTransform::fromTranslate(10, 10));
        textShape->setParent(parentGroup);
    }

    KUndo2Command *parentCommand = new KUndo2Command();

    QRectF textOutlineRect = textShape->outlineRect();
    QRectF textBoundingRect = textShape->boundingRect();

    QRectF shapeBoundingRect;
    QList<KoShape*> clonedShapes;
    Q_FOREACH(KoShape *shape, shapes) {
        shapeBoundingRect |= shape->boundingRect();
        KoShape *clone = shape->cloneShape();
        if (contourParentGroup) {
            clone->setParent(contourParentGroup);
        }
        clonedShapes.append(clone);
        new KoSvgTextAddShapeCommand(textShape, clone, inside, parentCommand);
    }

    parentCommand->redo();

    QVERIFY(textShape->boundingRect() == shapeBoundingRect);
    QVERIFY(textShape->boundingRect() != textBoundingRect);
    QVERIFY(textShape->outlineRect() != textOutlineRect);
    for (int i = 0; i< clonedShapes.size(); i++) {
        KoShape *contour = clonedShapes.at(i);
        KoShape *original = shapes.at(i);
        QVERIFY(contour->absoluteTransformation() == original->absoluteTransformation());
    }

    parentCommand->undo();
    QVERIFY(textShape->boundingRect() != shapeBoundingRect);
    QVERIFY(textShape->boundingRect() == textBoundingRect);
    QVERIFY(textShape->outlineRect() == textOutlineRect);

    for (int i = 0; i< clonedShapes.size(); i++) {
        KoShape *contour = clonedShapes.at(i);
        KoShape *original = shapes.at(i);
        QVERIFY(contour->parent() == original->parent());
        QVERIFY(contour->transformation() == original->transformation());
        QVERIFY(contour->absoluteTransformation() == original->absoluteTransformation());

    }

    // Don't understand how to test textAfter transform with contour transform???
}

void TestSvgTextShape::testRemoveShapeFromText()
{
    KoSvgTextShape *textShape = new KoSvgTextShape();
    textShape->insertText(0, "The quick brown fox jumps over the lazy dog.");
    QList<KoShape*> shapes;

    QTransform rotate;
    rotate.rotate(45);
    for (int i = 0; i < 3; i++) {
        KoShape *p = createPolygon();
        p->setTransformation(rotate*QTransform::fromTranslate(i*10, i*10));
        shapes.append(p);
    }
    textShape->setShapesInside(shapes);

    KUndo2Command *cmd = new KoSvgTextRemoveShapeCommand(textShape, shapes.last());

    cmd->redo();

    QVERIFY(!textShape->shapeInContours(shapes.last()));

    cmd->undo();

    QVERIFY(textShape->shapeInContours(shapes.last()));
}

void TestSvgTextShape::testSetSize_data()
{
    QTest::addColumn<QSizeF>("newSize");
    QTest::addColumn<int>("contourCount");
    QTest::addColumn<bool>("useParametric");

    QTest::addRow("smaller, 1 contour") << QSizeF(50, 50) << 1 << false;
    QTest::addRow("smaller, 2 contours") << QSizeF(50, 50) << 2 << false;
    QTest::addRow("bigger, 1 contour") << QSizeF(200, 200) << 1 << false;
    QTest::addRow("bigger, 2 contours") << QSizeF(200, 200) << 2 << false;
    QTest::addRow("smaller, 1 contour, parametric") << QSizeF(50, 50) << 1 << true;
    QTest::addRow("smaller, 2 contours, parametric") << QSizeF(50, 50) << 2 << true;
    QTest::addRow("bigger, 1 contour, parametric") << QSizeF(200, 200) << 1 << true;
    QTest::addRow("bigger, 2 contours, parametric") << QSizeF(200, 200) << 2 << true;
}

void TestSvgTextShape::testSetSize()
{
    QFETCH(QSizeF, newSize);
    QFETCH(int, contourCount);
    QFETCH(bool, useParametric);

    KoSvgTextShape *textShape = new KoSvgTextShape();
    textShape->insertText(0, "The quick brown fox jumps over the lazy dog.");
    QList<KoShape*> shapes;

    QTransform rotate;
    rotate.rotate(45);
    for (int i = 0; i < contourCount; i++) {
        KoShape *p = useParametric? createRectangles(): createPolygon();
        p->setTransformation(rotate*QTransform::fromTranslate(i*10, i*10));
        shapes.append(p);
    }

    textShape->setShapesInside(shapes);
    QList<KoShape*> tShapes = {textShape};

    QSizeF originalSize = textShape->size();
    KoShapeSizeCommand *cmd = new KoShapeSizeCommand(tShapes, {originalSize}, {newSize});

    cmd->redo();
    QVERIFY2(textShape->size() == newSize, QString("Sizes don't match, %1 x %2, expected %3 x %4")
             .arg(textShape->size().width()).arg(textShape->size().height())
             .arg(newSize.width()).arg(newSize.height()).toLatin1());

    cmd->undo();
    QVERIFY2(textShape->size() == originalSize, QString("Sizes don't match, %1 x %2, expected %3 x %4")
             .arg(textShape->size().width()).arg(textShape->size().height())
             .arg(originalSize.width()).arg(originalSize.height()).toLatin1());

}

void TestSvgTextShape::testToggleShapeType()
{
    KoSvgTextShape *textShape = new KoSvgTextShape();
    textShape->insertText(0, "The quick brown fox jumps over the lazy dog.");

    KoPathShape *shapeInside = createPolygon();
    KoPathShape *shapeSubtract = createPolygon();
    shapeSubtract->setTransformation(QTransform::fromTranslate(50, 50));
    textShape->addShapeContours({shapeInside}, true);
    textShape->addShapeContours({shapeSubtract}, false);
    KUndo2Command *parentCommand = new KUndo2Command();

    new KoSvgTextFlipShapeContourTypeCommand(textShape, shapeSubtract, parentCommand);
    new KoSvgTextFlipShapeContourTypeCommand(textShape, shapeInside, parentCommand);

    parentCommand->redo();

    QVERIFY(textShape->shapesInside().contains(shapeSubtract));
    QVERIFY(textShape->shapesSubtract().contains(shapeInside));

    parentCommand->undo();

    QVERIFY(textShape->shapesInside().contains(shapeInside));
    QVERIFY(textShape->shapesSubtract().contains(shapeSubtract));
}

void TestSvgTextShape::testReorderShapesInside_data()
{
    QTest::addColumn<int>("type");
    QTest::addColumn<int>("amount");

    QTest::addRow("BringToFront, 1 shape") << int(KoSvgTextReorderShapeInsideCommand::BringToFront) << 1;
    QTest::addRow("BringToFront, 2 shapes") << int(KoSvgTextReorderShapeInsideCommand::BringToFront) << 2;
    QTest::addRow("MoveEarlier, 1 shape") << int(KoSvgTextReorderShapeInsideCommand::MoveEarlier) << 1;
    QTest::addRow("MoveEarlier, 2 shapes") << int(KoSvgTextReorderShapeInsideCommand::MoveEarlier) << 2;
    QTest::addRow("MoveLater, 1 shape") << int(KoSvgTextReorderShapeInsideCommand::MoveLater) << 1;
    QTest::addRow("MoveLater, 2 shapes") << int(KoSvgTextReorderShapeInsideCommand::MoveLater) << 2;
    QTest::addRow("SendToBack, 1 shape") << int(KoSvgTextReorderShapeInsideCommand::SendToBack) << 1;
    QTest::addRow("SendToBack, 2 shapes") << int(KoSvgTextReorderShapeInsideCommand::SendToBack) << 2;
}

void TestSvgTextShape::testReorderShapesInside()
{
    QFETCH(int, type);
    QFETCH(int, amount);
    KoSvgTextShape *textShape = new KoSvgTextShape();
    textShape->insertText(0, "The quick brown fox jumps over the lazy dog.");
    QList<KoShape*> shapes;
    for (int i = 0; i < 5; i++) {
        KoPathShape *p = createPolygon();
        p->setTransformation(QTransform::fromTranslate(i*10, i*10));
        shapes.append(p);
    }

    textShape->setShapesInside(shapes);

    KoSvgTextReorderShapeInsideCommand::MoveShapeType moveShapeType = KoSvgTextReorderShapeInsideCommand::MoveShapeType (type);

    QList<KoShape*> targets;
    KUndo2Command *parentCommand = new KUndo2Command();
    for (int i = 2; i < qMin(2+amount, shapes.size()); i++) {
        KoShape *target = shapes.at(i);
        targets.append(target);
    }

    new KoSvgTextReorderShapeInsideCommand(textShape, targets, moveShapeType, parentCommand);
    parentCommand->redo();

    int start = 0;
    switch(moveShapeType) {
    case KoSvgTextReorderShapeInsideCommand::BringToFront:
        start = 0;
        break;
    case KoSvgTextReorderShapeInsideCommand::MoveEarlier:
        start = 1;
        break;
    case KoSvgTextReorderShapeInsideCommand::MoveLater:
        start = 3;
        break;
    case KoSvgTextReorderShapeInsideCommand::SendToBack:
        start = (shapes.size()-(amount));
        break;
    }
    Q_FOREACH(KoShape *target, targets) {
        const int final = textShape->shapesInside().indexOf(target);
        QVERIFY2(final == start, QString("Expected: %1, got: %2").arg(start).arg(final).toLatin1());
        start +=1;
    }

    parentCommand->undo();

    int startOld = 2;
    Q_FOREACH(KoShape *target, targets) {
        const int final = textShape->shapesInside().indexOf(target);
        QVERIFY2(final == startOld, QString("Expected: %1, got: %2").arg(startOld).arg(final).toLatin1());
        startOld+=1;
    }
}

KISTEST_MAIN(TestSvgTextShape)
