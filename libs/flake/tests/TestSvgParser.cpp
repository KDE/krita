/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "TestSvgParser.h"


#include <QTest>
#include <svg/SvgParser.h>
#include <kis_debug.h>
#include <kis_global.h>
#include <KoDocumentResourceManager.h>
#include <KoShape.h>
#include <KoShapeGroup.h>
#include <svg/SvgUtil.h>
#include <KoViewConverter.h>
#include <KoShapePaintingContext.h>
#include <QPainter>
#include <KoShapeStrokeModel.h>

struct SvgTester
{
    SvgTester (const QString &data)
        :  parser(&resourceManager)
    {
        QVERIFY(doc.setContent(data.toLatin1()));
        root = doc.documentElement();

        parser.setXmlBaseDir("./");

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
    KoXmlDocument doc;
    KoXmlElement root;
    QSizeF fragmentSize;
    QList<KoShape*> shapes;
};

void TestSvgParser::testUnitPx()
{
    const QString data =
            "<svg width=\"10px\" height=\"20px\" viewBox=\"0 0 10 20\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"0\" y=\"0\" width=\"10\" height=\"20\""
            "    fill=\"none\" stroke=\"none\" stroke-width=\"10\"/>"

            "</svg>";

    SvgTester t (data);
    t.parser.setResolution(QRectF(0, 0, 600, 400) /* px */, 72 /* ppi */);
    t.run();

    KoShape *shape = t.findShape("testRect");
    QVERIFY(shape);

    QCOMPARE(shape->boundingRect(), kisGrowRect(QRectF(0,0,10,20), 0.5));
    QCOMPARE(shape->absoluteTransformation(0), QTransform());
    QCOMPARE(shape->outlineRect(), QRectF(0,0,10,20));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeftCorner), QPointF(0,0));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRightCorner), QPointF(10,20));
}

void TestSvgParser::testUnitPxResolution()
{
    const QString data =
            "<svg width=\"10px\" height=\"20px\" viewBox=\"0 0 10 20\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"0\" y=\"0\" width=\"10\" height=\"20\""
            "    fill=\"none\" stroke=\"none\" stroke-width=\"10\"/>"

            "</svg>";

    SvgTester t (data);
    t.parser.setResolution(QRectF(0, 0, 600, 400) /* px */, 144 /* ppi */);
    t.run();

    KoShape *shape = t.findShape("testRect");
    QVERIFY(shape);

    QCOMPARE(shape->boundingRect(), kisGrowRect(QRectF(0,0,5,10), 0.25));
    QCOMPARE(shape->absoluteTransformation(0), QTransform::fromScale(0.5, 0.5));
    QCOMPARE(shape->outlineRect(), QRectF(0,0,10,20));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeftCorner), QPointF(0,0));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRightCorner), QPointF(5,10));
}


void TestSvgParser::testUnitPt()
{
    const QString data =
            "<svg width=\"10pt\" height=\"20pt\" viewBox=\"0 0 10 20\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"0\" y=\"0\" width=\"10\" height=\"20\""
            "    fill=\"none\" stroke=\"none\" stroke-width=\"10\"/>"

            "</svg>";

    SvgTester t (data);
    t.parser.setResolution(QRectF(0, 0, 600, 400) /* px */, 666 /* ppi */);
    t.run();

    KoShape *shape = t.findShape("testRect");
    QVERIFY(shape);

    QCOMPARE(shape->boundingRect(), kisGrowRect(QRectF(0,0,10,20), 0.5));
    QCOMPARE(shape->absoluteTransformation(0), QTransform());
    QCOMPARE(shape->outlineRect(), QRectF(0,0,10,20));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeftCorner), QPointF(0,0));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRightCorner), QPointF(10,20));
}

void TestSvgParser::testUnitIn()
{
    const QString data =
            "<svg width=\"10in\" height=\"20in\" viewBox=\"0 0 10 20\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"0\" y=\"0\" width=\"10\" height=\"20\""
            "    fill=\"none\" stroke=\"none\" stroke-width=\"10\"/>"

            "</svg>";

    SvgTester t (data);
    t.parser.setResolution(QRectF(0, 0, 600, 400) /* px */, 666 /* ppi */);
    t.run();

    KoShape *shape = t.findShape("testRect");
    QVERIFY(shape);

    QCOMPARE(shape->boundingRect(), kisGrowRect(QRectF(0,0,720,1440), 36));
    QCOMPARE(shape->absoluteTransformation(0), QTransform::fromScale(72, 72));
    QCOMPARE(shape->outlineRect(), QRectF(0,0,10,20));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeftCorner), QPointF(0,0));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRightCorner), QPointF(720,1440));
}

