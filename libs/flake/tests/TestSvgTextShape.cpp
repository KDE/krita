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
#include <KoSvgTextShapeMarkupConverter.h>
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

#include <KoParameterShape.h>
#include <KisTransformComponents.h>

namespace {

// Quick function to get a rectangle.
KoShape *createRectangle() {
    KoShapeFactoryBase *base = KoShapeRegistry::instance()->get("RectangleShape");

    KoParameterShape *shape = dynamic_cast<KoParameterShape*>(base->createDefaultShape());
    shape->moveHandle(0, QPointF(10,10));
    shape->moveHandle(1, QPointF(15,15));
    shape->setBackground(nullptr);

    return shape;
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

///
/// Uncomment to get debugging output and vesual representation of the shapes
/// as .png files
///
// #define DEBUG_SHAPE_RENDERING

#ifdef DEBUG_SHAPE_RENDERING

QString stripTestName(QString name)
{
    name.replace("][", "_");
    name.replace(':', '_');
    name.replace('[', '_');
    name.replace(']', '_');
    name.replace(' ', '_');
    name.replace('-', '_');
    name.remove('(');
    name.remove(')');
    return name;
}
#endif /* DEBUG_SHAPE_RENDERING */

void paintShapes(KoShape *textShape, QList<KoShape *> contourShapes, const QString &testName)
{
#ifdef DEBUG_SHAPE_RENDERING
    QImage image(QSize(300, 200), QImage::Format_ARGB32);
    image.fill(0);
    QPainter gc(&image);
    gc.setClipRect(image.rect());

    QList<KoShape *> paintedShapes;
    if (textShape) {
        paintedShapes << textShape;
    }
    paintedShapes << contourShapes;

    KoShapePainter p;
    p.setShapes(paintedShapes);
    p.paint(gc, image.rect(), image.rect());
    image.save(stripTestName(testName));
#else /* DEBUG_SHAPE_RENDERING */
    Q_UNUSED(textShape)
    Q_UNUSED(contourShapes)
    Q_UNUSED(testName)
#endif /* DEBUG_SHAPE_RENDERING*/
}

void debugShapes(KoShape *textShape, QList<KoShape *> contourShapes)
{
#ifdef DEBUG_SHAPE_RENDERING
    qDebug() << ppVar(textShape->boundingRect());
    qDebug() << ppVar(textShape->transformation());
    for (int i = 0; i < contourShapes.size(); i++) {
        KoShape *shape = contourShapes[i];
        qDebug() << "contour" << i << ppVar(shape->boundingRect());
        qDebug() << "contour" << i << ppVar(shape->absoluteOutlineRect());
        qDebug() << "contour" << i << ppVar(shape->transformation());
    }
#else /* DEBUG_SHAPE_RENDERING */
    Q_UNUSED(textShape)
    Q_UNUSED(contourShapes)
#endif /* DEBUG_SHAPE_RENDERING */
}

struct ComplexTextInContourBuilder
{
    ComplexTextInContourBuilder(const QTransform &textTransform,
                                int contourCount,
                                const QTransform &contourTransform,
                                bool textHasParent,
                                bool contourHasParent,
                                bool contourIsParametric = false)
    {
        textShape = new KoSvgTextShape();

        const char* text = contourCount > 1
            ? "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed eu dui felis. Aliquam pellentesque nisi ut "
              "est dapibus congue. Donec nunc lectus, ornare eget turpis convallis, pharetra laoreet metus. Donec non "
              "bibendum nisi, eget egestas libero. Praesent ac fermentum lectus. Aenean faucibus est consectetur "
              "ornare mattis. Etiam laoreet vulputate ex id bibendum."
            : "The quick brown fox jumps over the lazy dog.";

        textShape->insertText(0, text);

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
            KoShape *p = !contourIsParametric ? createPolygon() : createRectangle();

            QTransform transform = contourTransform;

            if (i > 0) {
                transform = transform * QTransform::fromTranslate(i * 80, i * 20);
            }

            p->setTransformation(transform);
            if (contourHasParent) {
                p->setParent(commonParent);
            }
            shapes.append(p);
        }
    }

    ~ComplexTextInContourBuilder()
    {
        textToContourCommand.reset();

        Q_FOREACH(KoShape *shape, shapes) {
            if (!shape->parent()) {
                delete shape;
            }
        }
        shapes.clear();

        // for the case when the grouping command is not undone
        // we should delte the text shape **after** the contours
        if (!textShape->parent()) {
            delete textShape;
            textShape = nullptr;
        }

        if (commonParent) {
            delete commonParent;
            commonParent = nullptr;
        }
    }


    void addTextToContour()
    {
        textToContourCommand.reset(new KUndo2Command());

        Q_FOREACH (KoShape *shape, shapes) {
            new KoSvgTextAddShapeCommand(textShape, shape, true, textToContourCommand.data());
        }
        textToContourCommand->redo();
    }

    void undoTextToContour() {
        textToContourCommand->undo();
    }

    KoSvgTextShape *textShape { nullptr };
    KoShapeGroup *commonParent { nullptr };
    QList<KoShape*> shapes;
    QScopedPointer<KUndo2Command> textToContourCommand;
};

const char* anchorPositionToString(KoFlake::AnchorPosition anchor) {
    switch (anchor) {
        case KoFlake::TopLeft: return "TopLeft";
        case KoFlake::Top: return "Top";
        case KoFlake::TopRight: return "TopRight";
        case KoFlake::Left: return "Left";
        case KoFlake::Center: return "Center";
        case KoFlake::Right: return "Right";
        case KoFlake::BottomLeft: return "BottomLeft";
        case KoFlake::Bottom: return "Bottom";
        case KoFlake::BottomRight: return "BottomRight";
        case KoFlake::NoAnchor: return "NoAnchor";
        default: return "Unknown";
    }
};

const char* hasParentToString(bool hasParent) {
    return hasParent ? "parent" : "no-parent";
}

}

