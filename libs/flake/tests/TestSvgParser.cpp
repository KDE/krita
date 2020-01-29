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
#include <svg/SvgUtil.h>
#include <KoShapeStrokeModel.h>


#include "SvgParserTestingUtils.h"

#include "../../sdk/tests/qimage_test_util.h"

#ifdef USE_ROUND_TRIP
#include "SvgWriter.h"
#include <QBuffer>
#include <QDomDocument>
#endif


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

    QCOMPARE(shape->boundingRect(), QRectF(0,0,10,20));
    QCOMPARE(shape->absoluteTransformation(), QTransform());
    QCOMPARE(shape->outlineRect(), QRectF(0,0,10,20));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeft), QPointF(0,0));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRight), QPointF(10,20));
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

    QCOMPARE(shape->boundingRect(), QRectF(0,0,5,10));
    QCOMPARE(shape->absoluteTransformation(), QTransform::fromScale(0.5, 0.5));
    QCOMPARE(shape->outlineRect(), QRectF(0,0,10,20));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeft), QPointF(0,0));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRight), QPointF(5,10));
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

    QCOMPARE(shape->boundingRect(), kisGrowRect(QRectF(0,0,10,20), 0));
    QCOMPARE(shape->absoluteTransformation(), QTransform());
    QCOMPARE(shape->outlineRect(), QRectF(0,0,10,20));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeft), QPointF(0,0));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRight), QPointF(10,20));
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

    QCOMPARE(shape->boundingRect(), QRectF(0,0,720,1440));
    QCOMPARE(shape->absoluteTransformation(), QTransform::fromScale(72, 72));
    QCOMPARE(shape->outlineRect(), QRectF(0,0,10,20));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeft), QPointF(0,0));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRight), QPointF(720,1440));
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

    QCOMPARE(shape->boundingRect(), QRectF(0,0,5,10));
    QCOMPARE(shape->absoluteTransformation(), QTransform::fromScale(0.5, 0.5));
    QCOMPARE(shape->outlineRect(), QRectF(0,0,10,20));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeft), QPointF(0,0));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRight), QPointF(5,10));
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

    QCOMPARE(shape->absoluteTransformation(), QTransform::fromTranslate(4, 4) * QTransform::fromScale(0.5, 0.5));
    QCOMPARE(shape->outlineRect(), QRectF(0,0,12,32));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeft), QPointF(2,2));
    QCOMPARE(shape->absolutePosition(KoFlake::TopRight), QPointF(8,2));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomLeft), QPointF(2,18));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRight), QPointF(8,18));
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

    QCOMPARE(shape->absoluteTransformation(), QTransform::fromTranslate(4, 4) * QTransform::fromScale(0.5, 0.5));
    QCOMPARE(shape->outlineRect(), QRectF(0,0,12,32));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeft), QPointF(2,2));
    QCOMPARE(shape->absolutePosition(KoFlake::TopRight), QPointF(8,2));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomLeft), QPointF(2,18));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRight), QPointF(8,18));
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

    QCOMPARE(shape->absoluteTransformation(), QTransform::fromTranslate(4, 4) * QTransform::fromScale(0.5, 0.5));
    QCOMPARE(shape->outlineRect(), QRectF(0,0,12,32));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeft), QPointF(2,2));
    QCOMPARE(shape->absolutePosition(KoFlake::TopRight), QPointF(8,2));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomLeft), QPointF(2,18));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRight), QPointF(8,18));
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

    QCOMPARE(shape->absoluteTransformation(), QTransform::fromTranslate(4, 24) * QTransform::fromScale(0.5, 0.5));
    QCOMPARE(shape->outlineRect(), QRectF(0,0,12,32));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeft), QPointF(2,12));
    QCOMPARE(shape->absolutePosition(KoFlake::TopRight), QPointF(8,12));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomLeft), QPointF(2,28));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRight), QPointF(8,28));
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

    QCOMPARE(shape->absoluteTransformation(), QTransform::fromTranslate(4, 4) * QTransform::fromScale(0.5, 0.5));
    QCOMPARE(shape->outlineRect(), QRectF(0,0,12,32));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeft), QPointF(2,2));
    QCOMPARE(shape->absolutePosition(KoFlake::TopRight), QPointF(8,2));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomLeft), QPointF(2,18));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRight), QPointF(8,18));
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

    QCOMPARE(shape->absoluteTransformation(), QTransform::fromTranslate(4, 4) * QTransform::fromScale(0.5, 0.5));
    QCOMPARE(shape->outlineRect(), QRectF(0,0,12,32));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeft), QPointF(2,2));
    QCOMPARE(shape->absolutePosition(KoFlake::TopRight), QPointF(8,2));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomLeft), QPointF(2,18));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRight), QPointF(8,18));
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

    QCOMPARE(shape->absoluteTransformation(), QTransform::fromTranslate(4, 4) * QTransform::fromScale(0.25, 0.25));
    QCOMPARE(shape->outlineRect(), QRectF(0,0,12,32));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeft), QPointF(1,1));
    QCOMPARE(shape->absolutePosition(KoFlake::TopRight), QPointF(4,1));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomLeft), QPointF(1,9));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRight), QPointF(4,9));
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

    QCOMPARE(shape->absoluteTransformation(), QTransform::fromTranslate(4, 4) * QTransform::fromScale(0.5, 0.5));
    QCOMPARE(shape->outlineRect(), QRectF(0,0,12,32));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeft), QPointF(2,2));
    QCOMPARE(shape->absolutePosition(KoFlake::TopRight), QPointF(8,2));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomLeft), QPointF(2,18));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRight), QPointF(8,18));
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
     * element, while 'viewBox' affects only the descendants!
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

    QCOMPARE(shape->absoluteTransformation(), QTransform::fromTranslate(10, 4) * QTransform::fromScale(0.5, 0.5));
    QCOMPARE(shape->outlineRect(), QRectF(0,0,12,32));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeft), QPointF(5,2));
    QCOMPARE(shape->absolutePosition(KoFlake::TopRight), QPointF(11,2));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomLeft), QPointF(5,18));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRight), QPointF(11,18));
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

    QCOMPARE(shape->boundingRect(), QRectF(10,10,20,20));
    QCOMPARE(shape->outlineRect(), QRectF(0,0,10,20));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeft), QPointF(10,10));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRight), QPointF(30,30));
}

void TestSvgParser::testTransformNestingGroups()
{
    const QString data =
            "<svg width=\"10px\" height=\"20px\" viewBox=\"0 0 10 20\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g transform=\"translate(10,10)\">"
            "    <rect id=\"testRect\" x=\"0\" y=\"0\" width=\"10\" height=\"20\""
            "        transform=\"scale(2, 1)\""
            "        fill=\"none\" stroke=\"none\" stroke-width=\"10\"/>"
            "</g>"

            "</svg>";

    SvgTester t (data);
    t.parser.setResolution(QRectF(0, 0, 600, 400) /* px */, 72 /* ppi */);
    t.run();

    KoShape *shape = t.findShape("testRect");
    QVERIFY(shape);

    QCOMPARE(shape->boundingRect(), QRectF(10,10, 20, 20));
    QCOMPARE(shape->outlineRect(), QRectF(0,0,10,20));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeft), QPointF(10,10));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRight), QPointF(30,30));
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

    QCOMPARE(shape->boundingRect(), QRectF(-20,0,20,10));
    QCOMPARE(shape->outlineRect(), QRectF(0,0,10,20));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeft), QPointF(0,0));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRight), QPointF(-20,10));
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

    QCOMPARE(shape->boundingRect(), QRectF(5,5,20,10));
    QCOMPARE(shape->outlineRect(), QRectF(0,0,10,20));
    QCOMPARE(shape->absolutePosition(KoFlake::TopLeft), QPointF(5,15));
    QCOMPARE(shape->absolutePosition(KoFlake::BottomRight), QPointF(25,5));
}



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
    t.setFuzzyThreshold(1);
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
    t.setFuzzyThreshold(1);
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
    QEXPECT_FAIL("", "TODO: Fix 'display' attribute not to be inherited in shapes hierarchy!", Continue);
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

void TestSvgParser::testRenderFillLinearGradientRelativePercent()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<defs>"
            "    <linearGradient id=\"testGrad\" x1=\"0%\" y1=\"50%\" x2=\"100%\" y2=\"50%\">"
            "        <stop offset=\"20%\" stop-color=\"#F60\" />"
            "        <stop offset=\"80%\" stop-color=\"#FF6\" />"
            "    </linearGradient>"
            "</defs>"

            "<rect id=\"testRect\" x=\"5\" y=\"5\" width=\"10\" height=\"20\""
            "    style = \"fill:url(#testGrad) magenta; stroke:none; stroke-width:2;\"/>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("fill_gradient");
}

void TestSvgParser::testRenderFillLinearGradientRelativePortion()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<linearGradient id=\"testGrad\" x1=\"0\" y1=\"0.5\" x2=\"1.0\" y2=\"0.5\">"
            "    <stop offset=\"20%\" stop-color=\"#F60\" />"
            "    <stop offset=\"80%\" stop-color=\"#FF6\" />"
            "</linearGradient>"

            "<rect id=\"testRect\" x=\"5\" y=\"5\" width=\"10\" height=\"20\""
            "    style = \"fill:url(#testGrad) magenta; stroke:none; stroke-width:2;\"/>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("fill_gradient");
}

void TestSvgParser::testRenderFillLinearGradientUserCoord()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\" viewBox=\"60 70 60 90\""
            "        preserveAspectRatio=\"none meet\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<linearGradient id=\"testGrad\" x1=\"70\" y1=\"115\" x2=\"90\" y2=\"115\""
            "    gradientUnits=\"userSpaceOnUse\">"
            "    <stop offset=\"20%\" stop-color=\"#F60\" />"
            "    <stop offset=\"80%\" stop-color=\"#FF6\" />"
            "</linearGradient>"

            "<rect id=\"testRect\" x=\"70\" y=\"85\" width=\"20\" height=\"60\""
            "    fill=\"url(#testGrad) magenta\" stroke=\"none\"/>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("fill_gradient");
}