void TestSvgParser::testUnitPercentInitial()
{
    const QString data =
            "<svg width=\"12.5%\" height=\"25%\" viewBox=\"0 0 10 20\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"0\" y=\"0\" width=\"10\" height=\"20\""
            "    fill=\"none\" stroke=\"none\" stroke-width=\"10\"/>"

            "</svg>";

    SvgTester t (data);
    t.parser.setResolution(QRectF(0, 0, 80, 80) /* px */, 144 /* ppi */);
    t.run();

    KoShape *shape = t.findShape("testRect");
    QVERIFY(shape);

    QCOMPARE(shape->boundingRect(), kisGrowRect(QRectF(0,0,5,10), 0.25));
    QCOMPARE(shape->absoluteTransformation(0), QTransform::fromScale(0.5, 0.5));
    QCOMPARE(shape->outlineRect(), QRectF(0,0,10,20));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeftCorner), QPointF(0,0));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRightCorner), QPointF(5,10));
}

void TestSvgParser::testScalingViewport()
{
    const QString data =
            "<svg width=\"10px\" height=\"20px\" viewBox=\"60 70 20 40\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"64\" y=\"74\" width=\"12\" height=\"32\""
            "    fill=\"none\" stroke=\"none\" stroke-width=\"10\"/>"

            "</svg>";

    SvgTester t (data);
    t.parser.setResolution(QRectF(0, 0, 600, 400) /* px */, 72 /* ppi */);
    t.run();

    KoShape *shape = t.findShape("testRect");
    QVERIFY(shape);

    QCOMPARE(shape->absoluteTransformation(0), QTransform::fromTranslate(4, 4) * QTransform::fromScale(0.5, 0.5));
    QCOMPARE(shape->outlineRect(), QRectF(0,0,12,32));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeftCorner), QPointF(2,2));
    QCOMPARE(shape->absolutePosition(KoFlake::TopRightCorner), QPointF(8,2));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomLeftCorner), QPointF(2,18));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRightCorner), QPointF(8,18));
}

void TestSvgParser::testScalingViewportKeepMeet1()
{
    const QString data =
            "<svg width=\"10px\" height=\"30px\" viewBox=\"60 70 20 40\""
            "    preserveAspectRatio=\" xMinYMin meet\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"64\" y=\"74\" width=\"12\" height=\"32\""
            "    fill=\"none\" stroke=\"none\" stroke-width=\"10\"/>"

            "</svg>";

    SvgTester t (data);
    t.parser.setResolution(QRectF(0, 0, 600, 400) /* px */, 72 /* ppi */);
    t.run();

    KoShape *shape = t.findShape("testRect");
    QVERIFY(shape);

    QCOMPARE(shape->absoluteTransformation(0), QTransform::fromTranslate(4, 4) * QTransform::fromScale(0.5, 0.5));
    QCOMPARE(shape->outlineRect(), QRectF(0,0,12,32));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeftCorner), QPointF(2,2));
    QCOMPARE(shape->absolutePosition(KoFlake::TopRightCorner), QPointF(8,2));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomLeftCorner), QPointF(2,18));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRightCorner), QPointF(8,18));
}

void TestSvgParser::testScalingViewportKeepMeet2()
{
    const QString data =
            "<svg width=\"15px\" height=\"20px\" viewBox=\"60 70 20 40\""
            "    preserveAspectRatio=\" xMinYMin meet\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"64\" y=\"74\" width=\"12\" height=\"32\""
            "    fill=\"none\" stroke=\"none\" stroke-width=\"10\"/>"

            "</svg>";

    SvgTester t (data);
    t.parser.setResolution(QRectF(0, 0, 600, 400) /* px */, 72 /* ppi */);
    t.run();

    KoShape *shape = t.findShape("testRect");
    QVERIFY(shape);

    QCOMPARE(shape->absoluteTransformation(0), QTransform::fromTranslate(4, 4) * QTransform::fromScale(0.5, 0.5));
    QCOMPARE(shape->outlineRect(), QRectF(0,0,12,32));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeftCorner), QPointF(2,2));
    QCOMPARE(shape->absolutePosition(KoFlake::TopRightCorner), QPointF(8,2));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomLeftCorner), QPointF(2,18));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRightCorner), QPointF(8,18));
}

