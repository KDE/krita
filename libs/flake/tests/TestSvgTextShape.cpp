/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "TestSvgTextShape.h"

#include <kis_algebra_2d.h>
#include <kis_debug.h>

#include <QPainter>

#include <kistest.h>
#include <KoSvgTextShape.h>
#include <KoShape.h>
#include <KoPathShape.h>
#include <KoShapeGroup.h>
#include <KoShapeManager.h>

#include <KoShapeRegistry.h>
#include "KoShapePainter.h"
#include <KoShapeStroke.h>

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
    KoShapeStrokeModelSP blackStroke(new KoShapeStroke(1.0, QColor(Qt::black)));
    p->setStroke(blackStroke);
    return p;
}

namespace detail {
Q_NAMESPACE

enum TransformComponent
{
    None = 0x0,
    Translate = 0x1,
    Scale = 0x2,
    Rotate = 0x4,
    Shear = 0x8,
    Project = 0x10
};
Q_ENUM_NS(TransformComponent)

}

using detail::TransformComponent;

Q_DECLARE_FLAGS(TransformComponents, TransformComponent)
Q_DECLARE_OPERATORS_FOR_FLAGS(TransformComponents)
Q_DECLARE_METATYPE(TransformComponents)

TransformComponents componentsForTransform(const QTransform &t) {
    TransformComponents result = TransformComponent::None;

    KisAlgebra2D::DecomposedMatrix m(t);

    result.setFlag(TransformComponent::Translate,
        !qFuzzyIsNull(m.dx) || !qFuzzyIsNull(m.dy));

    result.setFlag(TransformComponent::Scale,
        !qFuzzyCompare(m.scaleX, 1.0) || !qFuzzyCompare(m.scaleY, 1.0));

    result.setFlag(TransformComponent::Shear,
        !qFuzzyIsNull(m.shearXY));

    result.setFlag(TransformComponent::Rotate,
        !qFuzzyIsNull(m.angle));

    result.setFlag(TransformComponent::Project,
        !qFuzzyIsNull(m.proj[0]) || !qFuzzyIsNull(m.proj[1]) || !qFuzzyCompare(m.proj[2], 1.0));

    return result;
}
auto rotateAroundPoint = [] (qreal deg, const QPointF center) {
    return QTransform::fromTranslate(-center.x(), -center.y()) * QTransform().rotate(deg) * QTransform::fromTranslate(center.x(), center.y());
};


void TestSvgTextShape::testSetTextOnShape_data()
{

    QTest::addColumn<QTransform>("textTransform");
    QTest::addColumn<int>("contourCount");
    QTest::addColumn<QTransform>("contourTransform");
    QTest::addColumn<bool>("textHasParent");
    QTest::addColumn<bool>("contourHasParent");
    QTest::addColumn<TransformComponents>("expectedTextTransformComponents");

    for (int contourCount : {1, 2}) {
        for (bool textHasParent : {false, true}) {
            for (bool contourHasParent : {false, true}) {
                const char *textParentPrefix =
                        textHasParent ? "[text:parent]" : "[text:no-parent]";
                const char *contourParentPrefix =
                        contourHasParent ? "[contour:parent]" : "[contour:no-parent]";

                QTest::addRow("%s%s text and %d contour(s)", textParentPrefix, contourParentPrefix, contourCount)
                        << QTransform::fromTranslate(10, 20) << contourCount
                        << QTransform::fromTranslate(5, 5)
                        << textHasParent
                        << contourHasParent
                        << TransformComponents(TransformComponent::Translate);

                QTest::addRow("%s%s text and %d contour(s) (scaled)", textParentPrefix, contourParentPrefix, contourCount)
                        << QTransform::fromTranslate(10, 20) << contourCount
                        << QTransform::fromScale(1.0, 1.5) * QTransform::fromTranslate(5, 5)
                        << textHasParent
                        << contourHasParent
                        << TransformComponents(TransformComponent::Translate);

                QTest::addRow("%s%s text and %d contour(s) (rotated)", textParentPrefix, contourParentPrefix, contourCount)
                        << QTransform::fromTranslate(10, 20) << contourCount
                        << rotateAroundPoint(30, QPointF(50, 50)) * QTransform::fromTranslate(20, 25)
                        << textHasParent
                        << contourHasParent
                        << TransformComponents(TransformComponent::Translate);

                QTest::addRow("%s%s text (scaled) and %d contour(s)", textParentPrefix, contourParentPrefix, contourCount)
                        << QTransform::fromScale(1.0, 1.5) * QTransform::fromTranslate(10, 20) << contourCount
                        << QTransform::fromTranslate(5, 5)
                        << textHasParent
                        << contourHasParent
                        << TransformComponents(TransformComponent::Translate | TransformComponent::Scale);

                QTest::addRow("%s%s text (rotated) and %d contour(s)", textParentPrefix, contourParentPrefix, contourCount)
                        << rotateAroundPoint(30, QPointF(100, -10)) * QTransform::fromTranslate(10, 80) << contourCount
                        << QTransform::fromTranslate(5, 5)
                        << textHasParent
                        << contourHasParent
                        << TransformComponents(TransformComponent::Translate | TransformComponent::Rotate);
            }
        }
    }
}