void TestSvgParser::testRenderFillLinearGradientStopPortion()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<linearGradient id=\"testGrad\" x1=\"0\" y1=\"0.5\" x2=\"1.0\" y2=\"0.5\">"
            "    <stop offset=\"0.2\" stop-color=\"#F60\" />"
            "    <stop offset=\"0.8\" stop-color=\"#FF6\" />"
            "</linearGradient>"

            "<rect id=\"testRect\" x=\"5\" y=\"5\" width=\"10\" height=\"20\""
            "    style = \"fill:url(#testGrad) magenta; stroke:none; stroke-width:2;\"/>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("fill_gradient");
}

void TestSvgParser::testRenderFillLinearGradientTransform()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<linearGradient id=\"testGrad\" x1=\"0\" y1=\"0.5\" x2=\"1.0\" y2=\"0.5\""
            "    gradientTransform=\"rotate(90, 0.5, 0.5)\">"

            "    <stop offset=\"0.2\" stop-color=\"#F60\" />"
            "    <stop offset=\"0.8\" stop-color=\"#FF6\" />"
            "</linearGradient>"

            "<rect id=\"testRect\" x=\"5\" y=\"5\" width=\"10\" height=\"20\""
            "    style = \"fill:url(#testGrad) magenta; stroke:none; stroke-width:2;\"/>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("fill_gradient_vertical");
}

void TestSvgParser::testRenderFillLinearGradientTransformUserCoord()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\" viewBox=\"60 70 60 90\""
            "        preserveAspectRatio=\"none meet\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<linearGradient id=\"testGrad\" x1=\"70\" y1=\"115\" x2=\"90\" y2=\"115\""
            "    gradientUnits=\"userSpaceOnUse\""
            "    gradientTransform=\"rotate(90, 80, 115)\">"

            "    <stop offset=\"20%\" stop-color=\"#F60\" />"
            "    <stop offset=\"80%\" stop-color=\"#FF6\" />"
            "</linearGradient>"

            "<rect id=\"testRect\" x=\"70\" y=\"85\" width=\"20\" height=\"60\""
            "    fill=\"url(#testGrad) magenta\" stroke=\"none\"/>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("fill_gradient_vertical_in_user");
}

void TestSvgParser::testRenderFillLinearGradientRotatedShape()
{
    // DK: I'm not sure I fully understand if it is a correct transformation,
    //     but inkscape opens the file in the same way...

    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<linearGradient id=\"testGrad\" x1=\"0\" y1=\"0.5\" x2=\"1.0\" y2=\"0.5\""
            "    gradientTransform=\"rotate(90, 0.5, 0.5)\">"

            "    <stop offset=\"0.2\" stop-color=\"#F60\" />"
            "    <stop offset=\"0.8\" stop-color=\"#FF6\" />"
            "</linearGradient>"

            "<rect id=\"testRect\" x=\"5\" y=\"5\" width=\"10\" height=\"20\""
            "    style = \"fill:url(#testGrad) magenta; stroke:none; stroke-width:2;\""
            "    transform=\"rotate(90, 10, 12.5)\"/>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("fill_gradient_shape_rotated", false);
}

void TestSvgParser::testRenderFillLinearGradientRotatedShapeUserCoord()
{
    // DK: I'm not sure I fully understand if it is a correct transformation,
    //     but inkscape opens the file in the same way...

    const QString data =
            "<svg width=\"30px\" height=\"30px\" viewBox=\"60 70 60 90\""
            "        preserveAspectRatio=\"none meet\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<linearGradient id=\"testGrad\" x1=\"70\" y1=\"115\" x2=\"90\" y2=\"115\""
            "    gradientUnits=\"userSpaceOnUse\">"

            "    <stop offset=\"20%\" stop-color=\"#F60\" />"
            "    <stop offset=\"80%\" stop-color=\"#FF6\" />"
            "</linearGradient>"

            "<rect id=\"testRect\" x=\"70\" y=\"85\" width=\"20\" height=\"60\""
            "    fill=\"url(#testGrad) magenta\" stroke=\"none\""
            "    transform=\"rotate(90, 80, 115)\"/>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("fill_gradient_shape_rotated_in_user", false);
}

void TestSvgParser::testRenderFillRadialGradient()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<radialGradient id=\"testGrad\" cx=\"0.5\" cy=\"0.5\" fx=\"0.2\" fy=\"0.2\" r=\"0.5\">"
            "    <stop offset=\"0.2\" stop-color=\"#F60\" />"
            "    <stop offset=\"0.8\" stop-color=\"#FF6\" />"
            "</radialGradient>"

            "<rect id=\"testRect\" x=\"5\" y=\"5\" width=\"10\" height=\"20\""
            "    style = \"fill:url(#testGrad) magenta; stroke:none; stroke-width:2;\"/>"
            "</svg>";

    SvgRenderTester t (data);
    t.setFuzzyThreshold(1);
    t.test_standard_30px_72ppi("fill_gradient_radial");
}

void TestSvgParser::testRenderFillRadialGradientUserCoord()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<radialGradient id=\"testGrad\" cx=\"10\" cy=\"12.5\" fx=\"7\" fy=\"9\" r=\"5\""
            "    gradientUnits=\"userSpaceOnUse\">"

            "    <stop offset=\"0.2\" stop-color=\"#F60\" />"
            "    <stop offset=\"0.8\" stop-color=\"#FF6\" />"
            "</radialGradient>"

            "<rect id=\"testRect\" x=\"5\" y=\"5\" width=\"10\" height=\"20\""
            "    style = \"fill:url(#testGrad) magenta; stroke:none; stroke-width:2;\"/>"
            "</svg>";

    SvgRenderTester t (data);
    t.setFuzzyThreshold(1);
    t.test_standard_30px_72ppi("fill_gradient_radial_in_user");
}

void TestSvgParser::testRenderFillLinearGradientUserCoordPercent()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\" viewBox=\"60 70 60 90\""
            "        preserveAspectRatio=\"none meet\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<linearGradient id=\"testGrad\" x1=\"116.667%\" y1=\"127.778%\" x2=\"150%\" y2=\"127.778%\""
            "    gradientUnits=\"userSpaceOnUse\">"
            "    <stop offset=\"20%\" stop-color=\"#F60\" />"
            "    <stop offset=\"80%\" stop-color=\"#FF6\" />"
            "</linearGradient>"

            "<rect id=\"testRect\" x=\"70\" y=\"85\" width=\"20\" height=\"60\""
            "    fill=\"url(#testGrad) magenta\" stroke=\"none\"/>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("fill_gradient");
}

void TestSvgParser::testRenderStrokeLinearGradient()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<defs>"
            "    <linearGradient id=\"testGrad\" x1=\"0%\" y1=\"50%\" x2=\"100%\" y2=\"50%\">"
            "        <stop offset=\"20%\" stop-color=\"#F60\" />"
            "        <stop offset=\"80%\" stop-color=\"#FF6\" />"
            "    </linearGradient>"
            "</defs>"

            "<rect id=\"testRect\" x=\"5\" y=\"5\" width=\"10\" height=\"20\""
            "    style = \"grey; stroke:url(#testGrad); stroke-width:3; stroke-dasharray:3,1\"/>"

            "</svg>";

    SvgRenderTester t (data);
    t.test_standard_30px_72ppi("stroke_gradient_dashed");
}

QTransform rotateTransform(qreal degree, const QPointF &center) {
    QTransform rotate;
    rotate.rotate(degree);
    return
        QTransform::fromTranslate(-center.x(), -center.y()) *
        rotate *
        QTransform::fromTranslate(center.x(), center.y());
}

QTransform viewTransform(const QRectF &src, const QRectF &dst) {
    return QTransform::fromTranslate(-src.x(), -src.y()) *
            QTransform::fromScale(dst.width() / src.width(),
                                  dst.height() / src.height()) *
            QTransform::fromTranslate(dst.x(), dst.y());

}

QPainterPath bakeShape(const QPainterPath &path,
                       const QTransform &bakeTransform,
                       bool contentIsObb = false, const QRectF &shapeBoundingRect = QRectF(),
                       bool contentIsViewBox = false, const QRectF &viewBoxRect= QRectF(), const QRectF &refRect = QRectF())
{
    const QTransform relativeToShape(shapeBoundingRect.width(), 0, 0, shapeBoundingRect.height(),
                                     shapeBoundingRect.x(), shapeBoundingRect.y());

    QTransform newTransform = bakeTransform;

    if (contentIsObb) {
        newTransform = relativeToShape * newTransform;
    }

    if (contentIsViewBox) {
        newTransform = viewTransform(viewBoxRect, refRect) * newTransform;
    }

    return newTransform.map(path);
}

#include <KoBakedShapeRenderer.h>

void renderBakedPath(QPainter &painter,
                     const QPainterPath &bakedFillPath, const QTransform &bakedTransform,
                     const QRect &shapeOutline, const QTransform &shapeTransform,
                     const QRectF &referenceRect,
                     bool contentIsObb, const QRectF &bakedShapeBoundingRect,
                     bool referenceIsObb,
                     const QTransform &patternTransform,
                     QImage *stampResult)
{
    painter.setTransform(QTransform());
    painter.setPen(Qt::NoPen);

    QPainterPath shapeOutlinePath;
    shapeOutlinePath.addRect(shapeOutline);

    KoBakedShapeRenderer renderer(
                shapeOutlinePath,
                shapeTransform,
                bakedTransform,
                referenceRect,
                contentIsObb, bakedShapeBoundingRect,
                referenceIsObb,
                patternTransform);

    QPainter *patchPainter = renderer.bakeShapePainter();
    patchPainter->fillPath(bakedFillPath, Qt::blue);
    patchPainter->end();

    renderer.renderShape(painter);

    if (stampResult) {
        *stampResult = renderer.patchImage();
    }
}