void TestSvgParser::testScalingViewportKeepMeetAlign()
{
    const QString data =
            "<svg width=\"10px\" height=\"30px\" viewBox=\"60 70 20 40\""
            "    preserveAspectRatio=\" xMaxYMax meet\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"64\" y=\"74\" width=\"12\" height=\"32\""
            "    fill=\"none\" stroke=\"none\" stroke-width=\"10\"/>"

            "</svg>";

    SvgTester t (data);
    t.parser.setResolution(QRectF(0, 0, 600, 400) /* px */, 72 /* ppi */);
    t.run();

    KoShape *shape = t.findShape("testRect");
    QVERIFY(shape);

    QCOMPARE(shape->absoluteTransformation(0), QTransform::fromTranslate(4, 24) * QTransform::fromScale(0.5, 0.5));
    QCOMPARE(shape->outlineRect(), QRectF(0,0,12,32));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeftCorner), QPointF(2,12));
    QCOMPARE(shape->absolutePosition(KoFlake::TopRightCorner), QPointF(8,12));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomLeftCorner), QPointF(2,28));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRightCorner), QPointF(8,28));
}

void TestSvgParser::testScalingViewportKeepSlice1()
{
    const QString data =
            "<svg width=\"5px\" height=\"20px\" viewBox=\"60 70 20 40\""
            "    preserveAspectRatio=\" xMinYMin slice\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"64\" y=\"74\" width=\"12\" height=\"32\""
            "    fill=\"none\" stroke=\"none\" stroke-width=\"10\"/>"

            "</svg>";

    SvgTester t (data);
    t.parser.setResolution(QRectF(0, 0, 600, 400) /* px */, 72 /* ppi */);
    t.run();

    KoShape *shape = t.findShape("testRect");
    QVERIFY(shape);

    QCOMPARE(shape->absoluteTransformation(0), QTransform::fromTranslate(4, 4) * QTransform::fromScale(0.5, 0.5));
    QCOMPARE(shape->outlineRect(), QRectF(0,0,12,32));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeftCorner), QPointF(2,2));
    QCOMPARE(shape->absolutePosition(KoFlake::TopRightCorner), QPointF(8,2));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomLeftCorner), QPointF(2,18));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRightCorner), QPointF(8,18));
}

void TestSvgParser::testScalingViewportKeepSlice2()
{
    const QString data =
            "<svg width=\"10px\" height=\"15px\" viewBox=\"60 70 20 40\""
            "    preserveAspectRatio=\" xMinYMin slice\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"64\" y=\"74\" width=\"12\" height=\"32\""
            "    fill=\"none\" stroke=\"none\" stroke-width=\"10\"/>"

            "</svg>";

    SvgTester t (data);
    t.parser.setResolution(QRectF(0, 0, 600, 400) /* px */, 72 /* ppi */);
    t.run();

    KoShape *shape = t.findShape("testRect");
    QVERIFY(shape);

    QCOMPARE(shape->absoluteTransformation(0), QTransform::fromTranslate(4, 4) * QTransform::fromScale(0.5, 0.5));
    QCOMPARE(shape->outlineRect(), QRectF(0,0,12,32));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeftCorner), QPointF(2,2));
    QCOMPARE(shape->absolutePosition(KoFlake::TopRightCorner), QPointF(8,2));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomLeftCorner), QPointF(2,18));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRightCorner), QPointF(8,18));
}

void TestSvgParser::testScalingViewportResolution()
{
    const QString data =
            "<svg width=\"10px\" height=\"20px\" viewBox=\"60 70 20 40\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"64\" y=\"74\" width=\"12\" height=\"32\""
            "    fill=\"none\" stroke=\"none\" stroke-width=\"10\"/>"

            "</svg>";

    SvgTester t (data);
    t.parser.setResolution(QRectF(0, 0, 600, 400) /* px */, 144 /* ppi */);
    t.run();

    KoShape *shape = t.findShape("testRect");
    QVERIFY(shape);

    QCOMPARE(shape->absoluteTransformation(0), QTransform::fromTranslate(4, 4) * QTransform::fromScale(0.25, 0.25));
    QCOMPARE(shape->outlineRect(), QRectF(0,0,12,32));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeftCorner), QPointF(1,1));
    QCOMPARE(shape->absolutePosition(KoFlake::TopRightCorner), QPointF(4,1));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomLeftCorner), QPointF(1,9));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRightCorner), QPointF(4,9));
}