using KisAlgebra2D::KisTransformComponent;
using KisAlgebra2D::KisTransformComponents;
using KisAlgebra2D::componentsForTransform;
using KisAlgebra2D::compareTransformComponents;
using KisAlgebra2D::makeFullTransformComponents;

auto rotateAroundPoint = [] (qreal deg, const QPointF center) {
    return QTransform::fromTranslate(-center.x(), -center.y()) * QTransform().rotate(deg) * QTransform::fromTranslate(center.x(), center.y());
};


void TestSvgTextShape::testSetTextOnShape_data()
{

    QTest::addColumn<QTransform>("textTransform");
    QTest::addColumn<QTransform>("contourTransform");
    QTest::addColumn<int>("contourCount");
    QTest::addColumn<bool>("textHasParent");
    QTest::addColumn<bool>("contourHasParent");
    QTest::addColumn<KisTransformComponents>("expectedTextTransformComponents");

    for (int contourCount : {1, 2}) {
        for (bool textHasParent : {false, true}) {
            for (bool contourHasParent : {false, true}) {
                QTest::addRow("[text:%s][contour:%s] text and %d contour(s)",
                              hasParentToString(textHasParent),
                              hasParentToString(contourHasParent),
                              contourCount)
                        << QTransform::fromTranslate(10, 20)
                        << QTransform::fromTranslate(5, 5)
                        << contourCount
                        << textHasParent
                        << contourHasParent
                        << KisTransformComponents(KisTransformComponent::Translate);

                QTest::addRow("[text:%s][contour:%s] text and %d contour(s) (scaled)",
                              hasParentToString(textHasParent),
                              hasParentToString(contourHasParent),
                              contourCount)
                        << QTransform::fromTranslate(10, 20)
                        << QTransform::fromScale(1.0, 1.5) * QTransform::fromTranslate(5, 5)
                        << contourCount
                        << textHasParent
                        << contourHasParent
                        << KisTransformComponents(KisTransformComponent::Translate);

                QTest::addRow("[text:%s][contour:%s] text and %d contour(s) (rotated)",
                              hasParentToString(textHasParent),
                              hasParentToString(contourHasParent),
                              contourCount)
                        << QTransform::fromTranslate(10, 20)
                        << rotateAroundPoint(30, QPointF(50, 50)) * QTransform::fromTranslate(20, 25)
                        << contourCount
                        << textHasParent
                        << contourHasParent
                        << KisTransformComponents(KisTransformComponent::Translate);

                QTest::addRow("[text:%s][contour:%s] text (scaled) and %d contour(s)",
                              hasParentToString(textHasParent),
                              hasParentToString(contourHasParent),
                              contourCount)
                        << QTransform::fromScale(1.0, 1.5) * QTransform::fromTranslate(10, 20)
                        << QTransform::fromTranslate(5, 5)
                        << contourCount
                        << textHasParent
                        << contourHasParent
                        << KisTransformComponents(KisTransformComponent::Translate | KisTransformComponent::Scale);

                QTest::addRow("[text:%s][contour:%s] text (rotated) and %d contour(s)",
                              hasParentToString(textHasParent),
                              hasParentToString(contourHasParent),
                              contourCount)
                        << rotateAroundPoint(30, QPointF(100, -10)) * QTransform::fromTranslate(10, 80)
                        << QTransform::fromTranslate(5, 5)
                        << contourCount
                        << textHasParent
                        << contourHasParent
                        << KisTransformComponents(KisTransformComponent::Translate | KisTransformComponent::Rotate);
            }
        }
    }
}