void TestSvgParser::testManualRenderPattern_ContentUser_RefObb()
{
    const QRectF referenceRect(0, 0, 1.0, 0.5);

    QPainterPath fillPath;
    fillPath.addRect(QRect(2, 2, 6, 6));
    fillPath.addRect(QRect(8, 4, 3, 2));

    QTransform bakedTransform = QTransform::fromTranslate(10, 10) * QTransform::fromScale(2, 2);
    QPainterPath bakedFillPath = bakeShape(fillPath, bakedTransform);

    QRect shape1OutlineRect(0,0,10,20);

    QImage stampResult;
    QImage fillResult(QSize(60,60), QImage::Format_ARGB32);
    QPainter gc(&fillResult);

    fillResult.fill(0);
    renderBakedPath(gc,
                    bakedFillPath, bakedTransform,
                    shape1OutlineRect, bakedTransform,
                    referenceRect,
                    false, QRectF(),
                    true,
                    QTransform(),
                    &stampResult);

    QVERIFY(TestUtil::checkQImage(stampResult, "svg_render", "render", "pattern_c_user_r_obb_patch1"));
    QVERIFY(TestUtil::checkQImage(fillResult, "svg_render", "render", "pattern_c_user_r_obb_fill1"));

    QRect shape2OutlineRect(5,5,20,10);
    QTransform shape2Transform = QTransform::fromScale(2, 2) * QTransform::fromTranslate(5, 5);

    fillResult.fill(0);
    renderBakedPath(gc,
                    bakedFillPath, bakedTransform,
                    shape2OutlineRect, shape2Transform,
                    referenceRect,
                    false, QRectF(),
                    true,
                    QTransform(),
                    &stampResult);

    QVERIFY(TestUtil::checkQImage(stampResult, "svg_render", "render", "pattern_c_user_r_obb_patch2"));
    QVERIFY(TestUtil::checkQImage(fillResult, "svg_render", "render", "pattern_c_user_r_obb_fill2"));
}

void TestSvgParser::testManualRenderPattern_ContentObb_RefObb()
{
    const QRectF referenceRect(0.3, 0.3, 0.4, 0.4);

    QPainterPath fillPath;
    fillPath.addRect(QRectF(0.4, 0.4, 0.2, 0.2));
    fillPath.addRect(QRectF(0.6, 0.5, 0.1, 0.1));
    fillPath.addRect(QRectF(0.3, 0.4, 0.1, 0.1));


    const QRect bakedShapeRect(2,2,10,10);
    QTransform bakedTransform = QTransform::fromTranslate(10, 10) * QTransform::fromScale(2, 2);

    QPainterPath bakedFillPath = bakeShape(fillPath, bakedTransform, true, bakedShapeRect);

    QImage stampResult;
    QImage fillResult(QSize(60,60), QImage::Format_ARGB32);
    QPainter gc(&fillResult);

    // Round trip to the same shape

    fillResult.fill(0);
    renderBakedPath(gc,
                    bakedFillPath, bakedTransform,
                    bakedShapeRect, bakedTransform,
                    referenceRect,
                    true, bakedShapeRect,
                    true,
                    QTransform(),
                    &stampResult);

    QVERIFY(TestUtil::checkQImage(stampResult, "svg_render", "render", "pattern_c_obb_r_obb_patch1"));
    QVERIFY(TestUtil::checkQImage(fillResult, "svg_render", "render", "pattern_c_obb_r_obb_fill1"));

    // Move to a different shape

    QRect shape2OutlineRect(5,5,20,10);
    QTransform shape2Transform = QTransform::fromScale(2, 2) * QTransform::fromTranslate(5, 5);

    fillResult.fill(0);
    renderBakedPath(gc,
                    bakedFillPath, bakedTransform,
                    shape2OutlineRect, shape2Transform,
                    referenceRect,
                    true, bakedShapeRect,
                    true,
                    QTransform(),
                    &stampResult);

    QVERIFY(TestUtil::checkQImage(stampResult, "svg_render", "render", "pattern_c_obb_r_obb_patch2"));
    QVERIFY(TestUtil::checkQImage(fillResult, "svg_render", "render", "pattern_c_obb_r_obb_fill2"));
}

void TestSvgParser::testManualRenderPattern_ContentUser_RefUser()
{
    const QRectF referenceRect(5, 2, 8, 8);

    QPainterPath fillPath;
    fillPath.addRect(QRect(2, 2, 6, 6));
    fillPath.addRect(QRect(8, 4, 3, 2));


    QTransform bakedTransform = QTransform::fromTranslate(10, 10) * QTransform::fromScale(2, 2);
    QPainterPath bakedFillPath = bakeShape(fillPath, bakedTransform);

    QRect shape1OutlineRect(0,0,10,20);

    QImage stampResult;
    QImage fillResult(QSize(60,60), QImage::Format_ARGB32);
    QPainter gc(&fillResult);

    fillResult.fill(0);
    renderBakedPath(gc,
                    bakedFillPath, bakedTransform,
                    shape1OutlineRect, bakedTransform,
                    referenceRect,
                    false, QRectF(),
                    false,
                    QTransform(),
                    &stampResult);

    QVERIFY(TestUtil::checkQImage(stampResult, "svg_render", "render", "pattern_c_user_r_user_patch1"));
    QVERIFY(TestUtil::checkQImage(fillResult, "svg_render", "render", "pattern_c_user_r_user_fill1"));

    QRect shape2OutlineRect(5,5,20,10);
    QTransform shape2Transform = QTransform::fromScale(2, 2) * QTransform::fromTranslate(5, 5);

    fillResult.fill(0);
    renderBakedPath(gc,
                    bakedFillPath, bakedTransform,
                    shape2OutlineRect, shape2Transform,
                    referenceRect,
                    false, QRectF(),
                    false,
                    QTransform(),
                    &stampResult);

    QVERIFY(TestUtil::checkQImage(stampResult, "svg_render", "render", "pattern_c_user_r_user_patch2"));
    QVERIFY(TestUtil::checkQImage(fillResult, "svg_render", "render", "pattern_c_user_r_user_fill2"));
}

void TestSvgParser::testManualRenderPattern_ContentObb_RefObb_Transform_Rotate()
{
    const QRectF referenceRect(0.0, 0.0, 0.4, 0.2);

    QPainterPath fillPath;
    fillPath.addRect(QRectF(0.0, 0.0, 0.5, 0.1));
    fillPath.addRect(QRectF(0.0, 0.1, 0.1, 0.1));

    const QRect bakedShapeRect(2,1,10,10);
    QTransform bakedTransform = QTransform::fromScale(2, 2) * QTransform::fromTranslate(10,10);

    QPainterPath bakedFillPath = bakeShape(fillPath, bakedTransform, true, bakedShapeRect);

    QImage stampResult;
    QImage fillResult(QSize(60,60), QImage::Format_ARGB32);
    QPainter gc(&fillResult);

    QTransform patternTransform;
    patternTransform.rotate(90);
    patternTransform = patternTransform * QTransform::fromTranslate(0.5, 0.0);

    // Round trip to the same shape

    fillResult.fill(0);
    renderBakedPath(gc,
                    bakedFillPath, bakedTransform,
                    bakedShapeRect, bakedTransform,
                    referenceRect,
                    true, bakedShapeRect,
                    true,
                    patternTransform,
                    &stampResult);

    QVERIFY(TestUtil::checkQImage(stampResult, "svg_render", "render", "pattern_c_obb_r_obb_rotate_patch1"));
    QVERIFY(TestUtil::checkQImage(fillResult, "svg_render", "render", "pattern_c_obb_r_obb_rotate_fill1"));

    QRect shape2OutlineRect(5,5,20,10);
    QTransform shape2Transform = QTransform::fromScale(2, 2) * QTransform::fromTranslate(5, 5);

    fillResult.fill(0);
    renderBakedPath(gc,
                    bakedFillPath, bakedTransform,
                    shape2OutlineRect, shape2Transform,
                    referenceRect,
                    true, bakedShapeRect,
                    true,
                    patternTransform,
                    &stampResult);

    QVERIFY(TestUtil::checkQImage(stampResult, "svg_render", "render", "pattern_c_obb_r_obb_rotate_patch2"));
    QVERIFY(TestUtil::checkQImage(fillResult, "svg_render", "render", "pattern_c_obb_r_obb_rotate_fill2"));
}


void TestSvgParser::testManualRenderPattern_ContentView_RefObb()
{
    const QRectF referenceRect(0, 0, 0.5, 1.0/3.0);
    const QRectF viewRect(10,10,60,90);

    QPainterPath fillPath;
    fillPath.addRect(QRect(30, 10, 20, 60));
    fillPath.addRect(QRect(50, 40, 20, 30));


    QRect shape1OutlineRect(10,20,40,120);

    QTransform bakedTransform = QTransform::fromScale(2, 0.5) * QTransform::fromTranslate(40,30);
    QPainterPath bakedFillPath = bakeShape(fillPath, bakedTransform,
                                           true, shape1OutlineRect,
                                           true, viewRect, referenceRect);

    QImage stampResult;
    QImage fillResult(QSize(220,160), QImage::Format_ARGB32);
    QPainter gc(&fillResult);

    fillResult.fill(0);
    renderBakedPath(gc,
                    bakedFillPath, bakedTransform,
                    shape1OutlineRect, bakedTransform,
                    referenceRect,
                    true, shape1OutlineRect,
                    true,
                    QTransform(),
                    &stampResult);

    QVERIFY(TestUtil::checkQImage(stampResult, "svg_render", "render", "pattern_c_view_r_obb_patch1"));
    QVERIFY(TestUtil::checkQImage(fillResult, "svg_render", "render", "pattern_c_view_r_obb_fill1"));

    QRect shape2OutlineRect(20,10,60,90);
    QTransform shape2Transform = QTransform::fromScale(2, 1) * QTransform::fromTranslate(50, 50);

    fillResult.fill(0);
    renderBakedPath(gc,
                    bakedFillPath, bakedTransform,
                    shape2OutlineRect, shape2Transform,
                    referenceRect,
                    true, shape1OutlineRect,
                    true,
                    rotateTransform(90, QPointF(0, 1.0 / 3.0)),
                    &stampResult);

    QVERIFY(TestUtil::checkQImage(stampResult, "svg_render", "render", "pattern_c_view_r_obb_patch2"));
    QVERIFY(TestUtil::checkQImage(fillResult, "svg_render", "render", "pattern_c_view_r_obb_fill2"));
}

