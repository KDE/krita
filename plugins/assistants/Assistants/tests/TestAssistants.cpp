/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "TestAssistants.h"

#include <testui.h>
#include <testutil.h>


#include <kis_painting_assistant.h>
#include <PerspectiveBasedAssistantHelper.h>
#include <kis_algebra_2d.h>
#include <kis_global.h>

#include <ConcentricEllipseAssistant.h>
#include "EllipseInPolygon.h"

void TestAssistants::testConcentricEllipseAdjustLine()
{
    ConcentricEllipseAssistantFactory factory;
    KisPaintingAssistantSP assistant = KisPaintingAssistantSP(factory.createPaintingAssistant());
    //ConcentricEllipseAssistant* ellipse = new ConcentricEllipseAssistant();
    ConcentricEllipseAssistant* ellipse = dynamic_cast<ConcentricEllipseAssistant*>(assistant.data());
    QVERIFY(ellipse);

    ellipse->addHandle(new KisPaintingAssistantHandle(0, 100), HandleType::NORMAL);
    ellipse->addHandle(new KisPaintingAssistantHandle(100, 0), HandleType::NORMAL);
    ellipse->addHandle(new KisPaintingAssistantHandle(200, 200), HandleType::NORMAL);

    QPointF begin = QPointF(0, 100);
    //QPointF end = QPointF(100, 5);
    //QPointF end = QPointF(100, 0);
    //ellipse->adjustLine(end, begin);
    //ENTER_FUNCTION() << begin << end;

    ENTER_FUNCTION() << "Begin is " << begin;

    QList<QPointF> ends;
    ends << QPointF(100, 0) << QPointF(100, 5) << QPointF(200, 200) << QPointF(400, 400);

    for (int i = 0; i < ends.size(); i++) {
        QPointF endHere = ends[i];
        ellipse->adjustLine(endHere, begin);
        ENTER_FUNCTION() << ends[i] << "=>" << endHere;
    }


    begin = QPointF(0, 200);

    ENTER_FUNCTION() << "Begin is " << begin;

    ends.clear();
    ends << QPointF(200, 0) << QPointF(200, 5) << QPointF(400, 400) << QPointF(500, 500);

    for (int i = 0; i < ends.size(); i++) {
        QPointF endHere = ends[i];
        ellipse->adjustLine(endHere, begin);
        ENTER_FUNCTION() << ends[i] << "=>" << endHere;
    }



}

void TestAssistants::testMirroringPoints()
{

    auto mirror = [] (QPointF p, QLineF l) {
        // first translate
        // then mirror
        // then retranslate

        //Eigen::Matrix2f mirror;
        //Eigen::Vector2f b;
        qreal cos = qCos(qDegreesToRadians(-2*l.angle()));
        qreal sin = qSin(qDegreesToRadians(-2*l.angle()));

        ENTER_FUNCTION() << ppVar(l.angle()) << ppVar(cos) << ppVar(sin);
        // mirror transformation:
        // | cos2a  sin2a 0 |
        // | sin2a -cos2a 0 |
        // |     0      0 1 |


        QTransform t1;
        t1.fromTranslate(l.p1().x(), l.p1().y());
        QTransform t2;
        t2.setMatrix(cos, sin, 0, sin, -cos, 0, 0, 0, 1);
        QTransform t3;
        t3.fromTranslate(-l.p1().x(), -l.p1().y());
        QTransform all = t1*t2*t3;

        ENTER_FUNCTION() << ppVar(all);
        ENTER_FUNCTION() << ppVar(t2);
        ENTER_FUNCTION() << ppVar(t1.map(p)) << ppVar((t1*t2).map(p)) << ppVar((t1*t2*t3).map(p));

        return all.map(p);
    };

    ENTER_FUNCTION() << ppVar(mirror(QPointF(1, 1), QLineF(QPointF(0, 0), QPointF(0, 1))));
    ENTER_FUNCTION() << ppVar(mirror(QPointF(-100, -80), QLineF(QPointF(2, 2), QPointF(5, 5))));
    ENTER_FUNCTION() << ppVar(mirror(QPointF(100, 80), QLineF(QPointF(2, 2), QPointF(5, 5))));

    QCOMPARE(mirror(QPointF(100, 80), QLineF(QPointF(2, 2), QPointF(5, 5))), QPointF(80, 100));
    QCOMPARE(mirror(QPointF(-100, -80), QLineF(QPointF(2, 2), QPointF(5, 5))), QPointF(-80, -100));
    QCOMPARE(mirror(QPointF(-100, 80), QLineF(QPointF(2, 2), QPointF(5, 5))), QPointF(-80, 100));

    QCOMPARE(mirror(QPointF(100, 80), QLineF(QPointF(2, -2), QPointF(5, -5))), QPointF(80, 100));
    QCOMPARE(mirror(QPointF(-100, -80), QLineF(QPointF(2, -2), QPointF(5, -5))), QPointF(80, 100));
    QCOMPARE(mirror(QPointF(-100, 80), QLineF(QPointF(2, -2), QPointF(5, -5))), QPointF(-80, 100));


}


void TestAssistants::testProjection()
{
    EllipseInPolygon concentric;
    EllipseInPolygon original;

    QPointF point;


    // test 1:
    // just a normal circle 0-1
    original.updateToPolygon(QPolygonF(QRectF(0.0, 0.0, 1.0, 1.0)), QLineF(0.0, 0.0, 1.0, 0.0));
    point = QPointF(3, 3);


    concentric.updateToPointOnConcentricEllipse(original.originalTransform, point, original.horizon);

    qCritical() << concentric.project(point) << "should be " << point;
    qCritical() << ppVar(concentric.curveType);
    qCritical() << ppVar(concentric.finalEllipseCenter);
    qCritical() << ppVar(concentric.finalVertices);

    QCOMPARE(concentric.project(point), point);

    // test 2: ellipse with axis parallel to the 0X

    original.updateToPolygon(QPolygonF(QRectF(-0.5, 0.0, 1.5, 1.0)), QLineF(0.0, 0.0, 1.0, 0.0));
    point = QPointF(0.5, 3);


    concentric.updateToPointOnConcentricEllipse(original.originalTransform, point, original.horizon);

    qCritical() << concentric.project(point) << "should be " << point;
    qCritical() << ppVar(concentric.curveType);
    qCritical() << ppVar(concentric.finalEllipseCenter);
    qCritical() << ppVar(concentric.finalVertices);






}






KISTEST_MAIN(TestAssistants)