void TestSvgTextShape::testSetTextOnShape()
{
    QFETCH(QTransform, textTransform);
    QFETCH(int, contourCount);
    QFETCH(QTransform, contourTransform);
    QFETCH(bool, textHasParent);
    QFETCH(bool, contourHasParent);
    QFETCH(KisTransformComponents, expectedTextTransformComponents);

    ComplexTextInContourBuilder b(textTransform,
                                  contourCount,
                                  contourTransform,
                                  textHasParent,
                                  contourHasParent);

    const QRectF originalTextOutlineRect = b.textShape->outlineRect();
    const QRectF originalTextBoundingRect = b.textShape->boundingRect();
    const QRectF originalContourShapeBoundingRect = KoShape::boundingRect(b.shapes);

    QHash<KoShape*, QTransform> originalTransform;
    QHash<KoShape*, QTransform> originalAbsoluteTransform;
    QHash<KoShape*, KoShape*> originalParent;

    Q_FOREACH(KoShape *shape, b.shapes) {
        originalTransform.insert(shape, shape->transformation());
        originalAbsoluteTransform.insert(shape, shape->absoluteTransformation());
        originalParent.insert(shape, shape->parent());
    }

    debugShapes(b.textShape, b.shapes);
    paintShapes(b.textShape, b.shapes, QString("ddd_%1_00_initial.png").arg(QTest::currentDataTag()));

    b.addTextToContour();

    debugShapes(b.textShape, b.shapes);
    paintShapes(b.textShape, {}, QString("ddd_%1_10_redo.png").arg(QTest::currentDataTag()));

    QCOMPARE(componentsForTransform(b.textShape->absoluteTransformation()), expectedTextTransformComponents);

    QVERIFY(b.textShape->boundingRect() == originalContourShapeBoundingRect);
    QVERIFY(b.textShape->boundingRect() != originalTextBoundingRect);
    QVERIFY(b.textShape->outlineRect() != originalTextOutlineRect);

    Q_FOREACH (KoShape *shape, b.shapes) {
        QVERIFY(KisAlgebra2D::fuzzyMatrixCompare(shape->absoluteTransformation(), originalAbsoluteTransform[shape], 1e-3));
    }

    b.undoTextToContour();

    debugShapes(b.textShape, b.shapes);
    paintShapes(b.textShape, b.shapes, QString("ddd_%1_20_undo.png").arg(QTest::currentDataTag()));

    QVERIFY(b.textShape->boundingRect() != originalContourShapeBoundingRect);
    QVERIFY(b.textShape->boundingRect() == originalTextBoundingRect);
    QVERIFY(b.textShape->outlineRect() == originalTextOutlineRect);

    Q_FOREACH (KoShape *shape, b.shapes) {
        QVERIFY(shape->parent() == originalParent[shape]);

        QVERIFY(KisAlgebra2D::fuzzyMatrixCompare(shape->transformation(), originalTransform[shape], 1e-3));
        QVERIFY(KisAlgebra2D::fuzzyMatrixCompare(shape->absoluteTransformation(), originalAbsoluteTransform[shape], 1e-3));
    }
}