void TestSvgParser::testManualRenderPattern_ContentView_RefUser()
{
    const QRectF referenceRect(60, 0, 30, 20);
    const QRectF viewRect(10,10,60,90);

    QPainterPath fillPath;
    fillPath.addRect(QRect(30, 10, 20, 60));
    fillPath.addRect(QRect(50, 40, 20, 30));


    QRect shape1OutlineRect(10,20,40,120);

    QTransform bakedTransform = QTransform::fromScale(2, 0.5) * QTransform::fromTranslate(40,30);
    QPainterPath bakedFillPath = bakeShape(fillPath, bakedTransform,
                                           false, shape1OutlineRect,
                                           true, viewRect, referenceRect);

    QImage stampResult;
    QImage fillResult(QSize(220,160), QImage::Format_ARGB32);
    QPainter gc(&fillResult);

    fillResult.fill(0);
    renderBakedPath(gc,
                    bakedFillPath, bakedTransform,
                    shape1OutlineRect, bakedTransform,
                    referenceRect,
                    false, shape1OutlineRect,
                    false,
                    QTransform(),
                    &stampResult);

    QVERIFY(TestUtil::checkQImage(stampResult, "svg_render", "render", "pattern_c_view_r_user_patch1"));
    QVERIFY(TestUtil::checkQImage(fillResult, "svg_render", "render", "pattern_c_view_r_user_fill1"));

    QRect shape2OutlineRect(20,10,60,90);
    QTransform shape2Transform = QTransform::fromScale(2, 1) * QTransform::fromTranslate(50, 50);

    QTransform patternTransform2 = rotateTransform(90, QPointF()) * QTransform::fromTranslate(40, 10);

    fillResult.fill(0);
    renderBakedPath(gc,
                    bakedFillPath, bakedTransform,
                    shape2OutlineRect, shape2Transform,
                    referenceRect,
                    false, shape1OutlineRect,
                    false,
                    patternTransform2,
                    &stampResult);

    QVERIFY(TestUtil::checkQImage(stampResult, "svg_render", "render", "pattern_c_view_r_user_patch2"));
    QVERIFY(TestUtil::checkQImage(fillResult, "svg_render", "render", "pattern_c_view_r_user_fill2"));
}

void TestSvgParser::testRenderPattern_r_User_c_User()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<defs>"
            "    <pattern id=\"TestPattern\" patternUnits=\"userSpaceOnUse\""
            "        patternContentUnits=\"userSpaceOnUse\""
            "        x=\"60\" y=\"0\" width=\"30\" height=\"20\">"

            "        <g id=\"patternRect\">"
            "            <rect id=\"patternRect1\" x=\"70\" y=\"0\" width=\"10\" height=\"13.3333\""
            "                fill=\"red\" stroke=\"none\" />"

            "            <rect id=\"patternRect2\" x=\"80\" y=\"6.3333\" width=\"10\" height=\"6.6666\""
            "                fill=\"red\" stroke=\"none\" />"
            "        </g>"

            "    </pattern>"
            "</defs>"

            "<g>"
            "    <rect id=\"testRect\" x=\"10\" y=\"20\" width=\"40\" height=\"120\""
            "        transform=\"translate(40 30) scale(2 0.5)\""
            "        fill=\"url(#TestPattern)blue\" stroke=\"none\"/>"
            "</g>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("fill_pattern_base", false, QSize(160, 160));
}

void TestSvgParser::testRenderPattern_InfiniteRecursionWhenInherited()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<defs>"
            "    <pattern id=\"TestPattern\" patternUnits=\"userSpaceOnUse\""
            "        patternContentUnits=\"userSpaceOnUse\""
            "        x=\"60\" y=\"0\" width=\"30\" height=\"20\">"

            "        <g id=\"patternRect\">"
            "            <rect id=\"patternRect1\" x=\"70\" y=\"0\" width=\"10\" height=\"13.3333\""
            "                stroke=\"none\" />"

            "            <rect id=\"patternRect2\" x=\"80\" y=\"6.3333\" width=\"10\" height=\"6.6666\""
            "                stroke=\"none\" />"
            "        </g>"

            "    </pattern>"
            "</defs>"

            "<g>"
            "    <rect id=\"testRect\" x=\"10\" y=\"20\" width=\"40\" height=\"120\""
            "        transform=\"translate(40 30) scale(2 0.5)\""
            "        fill=\"url(#TestPattern)blue\" stroke=\"none\"/>"
            "</g>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("fill_pattern_base_black", false, QSize(160, 160));
}

void TestSvgParser::testRenderPattern_r_User_c_View()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<defs>"
            "    <pattern id=\"TestPattern\" patternUnits=\"userSpaceOnUse\""
            "        viewBox=\"10 10 60 90\""
            "        preserveAspectRatio=\"none meet\""
            "        x=\"60\" y=\"0\" width=\"30\" height=\"20\">"

            "        <g id=\"patternRect\">"
            "            <rect id=\"patternRect1\" x=\"30\" y=\"10\" width=\"20\" height=\"60\""
            "                fill=\"red\" stroke=\"none\" />"

            // y is changed to 39 from 40 to fix a rounding issue!
            "            <rect id=\"patternRect2\" x=\"50\" y=\"39\" width=\"20\" height=\"30\""
            "                fill=\"red\" stroke=\"none\" />"
            "        </g>"

            "    </pattern>"
            "</defs>"

            "<g>"
            "    <rect id=\"testRect\" x=\"10\" y=\"20\" width=\"40\" height=\"120\""
            "        transform=\"translate(40 30) scale(2 0.5)\""
            "        fill=\"url(#TestPattern)blue\" stroke=\"none\"/>"
            "</g>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("fill_pattern_base", false, QSize(160, 160));
}

void TestSvgParser::testRenderPattern_r_User_c_Obb()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<defs>"
            "    <pattern id=\"TestPattern\" patternUnits=\"userSpaceOnUse\""
            "        patternContentUnits=\"objectBoundingBox\""
            "        x=\"60\" y=\"0\" width=\"30\" height=\"20\">"

            "        <g id=\"patternRect\">"
            "            <rect id=\"patternRect1\" x=\"1.5\" y=\"-0.1666\" width=\"0.25\" height=\"0.111\""
            "                fill=\"red\" stroke=\"none\" />"

            "            <rect id=\"patternRect2\" x=\"1.75\" y=\"-0.12\" width=\"0.25\" height=\"0.07\""
            "                fill=\"red\" stroke=\"none\" />"
            "        </g>"

            "    </pattern>"
            "</defs>"

            "<g>"
            "    <rect id=\"testRect\" x=\"10\" y=\"20\" width=\"40\" height=\"120\""
            "        transform=\"translate(40 30) scale(2 0.5)\""
            "        fill=\"url(#TestPattern)blue\" stroke=\"none\"/>"
            "</g>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("fill_pattern_base", false, QSize(160, 160));
}

void TestSvgParser::testRenderPattern_r_User_c_View_Rotated()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<defs>"
            "    <pattern id=\"TestPattern\" patternUnits=\"userSpaceOnUse\""
            "        viewBox=\"10 10 60 90\""
            "        preserveAspectRatio=\"none meet\""
            "        x=\"60\" y=\"0\" width=\"30\" height=\"20\""
            "        patternTransform=\"translate(40 10) rotate(90)\">"

            "        <g id=\"patternRect\">"
            "            <rect id=\"patternRect1\" x=\"30\" y=\"10\" width=\"20\" height=\"60\""
            "                fill=\"red\" stroke=\"none\" />"
            "            <rect id=\"patternRect2\" x=\"50\" y=\"40\" width=\"20\" height=\"30\""
            "                fill=\"red\" stroke=\"none\" />"
            "        </g>"

            "    </pattern>"
            "</defs>"

            "<g>"
            "    <rect id=\"testRect\" x=\"20\" y=\"10\" width=\"60\" height=\"90\""
            "        transform=\"translate(50 50) scale(2 1)\""
            "        fill=\"url(#TestPattern)blue\" stroke=\"none\"/>"
            "</g>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("fill_pattern_rotated", false, QSize(220, 160));
}

void TestSvgParser::testRenderPattern_r_Obb_c_View_Rotated()
{
    /**
     * This test case differs from any application existent in the world :(
     *
     * Chrome and Firefox premultiply the patternTransform instead of doing post-
     * multiplication. Photoshop forgets to multiply the reference rect on it.
     *
     * So...
     */

    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<defs>"
            "    <pattern id=\"TestPattern\" patternUnits=\"objectBoundingBox\""
            "        viewBox=\"10 10 60 90\""
            "        preserveAspectRatio=\"none meet\""
            "        x=\"0\" y=\"0\" width=\"0.5\" height=\"0.333\""
            "        patternTransform=\"translate(0 0) rotate(90)\">"

            "        <g id=\"patternRect\">"
            "            <rect id=\"patternRect1\" x=\"30\" y=\"10\" width=\"20\" height=\"60\""
            "                fill=\"red\" stroke=\"none\" />"
            "            <rect id=\"patternRect2\" x=\"50\" y=\"40\" width=\"20\" height=\"30\""
            "                fill=\"red\" stroke=\"none\" />"
            "        </g>"

            "    </pattern>"
            "</defs>"

            "<g>"
            "    <rect id=\"testRect\" x=\"20\" y=\"10\" width=\"60\" height=\"90\""
            "        transform=\"translate(50 50) scale(1 1)\""
            "        fill=\"url(#TestPattern)blue\" stroke=\"none\"/>"
            "</g>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("fill_pattern_rotated_odd", false, QSize(220, 160));
}

#include <KoPathShape.h>
#include <KoColorBackground.h>
#include <KoClipPath.h>
#include <commands/KoShapeGroupCommand.h>

void TestSvgParser::testKoClipPathRendering()
{
    QPainterPath path1;
    path1.addRect(QRect(5,5,15,15));

    QPainterPath path2;
    path2.addRect(QRect(10,10,15,15));

    QPainterPath clipPath1;
    clipPath1.addRect(QRect(10, 0, 10, 30));

    QPainterPath clipPath2;
    clipPath2.moveTo(0,7);
    clipPath2.lineTo(30,7);
    clipPath2.lineTo(15,30);
    clipPath2.lineTo(0,7);

    QScopedPointer<KoPathShape> shape1(KoPathShape::createShapeFromPainterPath(path1));
    shape1->setBackground(QSharedPointer<KoColorBackground>(new KoColorBackground(Qt::blue)));

    QScopedPointer<KoPathShape> shape2(KoPathShape::createShapeFromPainterPath(path2));
    shape2->setBackground(QSharedPointer<KoColorBackground>(new KoColorBackground(Qt::red)));

    QScopedPointer<KoPathShape> clipShape1(KoPathShape::createShapeFromPainterPath(clipPath1));
    KoClipPath *koClipPath1 = new KoClipPath({clipShape1.take()}, KoFlake::UserSpaceOnUse);
    koClipPath1->setClipRule(Qt::WindingFill);
    shape1->setClipPath(koClipPath1);

    QScopedPointer<KoShapeGroup> group(new KoShapeGroup());
    {
        QList<KoShape*> shapes({shape1.take(), shape2.take()});

        KoShapeGroupCommand cmd(group.data(), shapes, false);
        cmd.redo();
    }

    QScopedPointer<KoPathShape> clipShape2(KoPathShape::createShapeFromPainterPath(clipPath2));
    KoClipPath *koClipPath2 = new KoClipPath({clipShape2.take()}, KoFlake::UserSpaceOnUse);
    koClipPath2->setClipRule(Qt::WindingFill);
    group->setClipPath(koClipPath2);

    SvgRenderTester::testRender(group.take(), "load", "clip_render_test", QSize(30,30));
}