void TestSvgParser::testScalingViewportPercentInternal()
{
    const QString data =
            "<svg width=\"10px\" height=\"20px\" viewBox=\"60 70 20 40\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"320%\" y=\"185%\" width=\"60%\" height=\"80%\""
            "    fill=\"none\" stroke=\"none\" stroke-width=\"10\"/>"

            "</svg>";

    SvgTester t (data);
    t.parser.setResolution(QRectF(0, 0, 600, 400) /* px */, 72 /* ppi */);
    t.run();

    KoShape *shape = t.findShape("testRect");
    QVERIFY(shape);

    QCOMPARE(shape->absoluteTransformation(0), QTransform::fromTranslate(4, 4) * QTransform::fromScale(0.5, 0.5));
    QCOMPARE(shape->outlineRect(), QRectF(0,0,12,32));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeftCorner), QPointF(2,2));
    QCOMPARE(shape->absolutePosition(KoFlake::TopRightCorner), QPointF(8,2));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomLeftCorner), QPointF(2,18));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRightCorner), QPointF(8,18));
}


void TestSvgParser::testParsePreserveAspectRatio()
{
    {
        SvgUtil::PreserveAspectRatioParser p(" defer  xMinYMax meet");
        QCOMPARE(p.defer, true);
        QCOMPARE(p.mode, Qt::KeepAspectRatio);
        QCOMPARE(p.xAlignment, SvgUtil::PreserveAspectRatioParser::Min);
        QCOMPARE(p.yAlignment, SvgUtil::PreserveAspectRatioParser::Max);
    }

    {
        SvgUtil::PreserveAspectRatioParser p(" xMinYMid slice");
        QCOMPARE(p.defer, false);
        QCOMPARE(p.mode, Qt::KeepAspectRatioByExpanding);
        QCOMPARE(p.xAlignment, SvgUtil::PreserveAspectRatioParser::Min);
        QCOMPARE(p.yAlignment, SvgUtil::PreserveAspectRatioParser::Middle);
    }

    {
        SvgUtil::PreserveAspectRatioParser p(" xmidYMid ");
        QCOMPARE(p.defer, false);
        QCOMPARE(p.mode, Qt::KeepAspectRatio);
        QCOMPARE(p.xAlignment, SvgUtil::PreserveAspectRatioParser::Middle);
        QCOMPARE(p.yAlignment, SvgUtil::PreserveAspectRatioParser::Middle);
    }

    {
        SvgUtil::PreserveAspectRatioParser p(" NoNe ");
        QCOMPARE(p.defer, false);
        QCOMPARE(p.mode, Qt::IgnoreAspectRatio);
        QCOMPARE(p.xAlignment, SvgUtil::PreserveAspectRatioParser::Min);
        QCOMPARE(p.yAlignment, SvgUtil::PreserveAspectRatioParser::Min);
    }

    {
        SvgUtil::PreserveAspectRatioParser p("defer NoNe ");
        QCOMPARE(p.defer, true);
        QCOMPARE(p.mode, Qt::IgnoreAspectRatio);
        QCOMPARE(p.xAlignment, SvgUtil::PreserveAspectRatioParser::Min);
        QCOMPARE(p.yAlignment, SvgUtil::PreserveAspectRatioParser::Min);
    }

    {
        SvgUtil::PreserveAspectRatioParser p("sweet brown fox jumps over a nice svg file");
        QCOMPARE(p.defer, false);
        QCOMPARE(p.mode, Qt::IgnoreAspectRatio);
        QCOMPARE(p.xAlignment, SvgUtil::PreserveAspectRatioParser::Min);
        QCOMPARE(p.yAlignment, SvgUtil::PreserveAspectRatioParser::Min);
    }
}

#include "parsers/SvgTransformParser.h"

void TestSvgParser::testParseTransform()
{
    {
        QString str("translate(-111.0, 33) translate(-111.0, 33) matrix (1 1 0 0 1, 3), translate(1)"
                    "scale(0.5) rotate(10) rotate(10, 3 3) skewX(1) skewY(2)");

        SvgTransformParser p(str);
        QCOMPARE(p.isValid(), true);
    }

    {
        // forget about one brace
        QString str("translate(-111.0, 33) translate(-111.0, 33 matrix (1 1 0 0 1, 3), translate(1)"
                    "scale(0.5) rotate(10) rotate(10, 3 3) skewX(1) skewY(2)");

        SvgTransformParser p(str);
        QCOMPARE(p.isValid(), false);
    }

    {
        SvgTransformParser p("translate(100, 50)");
        QCOMPARE(p.isValid(), true);
        QCOMPARE(p.transform(), QTransform::fromTranslate(100, 50));
    }

    {
        SvgTransformParser p("translate(100 50)");
        QCOMPARE(p.isValid(), true);
        QCOMPARE(p.transform(), QTransform::fromTranslate(100, 50));
    }

    {
        SvgTransformParser p("translate(100)");
        QCOMPARE(p.isValid(), true);
        QCOMPARE(p.transform(), QTransform::fromTranslate(100, 0));
    }

    {
        SvgTransformParser p("scale(100, 50)");
        QCOMPARE(p.isValid(), true);
        QCOMPARE(p.transform(), QTransform::fromScale(100, 50));
    }

    {
        SvgTransformParser p("scale(100)");
        QCOMPARE(p.isValid(), true);
        QCOMPARE(p.transform(), QTransform::fromScale(100, 100));
    }

    {
        SvgTransformParser p("rotate(90 70 74.0)");
        QCOMPARE(p.isValid(), true);
        QTransform t;
        t.rotate(90);
        t = QTransform::fromTranslate(-70, -74) * t * QTransform::fromTranslate(70, 74);
        qDebug() << ppVar(p.transform());
        QCOMPARE(p.transform(), t);
    }
}