void TestSvgTextShape::testRemoveShapeFromText_data()
{
    QTest::addColumn<QTransform>("textTransform");
    QTest::addColumn<QTransform>("contourTransform");
    QTest::addColumn<KisTransformComponents>("expectedContourTransformComponents");

    QTest::addRow("remove contour from text")
            << QTransform::fromTranslate(10, 20)
            << QTransform::fromTranslate(5, 5)
            << KisTransformComponents(KisTransformComponent::Translate);

    QTest::addRow("remove contour (scaled) from text")
            << QTransform::fromTranslate(10, 20)
            << QTransform::fromScale(1.0, 1.5) * QTransform::fromTranslate(5, 5)
            << KisTransformComponents(KisTransformComponent::Translate | KisTransformComponent::Scale);

    QTest::addRow("remove contour (rotated) from text")
            << QTransform::fromTranslate(10, 20)
            << rotateAroundPoint(30, QPointF(50, 50)) * QTransform::fromTranslate(20, 25)
            << KisTransformComponents(KisTransformComponent::Translate | KisTransformComponent::Rotate);

    QTest::addRow("remove contour from text (scaled)")
            << QTransform::fromScale(1.0, 1.5) * QTransform::fromTranslate(10, 20)
            << QTransform::fromTranslate(5, 5)
            << KisTransformComponents(KisTransformComponent::Translate | KisTransformComponent::Scale);

    QTest::addRow("remove contour from text (rotated)")
            << rotateAroundPoint(30, QPointF(100, -10)) * QTransform::fromTranslate(10, 80)
            << QTransform::fromTranslate(5, 5)
            << KisTransformComponents(KisTransformComponent::Translate | KisTransformComponent::Rotate);
}

