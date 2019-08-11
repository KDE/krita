/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "test_save_load_transform_args.h"

#include <QTest>

#include <QDomDocument>

#include "tool_transform_args.h"
#include "kis_liquify_transform_worker.h"


void TestSaveLoadTransformArgs::testFreeTransform()
{
    ToolTransformArgs args;

    args.setMode(ToolTransformArgs::FREE_TRANSFORM);
    args.setTransformedCenter(QPointF(1.0, 2.0));
    args.setOriginalCenter(QPointF(2.0, 3.0));
    args.setRotationCenterOffset(QPointF(3.0, 4.0));
    args.setAX(0.5);
    args.setAY(0.6);

    args.setAZ(0.7);
    args.setCameraPos(QVector3D(7.0, 8.0, 9.0));

    args.setScaleX(10.0);
    args.setScaleY(11.0);
    args.setShearX(10.0);
    args.setShearY(11.0);

    args.setKeepAspectRatio(true);

    args.setFlattenedPerspectiveTransform(QTransform::fromScale(12.0, 13.0));


    QDomDocument doc("test_type");
    QDomElement root = doc.createElement("root");
    doc.appendChild(root);

    args.toXML(&root);

    ToolTransformArgs newArgs = ToolTransformArgs::fromXML(root);
    QCOMPARE(newArgs, args);
}

void TestSaveLoadTransformArgs::testWarp()
{
    ToolTransformArgs args;

    args.setMode(ToolTransformArgs::WARP);
    args.setDefaultPoints(false);

    args.refOriginalPoints() << QPointF(1.0, 2.0);
    args.refOriginalPoints() << QPointF(2.0, 3.0);
    args.refOriginalPoints() << QPointF(3.0, 4.0);
    args.refOriginalPoints() << QPointF(4.0, 5.0);

    args.refTransformedPoints() << QPointF(6.0, 7.0);
    args.refTransformedPoints() << QPointF(7.0, 8.0);
    args.refTransformedPoints() << QPointF(8.0, 9.0);
    args.refTransformedPoints() << QPointF(9.0, 8.0);
    args.setWarpType(KisWarpTransformWorker::RIGID_TRANSFORM);
    args.setAlpha(0.5);

    QDomDocument doc("test_type");
    QDomElement root = doc.createElement("root");
    doc.appendChild(root);

    args.toXML(&root);

    ToolTransformArgs newArgs = ToolTransformArgs::fromXML(root);
    QCOMPARE(newArgs, args);
}

void TestSaveLoadTransformArgs::testCage()
{
    ToolTransformArgs args;

    args.setMode(ToolTransformArgs::CAGE);
    args.setDefaultPoints(false);
    qDebug() << "Running";

    args.refOriginalPoints() << QPointF(1.0, 2.0);
    args.refOriginalPoints() << QPointF(2.0, 3.0);
    args.refOriginalPoints() << QPointF(3.0, 4.0);
    args.refOriginalPoints() << QPointF(4.0, 5.0);

    args.refTransformedPoints() << QPointF(6.0, 7.0);
    args.refTransformedPoints() << QPointF(7.0, 8.0);
    args.refTransformedPoints() << QPointF(8.0, 9.0);
    args.refTransformedPoints() << QPointF(9.0, 8.0);
    args.setWarpType(KisWarpTransformWorker::RIGID_TRANSFORM);
    args.setAlpha(0.5);
    args.setPixelPrecision(8);
    args.setPreviewPixelPrecision(16);

    QDomDocument doc("test_type");
    QDomElement root = doc.createElement("root");
    doc.appendChild(root);

    args.toXML(&root);

    ToolTransformArgs newArgs = ToolTransformArgs::fromXML(root);
    QCOMPARE(newArgs, args);
}

void TestSaveLoadTransformArgs::testLiquify()
{
    ToolTransformArgs args;

    args.setMode(ToolTransformArgs::LIQUIFY);
    args.initLiquifyTransformMode(QRect(100, 200, 300, 400));


    args.liquifyProperties()->setMode(KisLiquifyProperties::ROTATE);
    args.liquifyProperties()->setSize(101);
    args.liquifyProperties()->setAmount(0.5);
    args.liquifyProperties()->setSpacing(0.87);
    args.liquifyProperties()->setSizeHasPressure(true);
    args.liquifyProperties()->setAmountHasPressure(true);
    args.liquifyProperties()->setReverseDirection(true);
    args.liquifyProperties()->setUseWashMode(true);
    args.liquifyProperties()->setFlow(0.63);

    args.liquifyWorker()->rotatePoints(QPointF(150, 250),
                                       M_PI / 3,
                                       50.0,
                                       false,
                                       1.0);

    QDomDocument doc("test_type");
    QDomElement root = doc.createElement("root");
    doc.appendChild(root);

    args.toXML(&root);

    ToolTransformArgs newArgs = ToolTransformArgs::fromXML(root);
    QCOMPARE(newArgs, args);
}

QTEST_MAIN(TestSaveLoadTransformArgs)