void TestSvgParser::testScalingViewportTransform()
{
    /**
     * Note: 'transform' affects all the attributes of the *current*
     * element, while 'vewBox' affects only the decendants!
     */

    const QString data =
            "<svg width=\"5px\" height=\"10px\" viewBox=\"60 70 20 40\""
            "    transform=\"scale(2)\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"64\" y=\"74\" width=\"12\" height=\"32\""
            "    transform=\"translate(6)\""
            "    fill=\"none\" stroke=\"none\" stroke-width=\"10\"/>"

            "</svg>";

    SvgTester t (data);
    t.parser.setResolution(QRectF(0, 0, 600, 400) /* px */, 72 /* ppi */);
    t.run();

    KoShape *shape = t.findShape("testRect");
    QVERIFY(shape);

    QCOMPARE(shape->absoluteTransformation(0), QTransform::fromTranslate(10, 4) * QTransform::fromScale(0.5, 0.5));
    QCOMPARE(shape->outlineRect(), QRectF(0,0,12,32));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeftCorner), QPointF(5,2));
    QCOMPARE(shape->absolutePosition(KoFlake::TopRightCorner), QPointF(11,2));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomLeftCorner), QPointF(5,18));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRightCorner), QPointF(11,18));
}

void TestSvgParser::testTransformNesting()
{
    const QString data =
            "<svg width=\"10px\" height=\"20px\" viewBox=\"0 0 10 20\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"0\" y=\"0\" width=\"10\" height=\"20\""
            "    transform=\"translate(10,10), scale(2, 1)\""
            "    fill=\"none\" stroke=\"none\" stroke-width=\"10\"/>"

            "</svg>";

    SvgTester t (data);
    t.parser.setResolution(QRectF(0, 0, 600, 400) /* px */, 72 /* ppi */);
    t.run();

    KoShape *shape = t.findShape("testRect");
    QVERIFY(shape);

    QCOMPARE(shape->boundingRect(), QRectF(10 - 1,10 - 0.5, 20 + 2, 20 + 1));
    QCOMPARE(shape->outlineRect(), QRectF(0,0,10,20));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeftCorner), QPointF(10,10));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRightCorner), QPointF(30,30));
}

void TestSvgParser::testTransformRotation1()
{
    const QString data =
            "<svg width=\"10px\" height=\"20px\" viewBox=\"0 0 10 20\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"0\" y=\"0\" width=\"10\" height=\"20\""
            "    transform=\"rotate(90)\""
            "    fill=\"none\" stroke=\"none\" stroke-width=\"10\"/>"

            "</svg>";

    SvgTester t (data);
    t.parser.setResolution(QRectF(0, 0, 600, 400) /* px */, 72 /* ppi */);
    t.run();

    KoShape *shape = t.findShape("testRect");
    QVERIFY(shape);

    QCOMPARE(shape->boundingRect(), kisGrowRect(QRectF(-20,0,20,10), 0.5));
    QCOMPARE(shape->outlineRect(), QRectF(0,0,10,20));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeftCorner), QPointF(0,0));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRightCorner), QPointF(-20,10));
}

void TestSvgParser::testTransformRotation2()
{
    const QString data =
            "<svg width=\"10px\" height=\"20px\" viewBox=\"0 0 10 20\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"0\" y=\"0\" width=\"10\" height=\"20\""
            "    transform=\"rotate(-90 10 5)\""
            "    fill=\"none\" stroke=\"none\" stroke-width=\"10\"/>"

            "</svg>";

    SvgTester t (data);
    t.parser.setResolution(QRectF(0, 0, 600, 400) /* px */, 72 /* ppi */);
    t.run();

    KoShape *shape = t.findShape("testRect");
    QVERIFY(shape);

    QCOMPARE(shape->boundingRect(), kisGrowRect(QRectF(5,5,20,10), 0.5));
    QCOMPARE(shape->outlineRect(), QRectF(0,0,10,20));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeftCorner), QPointF(5,15));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRightCorner), QPointF(25,5));
}

