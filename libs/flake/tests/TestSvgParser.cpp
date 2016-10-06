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

QTEST_GUILESS_MAIN(TestSvgParser)
