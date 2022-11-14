/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "TestAssistants.h"
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


void TestAssistants::testProjectionNewMethodTest2()
{

    //QFETCH(EllipseInPolygon, ellipse);
    //QFETCH(QPointF, point);

    qreal eps = 0.00001;

    auto doTest = [eps] (EllipseInPolygon ellipse, QPointF point) {

        QPointF res = ellipse.projectModifiedEberly(point);

        auto form = [] (QPointF p, EllipseInPolygon f) {
            return f.finalFormula[0]*p.x()*p.x()
                    + f.finalFormula[1]*p.x()*p.y()
                    + f.finalFormula[2]*p.y()*p.y()
                    + f.finalFormula[3]*p.x()
                    + f.finalFormula[4]*p.y()
                    + f.finalFormula[5];
        };
        qreal formValue = form(res, ellipse);
        if (formValue >= eps) {
            ENTER_FUNCTION() << ppVar(ellipse.finalFormula) << ppVar(point) << ppVar(res) << ppVar(formValue);
        }
        QVERIFY(formValue < eps);
        //return formValue < eps;
    };


    QRandomGenerator gen;

    auto genPoint = [gen] (int a, int b) mutable {
        return QPointF(gen.bounded(b - a) + a, gen.bounded(b - a) + a);
    };


    EllipseInPolygon original;
    QPolygonF poly;
    poly = QPolygonF();
    poly << QPointF(0, 0) << QPointF(1, 0) << QPointF(1, 1) << QPointF(0, 1);

    QPointF point;

    // test 1:
    // just a normal circle 0-1
    original.updateToPolygon(poly, QLineF(0.0, 0.0, 1.0, 0.0));
    //original.updateToPolygon(QPolygonF(QRectF(0.0, 0.0, 1.0, 1.0)), QLineF(0.0, 0.0, 1.0, 0.0));
    point = QPointF(3, 3);

    for (int i = 0; i < 100; i++) {
        QString testName = "circle" + QString::number(i);
        doTest(original, QPointF(gen.bounded(10.0) - 5.0, gen.bounded(10.0) - 5.0));
    }


    original.updateToPolygon(poly, QLineF(0.0, 0.0, 1.0, 0.0));
    //original.updateToPolygon(QPolygonF(QRectF(0.0, 0.0, 1.0, 1.0)), QLineF(0.0, 0.0, 1.0, 0.0));
    point = QPointF(3, 3);


    for (int i = 0; i < 100; i++) {
        QString testName = "circle" + QString::number(i);
        poly = QPolygonF();
        poly << genPoint(-5, 5) << genPoint(-5, 5) << genPoint(-5, 5) << genPoint(-5, 5);
        original.updateToPolygon(poly, QLineF(0.0, 0.0, 1.0, 0.0));
        doTest(original, genPoint(-5, 5));
    }

}

