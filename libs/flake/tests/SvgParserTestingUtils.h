/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SVGPARSERTESTINGUTILS_H
#define SVGPARSERTESTINGUTILS_H

#include <svg/SvgParser.h>
#include <kis_debug.h>
#include <kis_global.h>
#include <KoViewConverter.h>
#include <KoDocumentResourceManager.h>
#include <KoShape.h>
#include <KoShapeGroup.h>
#include <QPainter>
#include <KoShapePainter.h>

#include "kis_algebra_2d.h"

#include <QXmlSimpleReader>

struct SvgTester
{
    SvgTester (const QString &data)
        : parser(&resourceManager),
          doc(SvgParser::createDocumentFromSvg(data))
    {
        root = doc.documentElement();

        parser.setXmlBaseDir("./");


        savedData = data;
        //printf("%s", savedData.toUtf8().data());

    }

    ~SvgTester ()
    {
        qDeleteAll(shapes);
    }

    void run() {
        shapes = parser.parseSvg(root, &fragmentSize);
    }

    KoShape* findShape(const QString &name, KoShape *parent = 0) {
        if (parent && parent->name() == name) {
            return parent;
        }

        QList<KoShape*> children;

        if (!parent) {
            children = shapes;
        } else {
            KoShapeContainer *cont = dynamic_cast<KoShapeContainer*>(parent);
            if (cont) {
                children = cont->shapes();
            }
        }

        Q_FOREACH (KoShape *shape, children) {
            KoShape *result = findShape(name, shape);
            if (result) {
                return result;
            }
        }

        return 0;
    }

    KoShapeGroup* findGroup(const QString &name) {
        KoShapeGroup *group = 0;
        KoShape *shape = findShape(name);
        if (shape) {
            group = dynamic_cast<KoShapeGroup*>(shape);
        }
        return group;
    }



    KoDocumentResourceManager resourceManager;
    SvgParser parser;
    QDomDocument doc;
    QDomElement root;
    QSizeF fragmentSize;
    QList<KoShape*> shapes;
    QString savedData;
};

#include "../../sdk/tests/qimage_test_util.h"

#ifdef USE_ROUND_TRIP
#include "SvgWriter.h"
#include <QBuffer>
#include <QDomDocument>
#endif

struct SvgRenderTester : public SvgTester
{
    SvgRenderTester(const QString &data)
        : SvgTester(data),
          m_fuzzyThreshold(0)
    {
    }

    void setFuzzyThreshold(int fuzzyThreshold) {
        m_fuzzyThreshold = fuzzyThreshold;
    }

    void setCheckQImagePremultiplied(bool value) {
        m_checkQImagePremiltiplied = value;
    }

    static void testRender(KoShape *shape, const QString &prefix, const QString &testName, const QSize canvasSize, int fuzzyThreshold = 0, bool checkQImagePremultiplied = false) {
        QImage canvas(canvasSize, QImage::Format_ARGB32);
        canvas.fill(0);
        QPainter painter(&canvas);

        KoShapePainter p;
        p.setShapes({shape});
        painter.setClipRect(canvas.rect());
        p.paint(painter);

        QVERIFY(TestUtil::checkQImageImpl(false,
                                          canvas, "svg_render", prefix, testName,
                                          fuzzyThreshold, -1, 0,
                                          checkQImagePremultiplied));
    }

    void test_standard_30px_72ppi(const QString &testName, bool verifyGeometry = true, const QSize &canvasSize = QSize(30,30)) {
        test_standard_impl(testName, verifyGeometry, canvasSize, 72.0);
    }

    void test_standard(const QString &testName, const QSize &canvasSize, qreal pixelsPerInch) {
        test_standard_impl(testName, false, canvasSize, pixelsPerInch);
    }

    void test_standard_impl(const QString &testName, bool verifyGeometry, const QSize &canvasSize, qreal pixelsPerInch) {

        QSize sizeInPx = canvasSize;
        QSizeF sizeInPt = QSizeF(canvasSize) * 72.0 / pixelsPerInch;
        Q_UNUSED(sizeInPt); // used in some definitions only!


        parser.setResolution(QRectF(QPointF(), sizeInPx) /* px */, pixelsPerInch /* ppi */);
        run();

#ifdef USE_CLONED_SHAPES
        {
            QList<KoShape*> newShapes;
            Q_FOREACH (KoShape *shape, shapes) {
                KoShape *clonedShape = shape->cloneShape();
                KIS_ASSERT(clonedShape);

                newShapes << clonedShape;
            }

            qDeleteAll(shapes);
            shapes = newShapes;
        }

#endif /* USE_CLONED_SHAPES */

#ifdef USE_ROUND_TRIP

        QBuffer writeBuf;
        writeBuf.open(QIODevice::WriteOnly);

        {
            SvgWriter writer(shapes);
            writer.save(writeBuf, sizeInPt);
        }

        QDomDocument prettyDoc;
        prettyDoc.setContent(savedData);


        qDebug();
        printf("\n=== Original: ===\n\n%s\n", prettyDoc.toByteArray(4).data());
        printf("\n=== Saved: ===\n\n%s\n", writeBuf.data().data());
        qDebug();

        QVERIFY(doc.setContent(writeBuf.data()));
        root = doc.documentElement();
        run();
#endif /* USE_ROUND_TRIP */

        KoShape *shape = findShape("testRect");
        KIS_ASSERT(shape);

        if (verifyGeometry) {
            QCOMPARE(shape->absolutePosition(KoFlake::TopLeft), QPointF(5,5));

            const QPointF bottomRight= shape->absolutePosition(KoFlake::BottomRight);
            const QPointF expectedBottomRight(15,25);

            if (KisAlgebra2D::norm(bottomRight - expectedBottomRight) > 0.0001 ) {
                QCOMPARE(bottomRight, expectedBottomRight);
            }
        }

        testRender(shape, "load", testName, canvasSize, m_fuzzyThreshold, m_checkQImagePremiltiplied);
    }

private:
    int m_fuzzyThreshold;
    int m_checkQImagePremiltiplied = false;
};


#endif // SVGPARSERTESTINGUTILS_H