#include "../../sdk/tests/qimage_test_util.h"

struct SvgRenderTester : public SvgTester
{
    SvgRenderTester(const QString &data)
        : SvgTester(data)
    {
    }

    void testRender(KoShape *shape, const QString &prefix, const QString &testName, const QSize canvasSize) {
        QImage canvas(canvasSize, QImage::Format_ARGB32);
        canvas.fill(0);
        KoViewConverter converter;
        KoShapePaintingContext context;
        QPainter painter(&canvas);

        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::NoBrush);
        painter.save();
        painter.setTransform(shape->absoluteTransformation(&converter) * painter.transform());
        shape->paint(painter, converter, context);
        if (shape->stroke()) {
            shape->stroke()->paint(shape, painter, converter);
        }
        painter.restore();

        QVERIFY(TestUtil::checkQImage(canvas, "svg_render", prefix, testName));
    }

    void test_standard_30px_72ppi(const QString &testName) {
        parser.setResolution(QRectF(0, 0, 30, 30) /* px */, 72 /* ppi */);
        run();

        KoShape *shape = findShape("testRect");
        QCOMPARE(shape->absolutePosition(KoFlake::TopLeftCorner), QPointF(5,5));
        QCOMPARE(shape->absolutePosition(KoFlake::BottomRightCorner), QPointF(15,25));

        testRender(shape, "load", testName, QSize(30,30));
    }
};

void TestSvgParser::testRenderStrokeNone()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"5\" y=\"5\" width=\"10\" height=\"20\""
            "    fill=\"cyan\" stroke=\"none\" stroke-width=\"1\"/>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("stroke_none");
}

void TestSvgParser::testRenderStrokeColorName()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"5\" y=\"5\" width=\"10\" height=\"20\""
            "    fill=\"cyan\" stroke=\"blue\" stroke-width=\"1\"/>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("stroke_blue");
}

void TestSvgParser::testRenderStrokeColorHex3()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"5\" y=\"5\" width=\"10\" height=\"20\""
            "    fill=\"cyan\" stroke=\"#00f\" stroke-width=\"1\"/>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("stroke_blue");
}

void TestSvgParser::testRenderStrokeColorHex6()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"5\" y=\"5\" width=\"10\" height=\"20\""
            "    fill=\"cyan\" stroke=\"#0000ff\" stroke-width=\"1\"/>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("stroke_blue");
}

void TestSvgParser::testRenderStrokeColorRgbValues()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"5\" y=\"5\" width=\"10\" height=\"20\""
            "    fill=\"cyan\" stroke=\"rgb(0, 0 ,255)\" stroke-width=\"1\"/>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("stroke_blue");
}

void TestSvgParser::testRenderStrokeColorRgbPercent()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"5\" y=\"5\" width=\"10\" height=\"20\""
            "    fill=\"cyan\" stroke=\"rgb(0, 0 ,100%)\" stroke-width=\"1\"/>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("stroke_blue");
}

void TestSvgParser::testRenderStrokeColorCurrent()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g color=\"blue\">"
            "    <rect id=\"testRect\" x=\"5\" y=\"5\" width=\"10\" height=\"20\""
            "        fill=\"cyan\" stroke=\"currentColor\" stroke-width=\"1\"/>"
            "</g>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("stroke_blue");
}

void TestSvgParser::testRenderStrokeColorNonexistentIri()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"5\" y=\"5\" width=\"10\" height=\"20\""
            "    fill=\"cyan\" stroke=\"url(notexists) blue\" stroke-width=\"1\"/>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("stroke_blue");
}

void TestSvgParser::testRenderStrokeWidth()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"5\" y=\"5\" width=\"10\" height=\"20\""
            "    fill=\"cyan\" stroke=\"blue\" stroke-width=\"2\"/>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("stroke_blue_width_2");
}

void TestSvgParser::testRenderStrokeZeroWidth()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"5\" y=\"5\" width=\"10\" height=\"20\""
            "    fill=\"cyan\" stroke=\"blue\" stroke-width=\"0\"/>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("stroke_none");
}

void TestSvgParser::testRenderStrokeOpacity()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"5\" y=\"5\" width=\"10\" height=\"20\""
            "    fill=\"cyan\" stroke=\"blue\" stroke-width=\"4\" stroke-opacity=\"0.3\"/>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("stroke_blue_0_3_opacity");
}

void TestSvgParser::testRenderStrokeJointRound()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"5\" y=\"5\" width=\"10\" height=\"20\""
            "    fill=\"cyan\" stroke=\"blue\" stroke-width=\"4\" stroke-linejoin=\"round\"/>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("stroke_blue_join_round");
}