void TestSvgParser::testKoClipPathRelativeRendering()
{
    QPainterPath path1;
    path1.addRect(QRect(5,5,15,15));

    QPainterPath path2;
    path2.addRect(QRect(10,10,15,15));

    QPainterPath clipPath1;
    clipPath1.addRect(QRect(10, 0, 10, 30));

    QPainterPath clipPath2;
    clipPath2.moveTo(0,0);
    clipPath2.lineTo(1,0);
    clipPath2.lineTo(0.5,1);
    clipPath2.lineTo(0,0);

    QScopedPointer<KoPathShape> shape1(KoPathShape::createShapeFromPainterPath(path1));
    shape1->setBackground(QSharedPointer<KoColorBackground>(new KoColorBackground(Qt::blue)));

    QScopedPointer<KoPathShape> shape2(KoPathShape::createShapeFromPainterPath(path2));
    shape2->setBackground(QSharedPointer<KoColorBackground>(new KoColorBackground(Qt::red)));

    QScopedPointer<KoPathShape> clipShape1(KoPathShape::createShapeFromPainterPath(clipPath1));
    KoClipPath *koClipPath1 = new KoClipPath({clipShape1.take()}, KoFlake::UserSpaceOnUse);
    koClipPath1->setClipRule(Qt::WindingFill);
    shape1->setClipPath(koClipPath1);

    QScopedPointer<KoShapeGroup> group(new KoShapeGroup());
    {
        QList<KoShape*> shapes({shape1.take(), shape2.take()});

        KoShapeGroupCommand cmd(group.data(), shapes, false);
        cmd.redo();
    }

    QScopedPointer<KoPathShape> clipShape2(KoPathShape::createShapeFromPainterPath(clipPath2));
    KoClipPath *koClipPath2 = new KoClipPath({clipShape2.take()}, KoFlake::ObjectBoundingBox);
    koClipPath2->setClipRule(Qt::WindingFill);
    group->setClipPath(koClipPath2);

    SvgRenderTester::testRender(group.take(), "load", "relative_clip_render_test", QSize(30,30));
}

void TestSvgParser::testRenderClipPath_User()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<clipPath id=\"clip1\" clipPathUnits=\"userSpaceOnUse\">"
            "    <rect id=\"clipRect1\" x=\"10\" y=\"0\" width=\"10\" height=\"30\""
            "            fill=\"red\" stroke=\"none\" />"
            "    <rect id=\"clipRect1\" x=\"10\" y=\"0\" width=\"10\" height=\"30\""
            "            fill=\"yellow\" stroke=\"none\" />"
            "</clipPath>"

            "<clipPath id=\"clip2\" clipPathUnits=\"userSpaceOnUse\">"
            "    <path id=\"clipRect1\" d=\"M 0 7 L 30 7 15 30 0 7 z\""
            "            fill=\"red\" stroke=\"none\" />"
            "</clipPath>"

            "<g id=\"testRect\" clip-path=\"url(#clip2)\">"
            "    <rect id=\"testRect1\" x=\"5\" y=\"5\" width=\"15\" height=\"15\""
            "        fill=\"blue\" stroke=\"none\" clip-path=\"url(#clip1)\"/>"

            "    <rect id=\"testRect2\" x=\"10\" y=\"10\" width=\"15\" height=\"15\""
            "        fill=\"red\" stroke=\"none\"/>"
            "</g>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("clip_render_test", false);
}

void TestSvgParser::testRenderClipPath_Obb()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<clipPath id=\"clip1\" clipPathUnits=\"userSpaceOnUse\">"
            "    <rect id=\"clipRect1\" x=\"10\" y=\"0\" width=\"10\" height=\"30\""
            "            fill=\"red\" stroke=\"none\" />"
            "    <rect id=\"clipRect1\" x=\"10\" y=\"0\" width=\"10\" height=\"30\""
            "            fill=\"yellow\" stroke=\"none\" />"
            "</clipPath>"

            "<clipPath id=\"clip2\" clipPathUnits=\"objectBoundingBox\">"
            "    <path id=\"clipRect1\" d=\"M 0 0 L 1 0 0.5 1 0 0 z\""
            "            fill=\"red\" stroke=\"none\" />"
            "</clipPath>"

            "<g id=\"testRect\" clip-path=\"url(#clip2)\">"
            "    <rect id=\"testRect1\" x=\"5\" y=\"5\" width=\"15\" height=\"15\""
            "        fill=\"blue\" stroke=\"none\" clip-path=\"url(#clip1)\"/>"

            "    <rect id=\"testRect2\" x=\"10\" y=\"10\" width=\"15\" height=\"15\""
            "        fill=\"red\" stroke=\"none\"/>"
            "</g>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("relative_clip_render_test", false);
}

void TestSvgParser::testRenderClipPath_Obb_Transform()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<clipPath id=\"clip1\" clipPathUnits=\"userSpaceOnUse\""
            "    transform=\"rotate(90,15,15)\">"
            "    <rect id=\"clipRect1\" x=\"10\" y=\"0\" width=\"10\" height=\"30\""
            "            fill=\"red\" stroke=\"none\" />"
            "    <rect id=\"clipRect1\" x=\"10\" y=\"0\" width=\"10\" height=\"30\""
            "            fill=\"yellow\" stroke=\"none\" />"
            "</clipPath>"

            "<clipPath id=\"clip2\" clipPathUnits=\"objectBoundingBox\""
            "    transform=\"rotate(90 0.5 0.5)\">"
            "    <path id=\"clipRect1\" d=\"M 0 0 L 1 0 0.5 1 0 0 z\""
            "            fill=\"red\" stroke=\"none\" />"
            "</clipPath>"

            "<g id=\"testRect\" clip-path=\"url(#clip2)\">"
            "    <rect id=\"testRect1\" x=\"5\" y=\"5\" width=\"15\" height=\"15\""
            "        fill=\"blue\" stroke=\"none\" clip-path=\"url(#clip1)\"/>"

            "    <rect id=\"testRect2\" x=\"10\" y=\"10\" width=\"15\" height=\"15\""
            "        fill=\"red\" stroke=\"none\"/>"
            "</g>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("clip_render_test_rotated", false);
}

void TestSvgParser::testRenderClipMask_Obb()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<defs>"

            "    <linearGradient id=\"Gradient\" gradientUnits=\"objectBoundingBox\""
            "        x1=\"0\" y1=\"0\" x2=\"1\" y2=\"0\">"

            "        <stop offset=\"0\" stop-color=\"white\" stop-opacity=\"0\" />"
            "        <stop offset=\"1\" stop-color=\"white\" stop-opacity=\"1\" />"

            "    </linearGradient>"

            "    <mask id=\"Mask\" maskUnits=\"objectBoundingBox\""
            "        maskContentUnits=\"objectBoundingBox\""
            "        x=\"0.2\" y=\"0.2\" width=\"0.6\" height=\"0.6\">"

            "        <rect x=\"0\" y=\"0\" width=\"1\" height=\"1\" fill=\"url(#Gradient)\" />"

            "    </mask>"

            "</defs>"


            "<g id=\"testRect\">"
            "    <rect id=\"testRect1\" x=\"5\" y=\"5\" width=\"15\" height=\"15\""
            "        fill=\"blue\" stroke=\"none\"/>"

            "    <rect id=\"testRect2\" x=\"10\" y=\"10\" width=\"15\" height=\"15\""
            "        fill=\"red\" stroke=\"none\" mask=\"url(#Mask)\" />"
            "</g>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("clip_mask_obb", false);
}

void TestSvgParser::testRenderClipMaskOnGroup_Obb()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<defs>"

            "    <linearGradient id=\"Gradient\" gradientUnits=\"objectBoundingBox\""
            "        x1=\"0\" y1=\"0\" x2=\"1\" y2=\"0\">"

            "        <stop offset=\"0\" stop-color=\"white\" stop-opacity=\"0\" />"
            "        <stop offset=\"1\" stop-color=\"white\" stop-opacity=\"1\" />"

            "    </linearGradient>"

            "    <mask id=\"Mask\" maskUnits=\"objectBoundingBox\""
            "        maskContentUnits=\"objectBoundingBox\""
            "        x=\"0.2\" y=\"0.2\" width=\"0.6\" height=\"0.6\">"

            "        <rect x=\"0\" y=\"0\" width=\"1\" height=\"1\" fill=\"url(#Gradient)\" />"

            "    </mask>"

            "</defs>"


            "<g id=\"testRect\" mask=\"url(#Mask)\">"
            "    <rect id=\"testRect1\" x=\"5\" y=\"5\" width=\"15\" height=\"15\""
            "        fill=\"blue\" stroke=\"none\"/>"

            "    <rect id=\"testRect2\" x=\"10\" y=\"10\" width=\"15\" height=\"15\""
            "        fill=\"red\" stroke=\"none\" />"
            "</g>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("clip_mask_on_group_obb", false);
}

QByteArray fileFetcherFunc(const QString &name)
{
    const QString fileName = TestUtil::fetchDataFileLazy(name);
    QFile file(fileName);
    KIS_ASSERT(file.exists());
    file.open(QIODevice::ReadOnly);
    return file.readAll();
}