void TestSvgTextShape::testSetTextOnShape()
{
    auto stripTestName = [] (QString name) {
        name.replace(':', '_');
        name.replace('[', '_');
        name.replace(']', '_');
        name.replace(' ', '_');
        name.replace('-', '_');
        name.remove('(');
        name.remove(')');
        return name;
    };

    qDebug() << stripTestName;

    auto paintShapes = [stripTestName] (KoShape *textShape, QList<KoShape*> contourShapes, const QString &testName) {
        QImage image(QSize(300, 200), QImage::Format_ARGB32);
        image.fill(0);
        QPainter gc(&image);
        gc.setClipRect(image.rect());

        QList<KoShape*> paintedShapes;
        if (textShape) {
            paintedShapes << textShape;
        }
        paintedShapes << contourShapes;

        KoShapePainter p;
        p.setShapes(paintedShapes);
        p.paint(gc, image.rect(), image.rect());
        image.save(stripTestName(testName));
    };

    auto debugShapes = [] (KoShape *textShape, QList<KoShape*> contourShapes) {
        qDebug() << ppVar(textShape->boundingRect());
        qDebug() << ppVar(textShape->transformation());
        for (int i = 0; i < contourShapes.size(); i++) {
            KoShape *shape = contourShapes[i];
            qDebug() << "contour" << i << ppVar(shape->boundingRect());
            qDebug() << "contour" << i << ppVar(shape->absoluteOutlineRect());
            qDebug() << "contour" << i << ppVar(shape->transformation());
        }
    };



    KoSvgTextShape *textShape = new KoSvgTextShape();
    textShape->insertText(0, "The quick brown fox jumps over the lazy dog.");
    QList<KoShape*> shapes;

    QFETCH(QTransform, textTransform);
    QFETCH(int, contourCount);
    QFETCH(QTransform, contourTransform);
    QFETCH(bool, textHasParent);
    QFETCH(bool, contourHasParent);
    QFETCH(TransformComponents, expectedTextTransformComponents);

    KoShapeGroup *commonParent = nullptr;
    if (textHasParent || contourHasParent) {
        commonParent = new KoShapeGroup();
        commonParent->setTransformation(QTransform::fromTranslate(17, 23));
    }

    textShape->setTransformation(textTransform);
    if (textHasParent) {
        textShape->setParent(commonParent);
    }

    for (int i = 0; i < contourCount; i++) {
        KoPathShape *p = createPolygon();
        p->setTransformation(contourTransform);
        if (contourHasParent) {
            p->setParent(commonParent);
        }
        shapes.append(p);
    }

    bool inside = true;

    QScopedPointer<KUndo2Command> parentCommand(new KUndo2Command());

    const QRectF originalTextOutlineRect = textShape->outlineRect();
    const QRectF originalTextBoundingRect = textShape->boundingRect();
    const QRectF originalContourShapeBoundingRect = KoShape::boundingRect(shapes);

    QList<KoShape*> clonedShapes;
    Q_FOREACH(KoShape *shape, shapes) {
        KoShape *clone = shape->cloneShape();
        if (contourHasParent) {
            clone->setParent(commonParent);
        }
        clonedShapes.append(clone);
        new KoSvgTextAddShapeCommand(textShape, clone, inside, parentCommand.data());
    }

    debugShapes(textShape, clonedShapes);
    paintShapes(textShape, clonedShapes, QString("ddd_%1_00_initial.png").arg(QTest::currentDataTag()));

    parentCommand->redo();

    debugShapes(textShape, clonedShapes);
    paintShapes(textShape, {}, QString("ddd_%1_10_redo.png").arg(QTest::currentDataTag()));

    QCOMPARE(componentsForTransform(textShape->absoluteTransformation()), expectedTextTransformComponents);

    QVERIFY(textShape->boundingRect() == originalContourShapeBoundingRect);
    QVERIFY(textShape->boundingRect() != originalTextBoundingRect);
    QVERIFY(textShape->outlineRect() != originalTextOutlineRect);
    for (int i = 0; i< clonedShapes.size(); i++) {
        KoShape *contour = clonedShapes.at(i);
        KoShape *original = shapes.at(i);
        QVERIFY(KisAlgebra2D::fuzzyMatrixCompare(contour->absoluteTransformation(), original->absoluteTransformation(), 1e-3));
    }

    parentCommand->undo();

    debugShapes(textShape, clonedShapes);
    paintShapes(textShape, clonedShapes, QString("ddd_%1_20_undo.png").arg(QTest::currentDataTag()));

    QVERIFY(textShape->boundingRect() != originalContourShapeBoundingRect);
    QVERIFY(textShape->boundingRect() == originalTextBoundingRect);
    QVERIFY(textShape->outlineRect() == originalTextOutlineRect);

    for (int i = 0; i< clonedShapes.size(); i++) {
        KoShape *contour = clonedShapes.at(i);
        KoShape *original = shapes.at(i);
        QVERIFY(contour->parent() == original->parent());
        QVERIFY(KisAlgebra2D::fuzzyMatrixCompare(contour->transformation(), original->transformation(), 1e-3));
        QVERIFY(KisAlgebra2D::fuzzyMatrixCompare(contour->absoluteTransformation(), original->absoluteTransformation(), 1e-3));
    }

    if (!textHasParent) {
        delete textShape;
    }

    if (!contourHasParent) {
        qDeleteAll(clonedShapes);
        qDeleteAll(shapes);
    }

    if (commonParent) {
        delete commonParent;
    }
}