void TestSvgParser::testRenderStrokeLinecap()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<polyline id=\"testRect\" points=\"5,5 10,25 15,5\""
            "    fill=\"none\" stroke=\"blue\" stroke-width=\"5\" stroke-linecap=\"round\"/>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("stroke_blue_linecap_round");
}

void TestSvgParser::testRenderStrokeMiterLimit()
{
    // TODO:seems like doesn't work!!
    qWarning() << "WARNING: Miter limit test is skipped!!!";
    return;

    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<polyline id=\"testRect\" points=\"5,5 10,25 15,5\""
            "    fill=\"none\" stroke=\"blue\" stroke-width=\"5\" stroke-miterlimit=\"1.114\"/>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("stroke_blue_miter_limit");
}

void TestSvgParser::testRenderStrokeDashArrayEven()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"5\" y=\"5\" width=\"10\" height=\"20\""
            "    fill=\"cyan\" stroke=\"blue\" stroke-width=\"2\" stroke-dasharray=\"3 2, 5 2\"/>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("stroke_blue_dasharray_even");
}

void TestSvgParser::testRenderStrokeDashArrayEvenOffset()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"5\" y=\"5\" width=\"10\" height=\"20\""
            "    fill=\"cyan\" stroke=\"blue\" stroke-width=\"2\" stroke-dasharray=\"3 2, 5 2\""
            "    stroke-dashoffset=\"1\"/>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("stroke_blue_dasharray_even_offset");
}

void TestSvgParser::testRenderStrokeDashArrayOdd()
{
    // SVG 1.1: if the dasharray is odd, repeat it

    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"5\" y=\"5\" width=\"10\" height=\"20\""
            "    fill=\"cyan\" stroke=\"blue\" stroke-width=\"2\" stroke-dasharray=\"3 2, 5\"/>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("stroke_blue_dasharray_odd");
}

void TestSvgParser::testRenderStrokeDashArrayRelative()
{
    // SVG 1.1: relative to view box
    // (40 x 50) * sqrt(2) => dash length = 5 px

    const QString data =
            "<svg width=\"42.4264px\" height=\"56.56854px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"5\" y=\"5\" width=\"10\" height=\"20\""
            "    fill=\"cyan\" stroke=\"blue\" stroke-width=\"2\" stroke-dasharray=\"10% 10%\"/>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("stroke_blue_dasharray_relative");
}

void TestSvgParser::testRenderFillDefault()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"5\" y=\"5\" width=\"10\" height=\"20\"/>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("fill_black");
}

void TestSvgParser::testRenderFillRuleNonZero()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<polyline id=\"testRect\" points=\"5,5 15,11 15,19 5,25 5,19 15,5 15,25 5,11 5,5\""
            "    fill=\"black\" fill-rule=\"nonzero\"/>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("fill_non_zero");
}

void TestSvgParser::testRenderFillRuleEvenOdd()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<polyline id=\"testRect\" points=\"5,5 15,11 15,19 5,25 5,19 15,5 15,25 5,11 5,5\""
            "    fill=\"black\" fill-rule=\"evenodd\"/>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("fill_even_odd");
}

void TestSvgParser::testRenderFillOpacity()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"5\" y=\"5\" width=\"10\" height=\"20\""
            "    fill=\"cyan\" fill-opacity=\"0.3\"/>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("fill_opacity_0_3");
}

void TestSvgParser::testRenderDisplayAttribute()
{
    const QString data =
            "<svg width=\"10px\" height=\"20px\" viewBox=\"0 0 10 20\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"0\" y=\"0\" width=\"10\" height=\"20\""
            "    fill=\"black\" display=\"none\"/>"

            "</svg>";

    SvgTester t (data);
    t.parser.setResolution(QRectF(0, 0, 600, 400) /* px */, 144 /* ppi */);
    t.run();

    KoShape *shape = t.findShape("testRect");
    QVERIFY(shape);

    QCOMPARE(shape->isVisible(), false);
}