void TestSvgParser::testRenderClipMask_User_Clip_Obb()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            //"<defs>"

            "    <linearGradient id=\"Gradient\" gradientUnits=\"objectBoundingBox\""
            "        x1=\"0\" y1=\"0\" x2=\"1\" y2=\"0\">"

            "        <stop offset=\"0\" stop-color=\"white\" stop-opacity=\"0\" />"
            "        <stop offset=\"1\" stop-color=\"white\" stop-opacity=\"1\" />"

            "    </linearGradient>"

            "    <mask id=\"Mask\" maskUnits=\"objectBoundingBox\""
            "        maskContentUnits=\"userSpaceOnUse\""
            "        x=\"0.2\" y=\"0.2\" width=\"0.6\" height=\"0.6\">"

            "        <rect x=\"10\" y=\"10\" width=\"15\" height=\"15\" fill=\"url(#Gradient)\" />"

            "    </mask>"

            //"</defs>"


            "<g id=\"testRect\">"
            "    <rect id=\"testRect1\" x=\"5\" y=\"5\" width=\"15\" height=\"15\""
            "        fill=\"blue\" stroke=\"none\"/>"

            "    <rect id=\"testRect2\" x=\"10\" y=\"10\" width=\"15\" height=\"15\""
            "        fill=\"red\" stroke=\"none\" mask=\"url(#Mask)\" />"
            "</g>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("clip_mask_obb", false);
}

void TestSvgParser::testRenderClipMask_User_Clip_User()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            //"<defs>"

            "    <linearGradient id=\"Gradient\" gradientUnits=\"objectBoundingBox\""
            "        x1=\"0\" y1=\"0\" x2=\"1\" y2=\"0\">"

            "        <stop offset=\"0\" stop-color=\"white\" stop-opacity=\"0\" />"
            "        <stop offset=\"1\" stop-color=\"white\" stop-opacity=\"1\" />"

            "    </linearGradient>"

            "    <mask id=\"Mask\" maskUnits=\"userSpaceOnUse\""
            "        maskContentUnits=\"userSpaceOnUse\""
            "        x=\"13\" y=\"13\" width=\"9\" height=\"9\">"

            "        <rect x=\"10\" y=\"10\" width=\"15\" height=\"15\" fill=\"url(#Gradient)\" />"

            "    </mask>"

            //"</defs>"


            "<g id=\"testRect\">"
            "    <rect id=\"testRect1\" x=\"5\" y=\"5\" width=\"15\" height=\"15\""
            "        fill=\"blue\" stroke=\"none\"/>"

            "    <rect id=\"testRect2\" x=\"10\" y=\"10\" width=\"15\" height=\"15\""
            "        fill=\"red\" stroke=\"none\" mask=\"url(#Mask)\" />"
            "</g>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("clip_mask_obb", false);
}

void TestSvgParser::testRenderImage_AspectDefault()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g id=\"testRect\">"
            "    <rect id=\"testRect1\" x=\"2\" y=\"2\" width=\"26\" height=\"26\""
            "        fill=\"green\" stroke=\"none\"/>"

            "    <rect id=\"testRect1\" x=\"5\" y=\"5\" width=\"15\" height=\"10\""
            "        fill=\"white\" stroke=\"none\"/>"


            "    <image x=\"5\" y=\"5\" width=\"15\" height=\"10\""
            "        xlink:href=\"svg_render/testing_ref_image.png\">"

            "        <title>My image</title>"

            "    </image>"

            "</g>"

            "</svg>";

    SvgRenderTester t (data);
    t.parser.setFileFetcher(fileFetcherFunc);

    t.test_standard_30px_72ppi("image_aspect_default", false);
}

void TestSvgParser::testRenderImage_AspectNone()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g id=\"testRect\">"
            "    <rect id=\"testRect1\" x=\"2\" y=\"2\" width=\"26\" height=\"26\""
            "        fill=\"green\" stroke=\"none\"/>"

            "    <rect id=\"testRect1\" x=\"5\" y=\"5\" width=\"15\" height=\"10\""
            "        fill=\"white\" stroke=\"none\"/>"


            "    <image x=\"5\" y=\"5\" width=\"15\" height=\"10\""
            "        preserveAspectRatio=\"none\""
            "        xlink:href=\"svg_render/testing_ref_image.png\">"

            "        <title>My image</title>"

            "    </image>"

            "</g>"

            "</svg>";

    SvgRenderTester t (data);
    t.setFuzzyThreshold(5);
    t.parser.setFileFetcher(fileFetcherFunc);

    t.test_standard_30px_72ppi("image_aspect_none", false);
}

void TestSvgParser::testRenderImage_AspectMeet()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g id=\"testRect\">"
            "    <rect id=\"testRect1\" x=\"2\" y=\"2\" width=\"26\" height=\"26\""
            "        fill=\"green\" stroke=\"none\"/>"

            "    <rect id=\"testRect1\" x=\"5\" y=\"5\" width=\"15\" height=\"10\""
            "        fill=\"white\" stroke=\"none\"/>"


            "    <image x=\"5\" y=\"5\" width=\"15\" height=\"10\""
            "        preserveAspectRatio=\"xMinYMin meet\""
            "        xlink:href=\"svg_render/testing_ref_image.png\">"

            "        <title>My image</title>"

            "    </image>"

            "</g>"

            "</svg>";

    SvgRenderTester t (data);
    t.parser.setFileFetcher(fileFetcherFunc);

    t.test_standard_30px_72ppi("image_aspect_meet", false);
}

void TestSvgParser::testRectShapeRoundUniformX()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "    <rect id=\"testRect\" x=\"5\" y=\"5\" width=\"20\" height=\"20\""
            "        rx=\"5\""
            "        fill=\"blue\" stroke=\"none\"/>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("rect_5_5", false);
}

void TestSvgParser::testRectShapeRoundUniformY()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "    <rect id=\"testRect\" x=\"5\" y=\"5\" width=\"20\" height=\"20\""
            "        rx=\"5\""
            "        fill=\"blue\" stroke=\"none\"/>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("rect_5_5", false);
}

void TestSvgParser::testRectShapeRoundXY()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "    <rect id=\"testRect\" x=\"5\" y=\"5\" width=\"20\" height=\"20\""
            "        rx=\"5\" ry=\"10\""
            "        fill=\"blue\" stroke=\"none\"/>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("rect_5_10", false);
}

void TestSvgParser::testRectShapeRoundXYOverflow()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "    <rect id=\"testRect\" x=\"5\" y=\"5\" width=\"20\" height=\"20\""
            "        rx=\"5\" ry=\"25\""
            "        fill=\"blue\" stroke=\"none\"/>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("rect_5_10", false);
}

void TestSvgParser::testCircleShape()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "    <circle id=\"testRect\" cx=\"15\" cy=\"15\" r=\"10\""
            "        fill=\"blue\" stroke=\"white\"/>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("circle", false);
}

void TestSvgParser::testEllipseShape()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "    <ellipse id=\"testRect\" cx=\"15\" cy=\"15\" rx=\"10\" ry=\"5\""
            "        fill=\"blue\" stroke=\"white\"/>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("ellipse", false);
}

void TestSvgParser::testLineShape()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "    <line id=\"testRect\" x1=\"5\" y1=\"5\" x2=\"25\" y2=\"15\""
            "        fill=\"blue\" stroke=\"white\"/>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("line", false);
}

void TestSvgParser::testPolylineShape()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "    <polyline id=\"testRect\" points=\"5,5 10, 5 10,10 5,10 \n"
            "        5 ,15 15 , 15 15,20 20,20 20,10 25,10 25,25 5,25\""
            "        fill=\"red\" stroke=\"white\"/>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("polyline", false);
}

void TestSvgParser::testPolygonShape()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "    <polygon id=\"testRect\" points=\"5,5 10, 5 10,10 5,10 \n"
            "        5 ,15 15 , 15 15,20 20,20 20,10 25,10 25,25 5,25\""
            "        fill=\"red\" stroke=\"white\"/>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("polygon", false);
}

void TestSvgParser::testPathShape()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "    <path id=\"testRect\""
            "        d=\"M 5,5 h 5 l 0,5 L5,10 l 0,5 l10,0 v5 l 5, 0 L20, 10 l 5,0 L 25,25 L 5,25 z\""
            "        fill=\"red\" stroke=\"white\"/>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("polygon", false);
}

void TestSvgParser::testDefsHidden()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<g id=\"testRect\">"
            "    <defs>"
            "        <rect id=\"testRect1\" x=\"5\" y=\"5\" width=\"15\" height=\"15\""
            "            fill=\"blue\" stroke=\"none\"/>"
            "    </defs>"

            "    <rect id=\"testRect2\" x=\"10\" y=\"10\" width=\"15\" height=\"15\""
            "        fill=\"red\" stroke=\"none\"/>"
            "</g>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("test_defs_hidden", false);
}

void TestSvgParser::testDefsUseInheritance()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"



            "<g id=\"unrenderedRect\" fill=\"green\" >"
            "    <defs>"
            "        <rect id=\"testRect1\" x=\"1\" y=\"1\" width=\"15\" height=\"15\""
            "            stroke=\"none\"/>"
            "    </defs>"
            "</g>"


            /**
             * NOTES:
             * 1) width/height attributes for <use> are not implemented yet
             * 2) x and y are summed up
             * 3) stroke="white" is overridden by the original templated object
             * 4) fill="green" attribute from <defs> is not inherited
             */

            "<g id=\"testRect\" fill=\"blue\" >"
            "    <use x=\"4\" y=\"4\" xlink:href=\"#testRect1\""
            "        stroke=\"white\" stroke-width=\"1\" />"

            "    <rect id=\"testRect2\" x=\"10\" y=\"10\" width=\"15\" height=\"15\""
            "        fill=\"red\" stroke=\"none\"/>"
            "</g>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("defs_use_inheritance", false);
}

void TestSvgParser::testUseWithoutDefs()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            // technical rect for rendering
            "<g id=\"testRect\">"

            "<g id=\"renderedRect1\" fill=\"green\" >"
            "    <rect id=\"testRect1\" x=\"1\" y=\"1\" width=\"15\" height=\"15\""
            "        stroke=\"none\"/>"
            "</g>"


            /**
             * NOTES:
             * 1) width/height attributes for <use> are not implemented yet
             * 2) x and y are summed up
             * 3) stroke="white" is overridden by the original templated object
             * 4) fill="green" attribute from <defs> is not inherited
             */

            "<g id=\"renderedRect2\" fill=\"blue\" >"
            "    <use x=\"4\" y=\"4\" xlink:href=\"#testRect1\""
            "        stroke=\"white\" stroke-width=\"1\" />"

            "    <rect id=\"testRect2\" x=\"10\" y=\"10\" width=\"15\" height=\"15\""
            "        fill=\"red\" stroke=\"none\"/>"
            "</g>"

            "</g>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("use_without_defs", false);
}