void TestSvgTextShape::testRemoveShapeFromText_data()
{
    QTest::addColumn<QTransform>("textTransform");
    QTest::addColumn<QTransform>("contourTransform");
    QTest::addColumn<TransformComponents>("expectedContourTransformComponents");

    QTest::addRow("remove contour from text")
            << QTransform::fromTranslate(10, 20)
            << QTransform::fromTranslate(5, 5)
            << TransformComponents(TransformComponent::Translate);

    QTest::addRow("remove contour (scaled) from text")
            << QTransform::fromTranslate(10, 20)
            << QTransform::fromScale(1.0, 1.5) * QTransform::fromTranslate(5, 5)
            << TransformComponents(TransformComponent::Translate | TransformComponent::Scale);

    QTest::addRow("remove contour (rotated) from text")
            << QTransform::fromTranslate(10, 20)
            << rotateAroundPoint(30, QPointF(50, 50)) * QTransform::fromTranslate(20, 25)
            << TransformComponents(TransformComponent::Translate | TransformComponent::Rotate);

    QTest::addRow("remove contour from text (scaled)")
            << QTransform::fromScale(1.0, 1.5) * QTransform::fromTranslate(10, 20)
            << QTransform::fromTranslate(5, 5)
            << TransformComponents(TransformComponent::Translate | TransformComponent::Scale);

    QTest::addRow("remove contour from text (rotated)")
            << rotateAroundPoint(30, QPointF(100, -10)) * QTransform::fromTranslate(10, 80)
            << QTransform::fromTranslate(5, 5)
            << TransformComponents(TransformComponent::Translate | TransformComponent::Rotate);
}

void TestSvgTextShape::testRemoveShapeFromText()
{
    KoSvgTextShape *textShape = new KoSvgTextShape();
    textShape->insertText(0, "The quick brown fox jumps over the lazy dog.");
    QList<KoShape*> shapes;

    QFETCH(QTransform, textTransform);
    QFETCH(QTransform, contourTransform);
    QFETCH(TransformComponents, expectedContourTransformComponents);

    textShape->setTransformation(textTransform);

    for (int i = 0; i < 3; i++) {
        KoShape *p = createPolygon();
        p->setTransformation(QTransform::fromTranslate(i*10, i*10) * contourTransform);
        shapes.append(p);
    }
    textShape->setShapesInside(shapes);

    KUndo2Command *cmd = new KoSvgTextRemoveShapeCommand(textShape, shapes.last());

    cmd->redo();

    QVERIFY(!textShape->shapeInContours(shapes.last()));

    QCOMPARE(componentsForTransform(shapes.last()->absoluteTransformation()), expectedContourTransformComponents);

    cmd->undo();

    QVERIFY(textShape->shapeInContours(shapes.last()));

    delete textShape;
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

#include "TestSvgTextShape.moc"