void TestSvgParser::testRenderVisibilityAttribute()
{
    {
        const QString data =
                "<svg width=\"10px\" height=\"20px\" viewBox=\"0 0 10 20\""
                "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

                "<rect id=\"testRect\" x=\"0\" y=\"0\" width=\"10\" height=\"20\""
                "    fill=\"black\" visibility=\"visible\"/>"

                "</svg>";

        SvgTester t (data);
        t.parser.setResolution(QRectF(0, 0, 600, 400) /* px */, 144 /* ppi */);
        t.run();

        KoShape *shape = t.findShape("testRect");
        QVERIFY(shape);

        QCOMPARE(shape->isVisible(), true);
    }

    {
        const QString data =
                "<svg width=\"10px\" height=\"20px\" viewBox=\"0 0 10 20\""
                "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

                "<rect id=\"testRect\" x=\"0\" y=\"0\" width=\"10\" height=\"20\""
                "    fill=\"black\" visibility=\"hidden\"/>"

                "</svg>";

        SvgTester t (data);
        t.parser.setResolution(QRectF(0, 0, 600, 400) /* px */, 144 /* ppi */);
        t.run();

        KoShape *shape = t.findShape("testRect");
        QVERIFY(shape);

        QCOMPARE(shape->isVisible(), false);
    }

    {
        const QString data =
                "<svg width=\"10px\" height=\"20px\" viewBox=\"0 0 10 20\""
                "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

                "<rect id=\"testRect\" x=\"0\" y=\"0\" width=\"10\" height=\"20\""
                "    fill=\"black\" visibility=\"collapse\"/>"

                "</svg>";

        SvgTester t (data);
        t.parser.setResolution(QRectF(0, 0, 600, 400) /* px */, 144 /* ppi */);
        t.run();

        KoShape *shape = t.findShape("testRect");
        QVERIFY(shape);

        QCOMPARE(shape->isVisible(), false);
    }
}

void TestSvgParser::testRenderVisibilityInheritance()
{
    const QString data =
            "<svg width=\"10px\" height=\"20px\" viewBox=\"0 0 10 20\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g visibility=\"none\">"
            "    <rect id=\"testRect\" x=\"0\" y=\"0\" width=\"10\" height=\"20\""
            "        fill=\"black\" visibility=\"visible\"/>"
            "</g>"

            "</svg>";

    SvgTester t (data);
    t.parser.setResolution(QRectF(0, 0, 600, 400) /* px */, 144 /* ppi */);
    t.run();

    KoShape *shape = t.findShape("testRect");
    QVERIFY(shape);

    QCOMPARE(shape->isVisible(false), true);
    QCOMPARE(shape->isVisible(true), false);
}

void TestSvgParser::testRenderDisplayInheritance()
{
    const QString data =
            "<svg width=\"10px\" height=\"20px\" viewBox=\"0 0 10 20\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g display=\"none\">"
            "    <rect id=\"testRect\" x=\"0\" y=\"0\" width=\"10\" height=\"20\""
            "        fill=\"black\" visibility=\"visible\"/>"
            "</g>"

            "</svg>";

    SvgTester t (data);
    t.parser.setResolution(QRectF(0, 0, 600, 400) /* px */, 144 /* ppi */);
    t.run();

    KoShape *shape = t.findShape("testRect");
    QVERIFY(shape);

    QCOMPARE(shape->isVisible(false), true);
    QEXPECT_FAIL("", "TODO: Fix 'display' attribute not to be inherited in shapes heirarchy!", Continue);
    QCOMPARE(shape->isVisible(true), true);
}

void TestSvgParser::testRenderStrokeWithInlineStyle()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<rect id=\"testRect\" x=\"5\" y=\"5\" width=\"10\" height=\"20\""
            "    style = \"fill: cyan; stroke :blue; stroke-width:2;\"/>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("stroke_blue_width_2");
}

void TestSvgParser::testIccColor()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g xml:base=\"icc\">"
            "    <color-profile xlink:href=\"sRGB-elle-V4-srgbtrc.icc\""
            "        local=\"133a66607cffeebdd64dd433ada9bf4e\" name=\"default-profile\"/>"

            "    <color-profile xlink:href=\"sRGB-elle-V4-srgbtrc.icc\""
            "        local=\"133a66607cffeebdd64dd433ada9bf4e\" name=\"some-other-name\"/>"

            "    <rect id=\"testRect\" x=\"5\" y=\"5\" width=\"10\" height=\"20\""
            "        style = \"fill: cyan; stroke :blue; stroke-width:2;\"/>"
            "</g>"

            "</svg>";

    SvgRenderTester t (data);

    int numFetches = 0;

    t.parser.setFileFetcher(
        [&numFetches](const QString &name) {
            numFetches++;
            const QString fileName = TestUtil::fetchDataFileLazy(name);
            QFile file(fileName);
            KIS_ASSERT(file.exists());
            file.open(QIODevice::ReadOnly);
            return file.readAll();
        });

    t.test_standard_30px_72ppi("stroke_blue_width_2");
    QCOMPARE(numFetches, 1);
}

QTEST_GUILESS_MAIN(TestSvgParser)