void TestSvgParser::testMarkersAutoOrientation()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<marker id=\"SimpleRectMarker\""
            "    orient=\"auto\" refY=\"12.5\" refX=\"12.5\" >"

            "    <rect id=\"markerRect\" x=\"10\" y=\"10\" width=\"5\" height=\"5\""
            "        fill=\"red\" stroke=\"none\"/>"
            "    <rect id=\"markerRect\" x=\"14\" y=\"12\" width=\"1\" height=\"1\""
            "        fill=\"yellow\" stroke=\"none\"/>"
            "    <rect id=\"markerRect\" x=\"12\" y=\"12\" width=\"1\" height=\"1\""
            "        fill=\"white\" stroke=\"none\"/>"
            "</marker>"

            "<path id=\"testRect\""
            "    style=\"fill:none;stroke:#000000;stroke-width:1px;marker-start:url(#SimpleRectMarker);marker-end:url(#SimpleRectMarker);marker-mid:url(#SimpleRectMarker)\""
            "    d=\"M5,15 C5,5 25,5 25,15 L15,25\"/>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("markers", false);
}

void TestSvgParser::testMarkersAutoOrientationScaled()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<marker id=\"SimpleRectMarker\""
            "    orient=\"auto\" refY=\"12.5\" refX=\"12.5\" >"

            "    <rect id=\"markerRect\" x=\"10\" y=\"10\" width=\"5\" height=\"5\""
            "        fill=\"red\" stroke=\"none\"/>"
            "    <rect id=\"markerRect\" x=\"14\" y=\"12\" width=\"1\" height=\"1\""
            "        fill=\"yellow\" stroke=\"none\"/>"
            "    <rect id=\"markerRect\" x=\"12\" y=\"12\" width=\"1\" height=\"1\""
            "        fill=\"white\" stroke=\"none\"/>"
            "</marker>"

            "<path id=\"testRect\""
            "    style=\"fill:none;stroke:#000000;stroke-width:2px;marker-start:url(#SimpleRectMarker);marker-end:url(#SimpleRectMarker);marker-mid:url(#SimpleRectMarker)\""
            "    d=\"M5,15 C5,5 25,5 25,15 L15,25\"/>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("markers_scaled", false);
}

void TestSvgParser::testMarkersAutoOrientationScaledUserCoordinates()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<marker id=\"SimpleRectMarker\""
            "    markerUnits = \"userSpaceOnUse\""
            "    orient=\"auto\" refY=\"12.5\" refX=\"12.5\" >"

            "    <rect id=\"markerRect\" x=\"10\" y=\"10\" width=\"5\" height=\"5\""
            "        fill=\"red\" stroke=\"none\"/>"
            "    <rect id=\"markerRect\" x=\"14\" y=\"12\" width=\"1\" height=\"1\""
            "        fill=\"yellow\" stroke=\"none\"/>"
            "    <rect id=\"markerRect\" x=\"12\" y=\"12\" width=\"1\" height=\"1\""
            "        fill=\"white\" stroke=\"none\"/>"
            "</marker>"

            "<path id=\"testRect\""
            "    style=\"fill:none;stroke:#000000;stroke-width:2px;marker-start:url(#SimpleRectMarker);marker-end:url(#SimpleRectMarker);marker-mid:url(#SimpleRectMarker)\""
            "    d=\"M5,15 C5,5 25,5 25,15 L15,25\"/>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("markers_user_coordinates", false);
}

void TestSvgParser::testMarkersCustomOrientation()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<marker id=\"SimpleRectMarker\""
            "    orient=\"45\" refY=\"12.5\" refX=\"12.5\" >"

            "    <rect id=\"markerRect\" x=\"10\" y=\"10\" width=\"5\" height=\"5\""
            "        fill=\"red\" stroke=\"none\"/>"
            "    <rect id=\"markerRect\" x=\"14\" y=\"12\" width=\"1\" height=\"1\""
            "        fill=\"yellow\" stroke=\"none\"/>"
            "    <rect id=\"markerRect\" x=\"12\" y=\"12\" width=\"1\" height=\"1\""
            "        fill=\"white\" stroke=\"none\"/>"
            "</marker>"

            "<path id=\"testRect\""
            "    style=\"fill:none;stroke:#000000;stroke-width:1px;marker-start:url(#SimpleRectMarker);marker-end:url(#SimpleRectMarker);marker-mid:url(#SimpleRectMarker)\""
            "    d=\"M5,15 C5,5 25,5 25,15 L15,25\"/>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("markers_custom_orientation", false);
}

void TestSvgParser::testMarkersDifferent()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<marker id=\"SimpleRectMarker1\""
            "    orient=\"auto\" refY=\"12.5\" refX=\"12.5\" >"

            "    <rect id=\"markerRect\" x=\"10\" y=\"10\" width=\"5\" height=\"5\""
            "        fill=\"red\" stroke=\"none\"/>"
            "    <rect id=\"markerRect\" x=\"14\" y=\"12\" width=\"1\" height=\"1\""
            "        fill=\"yellow\" stroke=\"none\"/>"
            "    <rect id=\"markerRect\" x=\"12\" y=\"12\" width=\"1\" height=\"1\""
            "        fill=\"white\" stroke=\"none\"/>"
            "</marker>"

            "<marker id=\"SimpleRectMarker2\""
            "    orient=\"auto\" refY=\"12.5\" refX=\"12.5\" >"

            "    <rect id=\"markerRect\" x=\"10\" y=\"10\" width=\"5\" height=\"5\""
            "        fill=\"green\" stroke=\"none\"/>"
            "    <rect id=\"markerRect\" x=\"14\" y=\"12\" width=\"1\" height=\"1\""
            "        fill=\"yellow\" stroke=\"none\"/>"
            "    <rect id=\"markerRect\" x=\"12\" y=\"12\" width=\"1\" height=\"1\""
            "        fill=\"white\" stroke=\"none\"/>"
            "</marker>"

            "<marker id=\"SimpleRectMarker3\""
            "    orient=\"auto\" refY=\"12.5\" refX=\"12.5\" >"

            "    <rect id=\"markerRect\" x=\"10\" y=\"10\" width=\"5\" height=\"5\""
            "        fill=\"blue\" stroke=\"none\"/>"
            "    <rect id=\"markerRect\" x=\"14\" y=\"12\" width=\"1\" height=\"1\""
            "        fill=\"yellow\" stroke=\"none\"/>"
            "    <rect id=\"markerRect\" x=\"12\" y=\"12\" width=\"1\" height=\"1\""
            "        fill=\"white\" stroke=\"none\"/>"
            "</marker>"

            "<path id=\"testRect\""
            "    style=\"fill:none;stroke:#000000;stroke-width:1px;marker-start:url(#SimpleRectMarker1);marker-end:url(#SimpleRectMarker2);marker-mid:url(#SimpleRectMarker3)\""
            "    d=\"M5,15 C5,5 25,5 25,15 L15,25\"/>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("markers_different", false);
}

void TestSvgParser::testMarkersFillAsShape()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<defs>"
            "    <linearGradient id=\"testGrad\" x1=\"0%\" y1=\"50%\" x2=\"100%\" y2=\"50%\">"
            "        <stop offset=\"5%\" stop-color=\"#F60\" />"
            "        <stop offset=\"80%\" stop-color=\"#FF6\" />"
            "    </linearGradient>"

            "    <marker id=\"SimpleRectMarker\""
            "        orient=\"auto\" refY=\"12.5\" refX=\"12.5\" >"

            "        <rect id=\"markerRect\" x=\"10\" y=\"10\" width=\"5\" height=\"5\""
            "            fill=\"red\" stroke=\"none\"/>"
            "        <rect id=\"markerRect\" x=\"14\" y=\"12\" width=\"1\" height=\"1\""
            "            fill=\"yellow\" stroke=\"none\"/>"
            "        <rect id=\"markerRect\" x=\"12\" y=\"12\" width=\"1\" height=\"1\""
            "            fill=\"white\" stroke=\"none\"/>"
            "    </marker>"

            "</defs>"

            "<path id=\"testRect\""
            "    style=\"fill:none;stroke:url(#testGrad) magenta;stroke-width:2px;marker-start:url(#SimpleRectMarker);marker-end:url(#SimpleRectMarker);marker-mid:url(#SimpleRectMarker);krita|marker-fill-method:auto\""
            "    d=\"M5,15 C5,5 25,5 25,15 L15,25\"/>"
            //"    d=\"M5,15 L25,15\"/>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("markers_scaled_fill_as_shape", false);
}

void TestSvgParser::testMarkersOnClosedPath()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            "<marker id=\"SimpleRectMarker1\""
            "    orient=\"auto\" refY=\"12.5\" refX=\"12.5\" >"

            "    <rect id=\"markerRect\" x=\"9\" y=\"9\" width=\"7\" height=\"7\""
            "        fill=\"red\" stroke=\"none\"/>"
            "    <rect id=\"markerRect\" x=\"14\" y=\"12\" width=\"1\" height=\"1\""
            "        fill=\"yellow\" stroke=\"none\"/>"
            "    <rect id=\"markerRect\" x=\"12\" y=\"12\" width=\"1\" height=\"1\""
            "        fill=\"white\" stroke=\"none\"/>"
            "</marker>"

            "<marker id=\"SimpleRectMarker2\""
            "    orient=\"auto\" refY=\"12.5\" refX=\"12.5\" >"

            "    <rect id=\"markerRect\" x=\"10\" y=\"10\" width=\"5\" height=\"5\""
            "        fill=\"green\" stroke=\"none\"/>"
            "    <rect id=\"markerRect\" x=\"14\" y=\"12\" width=\"1\" height=\"1\""
            "        fill=\"yellow\" stroke=\"none\"/>"
            "    <rect id=\"markerRect\" x=\"12\" y=\"12\" width=\"1\" height=\"1\""
            "        fill=\"white\" stroke=\"none\"/>"
            "</marker>"

            "<marker id=\"SimpleRectMarker3\""
            "    orient=\"auto\" refY=\"12.5\" refX=\"12.5\" >"

            "    <rect id=\"markerRect\" x=\"10\" y=\"10\" width=\"5\" height=\"5\""
            "        fill=\"blue\" stroke=\"none\"/>"
            "    <rect id=\"markerRect\" x=\"14\" y=\"12\" width=\"1\" height=\"1\""
            "        fill=\"yellow\" stroke=\"none\"/>"
            "    <rect id=\"markerRect\" x=\"12\" y=\"12\" width=\"1\" height=\"1\""
            "        fill=\"white\" stroke=\"none\"/>"
            "</marker>"

            "<path id=\"testRect\""
            "    style=\"fill:none;stroke:#000000;stroke-width:1px;marker-start:url(#SimpleRectMarker1);marker-end:url(#SimpleRectMarker2);marker-mid:url(#SimpleRectMarker3)\""
            "    d=\"M5,15 C5,5 25,5 25,15 L15,25 z\"/>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("markers_on_closed_path", false);
}