void TestAssistants::testProjectionNewMethod()
{
    EllipseInPolygon concentric;
    EllipseInPolygon original;
    ENTER_FUNCTION() << "(1)";

    qreal eps = 1e-6;

    QPointF point;


    // test 1:
    // just a normal circle 0-1

    QPolygonF poly = QPolygonF(QRectF(0.0, 0.0, 2.0, 2.0));
    ENTER_FUNCTION() << poly;
    poly = QPolygonF();
    poly << QPointF(0, 0) << QPointF(1, 0) << QPointF(1, 1) << QPointF(0, 1);

    original.updateToPolygon(poly, QLineF(0.0, 0.0, 1.0, 0.0));

    ENTER_FUNCTION() << "(2)";

    point = QPointF(3, 3);

    QPointF result;
    /*
    QPointF result = original.projectModifiedEberly(point);
    ENTER_FUNCTION() << ppVar(result);
    */

    auto form = [] (QPointF p, EllipseInPolygon f) {
        return f.finalFormula[0]*p.x()*p.x()
                + f.finalFormula[1]*p.x()*p.y()
                + f.finalFormula[2]*p.y()*p.y()
                + f.finalFormula[3]*p.x()
                + f.finalFormula[4]*p.y()
                + f.finalFormula[5];
    };


    /*
    QRandomGenerator gen;
    qreal eps = 0.00001;
    for (int i = 0; i < 100; i++) {
        QPointF p(gen.bounded(5.0), gen.bounded(5.0));
        QPointF res = original.projectModifiedEberly(p);
        ENTER_FUNCTION() << "Result is on the ellipse: " << form(res, original);
        QVERIFY(form(res, original) < eps);
    }
    */

    point = QPointF(0.0226453,-2.99636);

    ENTER_FUNCTION() << "************************ START **************************";

    result = original.projectModifiedEberlySecond(point);
    ENTER_FUNCTION() << ppVar(result);
    ENTER_FUNCTION() << ppVar(result) << ppVar(form(result, original));

    ENTER_FUNCTION() << "############       RESULT =      " << form(result, original);
    QVERIFY(form(result, original) < eps);

    ENTER_FUNCTION() << "************************ START 2 **************************";

    poly.clear();
    poly << QPointF(704.529,342.744)  << QPointF(1049.58,529.788) << QPointF(683.107,1006.81)
         << QPointF(349.884,528.397);
    original.updateToPolygon(poly, QLineF());
    point = QPointF(749.893,461.21);

    result = original.projectModifiedEberlySecond(point);
    ENTER_FUNCTION() << ppVar(result);
    ENTER_FUNCTION() << ppVar(result) << ppVar(form(result, original));

    ENTER_FUNCTION() << "############       RESULT =      " << form(result, original);
    QVERIFY(form(result, original) < eps);


    ENTER_FUNCTION() << "************************ START 3 **************************";
    // Test 3.
    // ellipse: 2x^2 + xy + 1y^2 = 5
    // so: 2, 1, 2, 0, 0, -5

    original.finalFormula.clear();
    original.finalFormula << 2 << 1 << 2 << 0 << 0 << -5;

    point = QPointF(5, 5);


    result = original.projectModifiedEberlySecond(point);
    ENTER_FUNCTION() << ppVar(result);
    ENTER_FUNCTION() << ppVar(result) << ppVar(form(result, original));

    ENTER_FUNCTION() << "############       RESULT =      " << form(result, original);
    QVERIFY(form(result, original) < eps);


    ENTER_FUNCTION() << "************************ START 4 ************************";
    // Test 3.
    // ellipse: 2x^2 + xy + 1y^2 = 5
    // so: 2, 1, 2, 0, 0, -5



    original.finalFormula.clear();
    original.finalFormula << 2 << 1 << 2 << 0 << 0 << -5;

    point = QPointF(0, sqrt(5/2.0));


    result = original.projectModifiedEberlySecond(point);
    ENTER_FUNCTION() << ppVar(result);
    ENTER_FUNCTION() << ppVar(result) << ppVar(form(result, original));

    ENTER_FUNCTION() << "############       RESULT =      " << form(result, original);
    QVERIFY(form(result, original) < eps);



    ENTER_FUNCTION() << "************************ START 5 ************************";
    // Test 3.
    // ellipse: 2x^2 + xy + 1y^2 = 5
    // so: 2, 1, 2, 0, 0, -5


    original.finalFormula.clear();
    original.finalFormula << 2 << 1 << 2 << 0 << 0 << -5;

    point = QPointF(sqrt(5/2.0), sqrt(5/2.0));

    result = original.projectModifiedEberlySecond(point);
    ENTER_FUNCTION() << ppVar(result);
    ENTER_FUNCTION() << ppVar(result) << ppVar(form(result, original));

    ENTER_FUNCTION() << "############       RESULT =      " << form(result, original);
    QVERIFY(form(result, original) < eps);


    ENTER_FUNCTION() << "************************ START 6 ************************";
    // Test 3.
    // ellipse: 2x^2 + xy + 1y^2 = 5
    // so: 2, 1, 2, 0, 0, -5



    original.finalFormula.clear();
    original.finalFormula << 2 << 1 << 2 << 0 << 0 << -5;

    point = QPointF(0, -sqrt(5/2.0) -5.0);

    result = original.projectModifiedEberlySecond(point);
    ENTER_FUNCTION() << ppVar(result);
    ENTER_FUNCTION() << ppVar(result) << ppVar(form(result, original));

    ENTER_FUNCTION() << "############       RESULT =      " << form(result, original);


    //QVERIFY(form(result, original) < eps);
    QVERIFY(form(result, original) < eps);



    ENTER_FUNCTION() << "************************ START 7 (real, but nicer numbers) **************************";


    poly.clear();
    poly << QPointF(700,350)  << QPointF(1050,530) << QPointF(700,1000)
         << QPointF(350,530);
    original.updateToPolygon(poly, QLineF());
    point = QPointF(750,460);

    result = original.projectModifiedEberlySecond(point);
    ENTER_FUNCTION() << ppVar(result);
    ENTER_FUNCTION() << ppVar(result) << ppVar(form(result, original));

    ENTER_FUNCTION() << "############       RESULT =      " << form(result, original);
    QVERIFY(form(result, original) < eps);


    ENTER_FUNCTION() << "************************ START 8 (real 2, leading to the \"weird situation\") **************************";

    // The values were:  polygon = QVector(QPointF(704.529,342.744), QPointF(1049.58,529.788), QPointF(683.107,1006.81), QPointF(349.884,528.397))
    // originalPoint = QPointF(1067.62,719.146) and unfortunate result: result = QPointF(1057.48,709.158)
    poly.clear();
    poly << QPointF(704.529,342.744) << QPointF(1049.58,529.788) << QPointF(683.107,1006.81) << QPointF(349.884,528.397);
    original.updateToPolygon(poly, QLineF());
    point = QPointF(1067.62,719.146);

    result = original.projectModifiedEberlySecond(point);
    ENTER_FUNCTION() << ppVar(result);
    ENTER_FUNCTION() << ppVar(result) << ppVar(form(result, original));

    ENTER_FUNCTION() << "############       RESULT =      " << form(result, original);
    QVERIFY(form(result, original) < eps);

    ENTER_FUNCTION() << "************************ START 9 (real 3, leading to the fd being zero in Newton, and then nans) **************************";

    // The values were:  polygon = QVector(QPointF(704.529,342.744), QPointF(1049.58,529.788), QPointF(683.107,1006.81), QPointF(349.884,528.397))
    // originalPoint = QPointF(1067.62,719.146) and unfortunate result: result = QPointF(1057.48,709.158)
    poly.clear();
    poly << QPointF(704.529,342.744) << QPointF(1049.58,529.788) << QPointF(722.562,626.904) << QPointF(349.884,528.397);
    original.updateToPolygon(poly, QLineF());
    point = QPointF(553.452,264.769);

    result = original.projectModifiedEberlySecond(point);
    ENTER_FUNCTION() << ppVar(result);
    ENTER_FUNCTION() << ppVar(result) << ppVar(form(result, original));

    ENTER_FUNCTION() << "############       RESULT =      " << form(result, original);

    QVERIFY(form(result, original) < eps);


    // polygon = QVector(QPointF(704.529,342.744), QPointF(1049.58,529.788), QPointF(722.562,626.904), QPointF(349.884,528.397)) point = QPointF(553.452,264.769)




}






KISTEST_MAIN(TestAssistants)