void TestSvgTextShape::testRemoveShapeFromText()
{
    KoSvgTextShape *textShape = new KoSvgTextShape();
    textShape->insertText(0, "The quick brown fox jumps over the lazy dog.");
    QList<KoShape*> shapes;

    QFETCH(QTransform, textTransform);
    QFETCH(QTransform, contourTransform);
    QFETCH(KisTransformComponents, expectedContourTransformComponents);

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

Q_DECLARE_METATYPE(KoFlake::AnchorPosition)

void TestSvgTextShape::testSetSize_data()
{
    QTest::addColumn<qreal>("newSizeCoeffX");
    QTest::addColumn<qreal>("newSizeCoeffY");
    QTest::addColumn<int>("newSizeNumSteps");
    QTest::addColumn<KoFlake::AnchorPosition>("stillPointAnchor");

    QTest::addColumn<QTransform>("textTransform");
    QTest::addColumn<QTransform>("contourTransform");
    QTest::addColumn<bool>("contourIsParametric");
    QTest::addColumn<int>("contourCount");
        QTest::addColumn<bool>("textHasParent");
    QTest::addColumn<bool>("contourHasParent");

    QTest::addColumn<KisTransformComponents>("exprectedPreservedChildTransformComponents");

    const KisTransformComponents preserveEverythingButTranslation =
        makeFullTransformComponents().setFlag(KisTransformComponent::Translate, false);
    const KisTransformComponents preserveEverythingButTranslationAndScale =
        makeFullTransformComponents()
            .setFlag(KisTransformComponent::Translate, false)
            .setFlag(KisTransformComponent::Scale, false);
    const KisTransformComponents preserveNothing =
        KisTransformComponents(KisTransformComponent::Project);

    const std::vector<KoFlake::AnchorPosition> anchors = {
        KoFlake::BottomRight, KoFlake::TopLeft, KoFlake::Top
    };

    struct TestJob {
        bool contourIsParametric;
        int contourCount;
        bool textHasParent;
        bool contourHasParent;
        int steps;
        KoFlake::AnchorPosition anchor;
    };

    std::vector<TestJob> jobs;

    for (int contourCount : {1, 2}) {
        for (bool textHasParent : {false, true}) {
            for (bool contourHasParent : {false, true}) {
                for (int steps : {1, 3}) {
                    for (KoFlake::AnchorPosition anchor : anchors) {
                        // extensively test patch shape only
                        jobs.push_back({false, contourCount, textHasParent, contourHasParent, steps, anchor});
                    }
                }
            }
        }
    }

    // add one simple job for a parametric shape
    jobs.push_back({true, 1, true, true, 3, KoFlake::BottomRight});

    for (const auto &job : jobs) {
        const char *shapeType = job.contourIsParametric ? "rect" : "path";

        QTest::addRow("[%s][text:%s][contour:%s][anchor:%s][steps:%d] text and %d contour(s)",
                      shapeType,
                      hasParentToString(job.textHasParent),
                      hasParentToString(job.contourHasParent),
                      anchorPositionToString(job.anchor),
                      job.steps,
                      job.contourCount)
            << 1.3 << 1.5 << job.steps << job.anchor << QTransform::fromTranslate(10, 20)
            << QTransform::fromTranslate(5, 5) << job.contourIsParametric << job.contourCount << job.textHasParent
            << job.contourHasParent
            // when there is only translation in the internal shapes, then resize them instead
            // of scaling
            << preserveEverythingButTranslation;

        QTest::addRow("[%s][text:%s][contour:%s][anchor:%s][steps:%d] text and %d contour(s) (scaled)",
                      shapeType,
                      hasParentToString(job.textHasParent),
                      hasParentToString(job.contourHasParent),
                      anchorPositionToString(job.anchor),
                      job.steps,
                      job.contourCount)
            << 1.3 << 1.5 << job.steps << job.anchor << QTransform::fromTranslate(10, 20)
            << QTransform::fromScale(1.0, 1.5) * QTransform::fromTranslate(5, 5) << job.contourIsParametric
            << job.contourCount << job.textHasParent << job.contourHasParent
            << preserveEverythingButTranslationAndScale;

        QTest::addRow("[%s][text:%s][contour:%s][anchor:%s][steps:%d] text and %d contour(s) (rotated)",
                      shapeType,
                      hasParentToString(job.textHasParent),
                      hasParentToString(job.contourHasParent),
                      anchorPositionToString(job.anchor),
                      job.steps,
                      job.contourCount)
            << 1.3 << 1.5 << job.steps << job.anchor << QTransform::fromTranslate(10, 20)
            << rotateAroundPoint(30, QPointF(50, 50)) * QTransform::fromTranslate(20, 25) << job.contourIsParametric
            << job.contourCount << job.textHasParent << job.contourHasParent << preserveNothing;

        QTest::addRow("[%s][text:%s][contour:%s][anchor:%s][steps:%d] text (scaled) and %d contour(s)",
                      shapeType,
                      hasParentToString(job.textHasParent),
                      hasParentToString(job.contourHasParent),
                      anchorPositionToString(job.anchor),
                      job.steps,
                      job.contourCount)

            << 1.3 << 1.5 << job.steps << job.anchor

            << QTransform::fromScale(1.0, 1.5) * QTransform::fromTranslate(10, 20) << QTransform::fromTranslate(5, 5)
            << job.contourIsParametric << job.contourCount << job.textHasParent
            << job.contourHasParent
            // after a shape has been added to a transformed text, it received an
            // inverted transform of the text, so we cannot manipulate it normally
            // anymore
            << preserveEverythingButTranslationAndScale;

        // this case looks weird in the test, but in GUI it looks rather consistent,
        // becasue the handles are also rotated
        QTest::addRow("[%s][text:%s][contour:%s][anchor:%s][steps:%d] text (rotated) and %d contour(s)",
                      shapeType,
                      hasParentToString(job.textHasParent),
                      hasParentToString(job.contourHasParent),
                      anchorPositionToString(job.anchor),
                      job.steps,
                      job.contourCount)

            << 1.3 << 1.5 << job.steps << job.anchor

            << rotateAroundPoint(30, QPointF(100, -10)) * QTransform::fromTranslate(10, 80)
            << QTransform::fromTranslate(5, 5) << job.contourIsParametric << job.contourCount << job.textHasParent
            << job.contourHasParent
            // after a shape has been added to a transformed text, it received an
            // inverted transform of the text, so we cannot manipulate it normally
            // anymore
            << preserveNothing;
    }
}

#include <commands/KoShapeResizeCommand.h>

void TestSvgTextShape::testSetSize()
{
    QFETCH(qreal, newSizeCoeffX);
    QFETCH(qreal, newSizeCoeffY);
    QFETCH(int, newSizeNumSteps);
    QFETCH(KoFlake::AnchorPosition, stillPointAnchor);

    QFETCH(QTransform, textTransform);
    QFETCH(QTransform, contourTransform);
    QFETCH(bool, contourIsParametric);
    QFETCH(int, contourCount);
    QFETCH(bool, textHasParent);
    QFETCH(bool, contourHasParent);

    QFETCH(KisTransformComponents, exprectedPreservedChildTransformComponents);

    ComplexTextInContourBuilder b(textTransform,
                                  contourCount,
                                  contourTransform,
                                  textHasParent,
                                  contourHasParent,
                                  contourIsParametric);

    b.addTextToContour();

    debugShapes(b.textShape, b.shapes);
    paintShapes(b.textShape, b.shapes, QString("ddd_setSize_%1_00_initial.png").arg(QTest::currentDataTag()));

    const QSizeF newSize(b.textShape->size().width() * newSizeCoeffX, b.textShape->size().height() * newSizeCoeffY);

    QHash<KoShape*, QTransform> originalAbsoluteTransform;
    QHash<KoShape*, QTransform> originalTransform;
    QHash<KoShape*, QSizeF> originalSize;

    for (KoShape *shape : b.shapes) {
        originalSize[shape] = shape->size();
        originalTransform[shape] = shape->transformation();
        originalAbsoluteTransform[shape] = shape->absoluteTransformation();
    }

    const QPointF absoluteStillPoint = b.textShape->absolutePosition(stillPointAnchor);
    const QPointF firstContourAnchorPoint = b.shapes[0]->absolutePosition(stillPointAnchor);

    if (contourCount == 1 && componentsForTransform(b.shapes[0]->transformation()) == KisTransformComponent::Translate) {
        QCOMPARE(absoluteStillPoint, firstContourAnchorPoint);
    }

    const bool useGLobalMode = false;
    const bool usePostScaling = false;

    QScopedPointer<KoShapeResizeCommand> cmd;

    for (int i = 0; i < newSizeNumSteps; i++) {
        auto currentCoeff = [&] (qreal maxCoeff) {
            if (i == newSizeNumSteps - 1) {
                return maxCoeff;
            } else {
                const qreal alpha = qreal(i) / (newSizeNumSteps - 1);
                return KisAlgebra2D::lerp(1.0, maxCoeff, alpha);
            }
        };

        const qreal scaleX = currentCoeff(newSizeCoeffX);
        const qreal scaleY = currentCoeff(newSizeCoeffY);

        if (!cmd) {
            cmd.reset(new KoShapeResizeCommand({b.textShape}, scaleX, scaleY, absoluteStillPoint, useGLobalMode, usePostScaling, QTransform()));
            cmd->redo();
        } else {
            cmd->replaceResizeAction(scaleX, scaleY, absoluteStillPoint);
        }
    }

    debugShapes(b.textShape, b.shapes);
    paintShapes(b.textShape, b.shapes, QString("ddd_setSize_%1_30_final.png").arg(QTest::currentDataTag()));

    QCOMPARE(b.textShape->size(), newSize);
    QCOMPARE(b.textShape->absolutePosition(stillPointAnchor), absoluteStillPoint);

    if (contourCount == 1 && componentsForTransform(b.shapes[0]->transformation()) == KisTransformComponent::Translate) {
        // the still point of the only contour was unchanged (applies only in translation mode)
        QCOMPARE(b.shapes[0]->absolutePosition(stillPointAnchor), firstContourAnchorPoint);
    }

    for (KoShape *shape : b.shapes) {
        // Verify what transformation components were preserved in the child's transform.
        // When setSize() is passed down to the child change all components are preserved,
        // except the translation one. When the children are transformed using the transoform,
        // then their whole transformation will be messed up.
        auto preservedComponents = compareTransformComponents(shape->transformation(), originalTransform[shape]);
        QCOMPARE(preservedComponents, exprectedPreservedChildTransformComponents);
    }

    cmd->undo();

    debugShapes(b.textShape, b.shapes);
    paintShapes(b.textShape, b.shapes, QString("ddd_setSize_%1_40_undo.png").arg(QTest::currentDataTag()));

    for (KoShape *shape : b.shapes) {
        QCOMPARE(shape->size(), originalSize[shape]);
        QVERIFY(KisAlgebra2D::fuzzyMatrixCompare(shape->transformation(), originalTransform[shape], 1e-4));
        QVERIFY(KisAlgebra2D::fuzzyMatrixCompare(shape->absoluteTransformation(), originalAbsoluteTransform[shape], 1e-4));
    }
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

void TestSvgTextShape::testTextPathOnRange_data()
{
    QTest::addColumn<QString>("svg");
    QTest::addColumn<int>("startPos");
    QTest::addColumn<int>("endPos");

    QTest::addRow("Single node, full range") << "<text>Test</text>" << 0 << 4;
    QTest::addRow("Single node, front") << "<text>Test test</text>" << 0 << 4;
    QTest::addRow("Single node, back") << "<text>Test test</text>" << 4 << 10;
    QTest::addRow("Complex text") << "<text>Text <tspan fill=\"#ff0000\">path</tspan> test</text>" << 6 << 12;

    QTest::addRow("Complex with paths") << "<text>"
                                           "<textPath path=\"M0 0L10 10\">Text </textPath>"
                                           "<textPath path=\"M0 0L10 10\" fill=\"#ff0000\">path</textPath>"
                                           "<textPath path=\"M0 0L10 10\"> test</textPath>"
                                           "</text>" << 2 << 14;
    QTest::addRow("Complex with paths and deep tree")
            << "<text>"
               "<textPath path=\"M0 0L10 10\">The quick <tspan fill=\"#aa8800\">brown </tspan></textPath>"
               "<textPath path=\"M0 0L10 10\" fill=\"#ff0000\">fox</textPath>"
               "<textPath path=\"M0 0L10 10\"> jumps <tspan fill=\"#00ff00\">over the lazy dog</tspan></textPath>"
               "</text>" << 14 << 26;
}

void TestSvgTextShape::testTextPathOnRange()
{
    QFETCH(QString, svg);
    QFETCH(int, startPos);
    QFETCH(int, endPos);
    KoSvgTextShape *textShape = new KoSvgTextShape();
    KoSvgTextShapeMarkupConverter converter(textShape);
    converter.convertFromSvg(svg, QString(), QRectF(0, 0, 300, 300), 72.0);
    KoPathShape *path = createPolygon();
    KoSvgTextSetTextPathOnRangeCommand *cmd = new KoSvgTextSetTextPathOnRangeCommand(textShape, path, startPos, endPos);

    cmd->redo();

    KoSvgTextNodeIndex idx = textShape->topLevelNodeForPos((startPos+endPos)/2);

    KoPathShape *textPath = dynamic_cast<KoPathShape*>(idx.textPath());
    QVERIFY(textPath);
    QCOMPARE(textPath->toString(), path->toString());

    //TODO test undo.
}

KISTEST_MAIN(TestSvgTextShape)

#include "TestSvgTextShape.moc"