void TestSvgParser::testGradientRecoveringTransform()
{
    // used for experimenting purposes only!

    QImage image(100,100,QImage::Format_ARGB32);
    image.fill(0);
    QPainter painter(&image);

    painter.setPen(QPen(Qt::black, 0));


    QLinearGradient gradient(0, 0.5, 1, 0.5);
    gradient.setCoordinateMode(QGradient::ObjectBoundingMode);

    //QLinearGradient gradient(0, 50, 100, 50);
    //gradient.setCoordinateMode(QGradient::LogicalMode);

    gradient.setColorAt(0.0, Qt::red);
    gradient.setColorAt(1.0, Qt::blue);

    QTransform gradientTransform;
    gradientTransform.shear(0.2, 0);

    {
        QBrush brush(gradient);
        brush.setTransform(gradientTransform);
        painter.setBrush(brush);
    }

    QRect mainShape(3,3,94,94);
    painter.drawRect(mainShape);

    QTransform gradientToUser(mainShape.width(), 0, 0, mainShape.height(),
                              mainShape.x(), mainShape.y());

    QRect smallShape(0,0,20,20);
    QTransform smallShapeTransform;

    {
        smallShapeTransform =
            QTransform::fromTranslate(-smallShape.center().x(), -smallShape.center().y());

        QTransform r; r.rotate(90);
        smallShapeTransform *= r;

        smallShapeTransform *=
            QTransform::fromTranslate(mainShape.center().x(), mainShape.center().y());
    }


    {
        gradient.setCoordinateMode(QGradient::LogicalMode);
        QBrush brush(gradient);
        brush.setTransform(gradientTransform * gradientToUser * smallShapeTransform.inverted());
        painter.setBrush(brush);
        painter.setPen(Qt::NoPen);
    }

    painter.setTransform(smallShapeTransform);
    painter.drawRect(smallShape);

    //image.save("gradient_recovering_transform.png");
}

void TestSvgParser::testMarkersAngularUnits()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"

            // start
            "<marker id=\"SimpleRectMarker1\""
            "    orient=\"45deg\" refY=\"12.5\" refX=\"12.5\" >"

            "    <rect id=\"markerRect\" x=\"10\" y=\"10\" width=\"5\" height=\"5\""
            "        fill=\"red\" stroke=\"none\"/>"
            "    <rect id=\"markerRect\" x=\"14\" y=\"12\" width=\"1\" height=\"1\""
            "        fill=\"yellow\" stroke=\"none\"/>"
            "    <rect id=\"markerRect\" x=\"12\" y=\"12\" width=\"1\" height=\"1\""
            "        fill=\"white\" stroke=\"none\"/>"
            "</marker>"

            // end
            "<marker id=\"SimpleRectMarker2\""
            "    orient=\"200grad\" refY=\"12.5\" refX=\"12.5\" >"

            "    <rect id=\"markerRect\" x=\"10\" y=\"10\" width=\"5\" height=\"5\""
            "        fill=\"green\" stroke=\"none\"/>"
            "    <rect id=\"markerRect\" x=\"14\" y=\"12\" width=\"1\" height=\"1\""
            "        fill=\"yellow\" stroke=\"none\"/>"
            "    <rect id=\"markerRect\" x=\"12\" y=\"12\" width=\"1\" height=\"1\""
            "        fill=\"white\" stroke=\"none\"/>"
            "</marker>"

            // mid
            "<marker id=\"SimpleRectMarker3\""
            "    orient=\"-1.57rad\" refY=\"12.5\" refX=\"12.5\" >"

            "    <rect id=\"markerRect\" x=\"10\" y=\"10\" width=\"5\" height=\"5\""
            "        fill=\"blue\" stroke=\"none\"/>"
            "    <rect id=\"markerRect\" x=\"14\" y=\"12\" width=\"1\" height=\"1\""
            "        fill=\"yellow\" stroke=\"none\"/>"
            "    <rect id=\"markerRect\" x=\"12\" y=\"12\" width=\"1\" height=\"1\""
            "        fill=\"white\" stroke=\"none\"/>"
            "</marker>"

            "<path id=\"testRect\""
            "    style=\"fill:none;stroke:#000000;stroke-width:1px;marker-start:url(#SimpleRectMarker1);marker-end:url(#SimpleRectMarker2);marker-mid:url(#SimpleRectMarker3)\""
            "    d=\"M5,15 C5,5 25,5 25,15 L15,25\"/>"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("markers_angular_units", false);
}

#include "KoParameterShape.h"

void TestSvgParser::testSodipodiArcShape()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\""
            "    xmlns:sodipodi=\"http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd\""
            ">"

            "<path"
            "    fill=\"red\" stroke=\"black\""
            "    id=\"testRect\""

            "    sodipodi:type=\"arc\""
            "    sodipodi:cx=\"15.464287\""
            "    sodipodi:cy=\"14.517863\""
            "    sodipodi:rx=\"6.25\""
            "    sodipodi:ry=\"8.5\""
            "    sodipodi:start=\"5.5346039\""
            "    sodipodi:end=\"4.0381334\""
            //"    d=\"m 20.043381,8.7327624 a 6.25,8.5 0 0 1 1.053863,9.4677386 6.25,8.5 0 0 1 -6.094908,4.794113 6.25,8.5 0 0 1 -5.5086474,-5.963709 6.25,8.5 0 0 1 2.0686234,-9.1530031 l 3.901975,6.6399611 z\" />"
            "    d=\" some weird unparsable text \" />"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("sodipodi_closed_arc", false);

    KoShape *shape = t.findShape("testRect");
    QVERIFY(dynamic_cast<KoParameterShape*>(shape));
}

void TestSvgParser::testSodipodiArcShapeOpen()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\""
            "    xmlns:sodipodi=\"http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd\""
            ">"

            "<path"
            "    fill=\"red\" stroke=\"black\""
            "    id=\"testRect\""

            "    sodipodi:type=\"arc\""
            "    sodipodi:open=\"true\""
            "    sodipodi:cx=\"15.464287\""
            "    sodipodi:cy=\"14.517863\""
            "    sodipodi:rx=\"6.25\""
            "    sodipodi:ry=\"8.5\""
            "    sodipodi:start=\"5.5346039\""
            "    sodipodi:end=\"4.0381334\""
            //"    d=\"m 20.043381,8.7327624 a 6.25,8.5 0 0 1 1.053863,9.4677386 6.25,8.5 0 0 1 -6.094908,4.794113 6.25,8.5 0 0 1 -5.5086474,-5.963709 6.25,8.5 0 0 1 2.0686234,-9.1530031 l 3.901975,6.6399611 z\" />"
            "    d=\" some weird unparsable text \" />"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("sodipodi_open_arc", false);

    KoShape *shape = t.findShape("testRect");
    QVERIFY(dynamic_cast<KoParameterShape*>(shape));
}

void TestSvgParser::testKritaChordShape()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\""
            "    xmlns:sodipodi=\"http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd\""
            ">"

            "<path"
            "    fill=\"red\" stroke=\"black\""
            "    id=\"testRect\""

            "    krita:type=\"arc\""
            "    krita:arcType=\"chord\""
            "    krita:cx=\"15.464287\""
            "    krita:cy=\"14.517863\""
            "    krita:rx=\"6.25\""
            "    krita:ry=\"8.5\""
            "    krita:start=\"5.5346039\""
            "    krita:end=\"4.0381334\""
            //"    d=\"m 20.043381,8.7327624 a 6.25,8.5 0 0 1 1.053863,9.4677386 6.25,8.5 0 0 1 -6.094908,4.794113 6.25,8.5 0 0 1 -5.5086474,-5.963709 6.25,8.5 0 0 1 2.0686234,-9.1530031 l 3.901975,6.6399611 z\" />"
            "    d=\" some weird unparsable text \" />"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("sodipodi_chord_arc", false);

    KoShape *shape = t.findShape("testRect");
    QVERIFY(dynamic_cast<KoParameterShape*>(shape));
}

void TestSvgParser::testSodipodiChordShape()
{
    const QString data =
            "<svg width=\"30px\" height=\"30px\""
            "    xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\""
            "    xmlns:sodipodi=\"http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd\""
            ">"

            "<path"
            "    fill=\"red\" stroke=\"black\""
            "    id=\"testRect\""

            "    sodipodi:type=\"arc\""
            "    sodipodi:arc-type=\"chord\""
            "    sodipodi:open=\"true\""
            "    sodipodi:cx=\"15.464287\""
            "    sodipodi:cy=\"14.517863\""
            "    sodipodi:rx=\"6.25\""
            "    sodipodi:ry=\"8.5\""
            "    sodipodi:start=\"5.5346039\""
            "    sodipodi:end=\"4.0381334\""
            //"    d=\"m 20.043381,8.7327624 a 6.25,8.5 0 0 1 1.053863,9.4677386 6.25,8.5 0 0 1 -6.094908,4.794113 6.25,8.5 0 0 1 -5.5086474,-5.963709 6.25,8.5 0 0 1 2.0686234,-9.1530031 l 3.901975,6.6399611 z\" />"
            "    d=\" some weird unparsable text \" />"

            "</svg>";

    SvgRenderTester t (data);

    t.test_standard_30px_72ppi("sodipodi_chord_arc", false);

    KoShape *shape = t.findShape("testRect");
    QVERIFY(dynamic_cast<KoParameterShape*>(shape));
}


QTEST_MAIN(TestSvgParser)
