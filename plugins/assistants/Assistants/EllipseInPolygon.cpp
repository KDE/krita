/*
 * SPDX-FileCopyrightText: 2022 Srirupa Datta <srirupa.sps@gmail.com>
 */

#include "PerspectiveEllipseAssistant.h"
#include "PerspectiveBasedAssistantHelper.h"


#include <klocalizedstring.h>
#include "kis_debug.h"
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QTransform>

#include <kis_canvas2.h>
#include <kis_coordinates_converter.h>
#include "kis_algebra_2d.h"
#include <kis_dom_utils.h>
#include <Eigen/Eigenvalues>
#include "EllipseInPolygon.h"

#include <math.h>
#include<QDebug>
#include <QtMath>

#include <functional>


// ################################## Ellipse in Polygon (in Perspective) #######################################



EllipseInPolygon::EllipseInPolygon()
{
    finalFormula.clear();
    rerotatedFormula.clear();
    finalFormula << 1 << 0 << 1 << 0 << 0 << 0;
    rerotatedFormula << 1 << 0 << 1 << 0 << 0 << 0;

    finalEllipseCenter.clear();
    finalEllipseCenter << 0 << 0;

    finalVertices.clear();
    finalVertices << QPointF(-1, 0) << QPointF(1, 0) << QPointF(0, 1);
}

bool EllipseInPolygon::updateToPolygon(QVector<QPointF> _polygon, QLineF horizonLine)
{
    QTransform transform;

    m_valid = false; // let's make it false in case we return in the middle of the work
    polygon = _polygon; // the assistant needs to know the polygon even when it doesn't allow for a correct ellipse


    // this calculates the perspective transform that represents the current quad (polygon)
    // this is the transform that changes the original (0, 0, 1, 1) square to the quad
    // that means that our "original" ellipse is the circle in that square (with center in (0.5, 0.5), and radius 0.5)
    if (!QTransform::squareToQuad(polygon, transform)) {
        return false;
    }

    originalTransform = transform;

    // using the perspective transform, we can calculate some points on the ellipse
    // any points from the original ellipse would work here
    // but pt1-4 are just the simplest ones to write
    // and pR is another one easy to calculate (common point between the original ellipse and a line `y = x`)
    QPointF pt1 = originalTransform.map(QPointF(0.5, 1.0));
    QPointF pt2 = originalTransform.map(QPointF(1.0, 0.5));
    QPointF pt3 = originalTransform.map(QPointF(0.5, 0.0));
    QPointF pt4 = originalTransform.map(QPointF(0.0, 0.5));
    // a point on the ellipse and on the `y = x` line
    QPointF ptR = originalTransform.map(QPointF(0.5 - 1/(2*sqrt(2)), 0.5 - 1/(2*sqrt(2))));


    return updateToFivePoints({pt1, pt2, pt3, pt4, ptR}, horizonLine);


}

bool EllipseInPolygon::updateToPointOnConcentricEllipse(QTransform _originalTransform, QPointF pointOnConcetric, QLineF horizonLine)
{
    m_valid = false; // let's make it false in case we return in the middle of the work
    if (!_originalTransform.isInvertible()) {
        return false;
    }

    QTransform inverted = _originalTransform.inverted();

    QPointF pointInOriginalCoordinates = inverted.map(pointOnConcetric);

    QPointF center = QPointF(0.5, 0.5);
    double distanceFromCenter = kisDistance(pointInOriginalCoordinates, center);

    originalCircleRadius = distanceFromCenter;
    qCritical() << "distance from center" << distanceFromCenter;

    originalTransform = _originalTransform;

    QVector<QPointF> points;
    points << originalTransform.map(pointInOriginalCoordinates)
           << originalTransform.map(center + QPointF(distanceFromCenter, 0))
           << originalTransform.map(center + QPointF(-distanceFromCenter, 0))
           << originalTransform.map(center + QPointF(0, distanceFromCenter))
           << originalTransform.map(center + QPointF(0, -distanceFromCenter));

    concentricDefiningPointIndex = 0;

    ENTER_FUNCTION() << "Points are: " << pointInOriginalCoordinates
                     << center + QPointF(distanceFromCenter, 0) << center + QPointF(-distanceFromCenter, 0);

    // FIXME: it's good to update the polygon too, just in case




    return updateToFivePoints(points, horizonLine);
}

QPointF EllipseInPolygon::project(QPointF point)
{
    // normal line approach
    // so, for hyperbolas, the normal line is:
    // F(x - x0) - E(y - y0) = 0
    // F = Axy * x0 + Ayy * y0 + By
    // E = Axx * x0 + Axy * y0 + Bx
    // x0, y0 => point on the hyperbola
    // Axx = a
    // Axy = b/2
    // Ayy = c
    // Bx = d/2
    // By = e/2
    // the same lines goes through another point, which is exactly 'point'
    // let's call it P
    //

    QPointF response;
    qreal a = finalFormula[0];
    qreal b = finalFormula[1];
    qreal c = finalFormula[2];
    qreal d = finalFormula[3];
    qreal e = finalFormula[4];
    qreal f = finalFormula[5];

    qreal eps = 0.000001;
    qCritical() << "abc etc. inside project()" << a << b << c << d << e << f;
    if (qAbs(b) >= eps) {

        qreal y = (2*a*point.x() + b*point.y() + d)/b;
        qreal L = b*point.x() + 2*c*point.y() + e;
        qreal x = 2*(a - c)*y/b - L/b;

        //qCritical() << a << b << c<< d << e << f;
        qCritical() << y << L << x;

        response = QPointF(x + point.x(), y + point.y());
        return response;
    } else {
        return point;
    }




    /*

    // use naive option first:
    if (!originalTransform.isInvertible()) {
        return point;
    }
    QTransform reverseTransform = originalTransform.inverted();
    QPointF pInOriginalCoords = reverseTransform.map(point);
    QPointF pOriginalMoved = pInOriginalCoords - QPointF(0.5, 0.5);
    qreal distanceFromCenter = kisDistance(QPointF(0.5, 0.5), reverseTransform.map(originalPoints[0]));
    QPointF onCircle;
    // x^2 + y^2 = (0.5)^2
    // and ax + by = 0 (line going through the (0, 0) point)
    // by = -ax
    // y = -ax/b = -a/b * x
    // x^2 + (-a/b)^2 * x^2 = (0.5)^2
    // (1 + (a/b)^2)* x^2 = (0.5)^2
    // x^2 = (0.5)^2 / (1 + (a/b)^2)
    // x = 0.5 / sqrt(1 + (a/b)^2)
    // and y = -a/b * x

    // how to get a/b:
    // -a/b = y/x
    // a/b = -y/x

    if (pOriginalMoved.x() == 0) {
        onCircle = QPointF(0, distanceFromCenter*0.5*(KisAlgebra2D::signPZ(pOriginalMoved.y())));
    } else {
        // b != 0
        qreal ab = -pOriginalMoved.y()/pOriginalMoved.x();
        qreal onCircleX = KisAlgebra2D::signPZ(pOriginalMoved.x())*0.5*distanceFromCenter/(qSqrt(1 + ab*ab));
        onCircle = QPointF(onCircleX, -ab*onCircleX);
    }
    return originalTransform.map(onCircle + QPointF(0.5, 0.5));

    */

}

bool EllipseInPolygon::setSimpleEllipseVertices(Ellipse &ellipse) const
{
    if (finalVertices.size() > 2) {
        return ellipse.set(finalVertices[0], finalVertices[1], finalVertices[2]);
    }
    return false;
}

bool EllipseInPolygon::formulaRepresentsAnEllipse(double a, double b, double c)
{
    ENTER_FUNCTION() << "(b*b - 4*a*c)" << (b*b - 4*a*c) << " < 0 (should be for ellipse)" << a << b << c;
    return (b*b - 4*a*c) < 0;
}

EllipseInPolygon::CURVE_TYPE EllipseInPolygon::curveTypeForFormula(double a, double b, double c)
{
    double condition = (b*b - 4*a*c);
    if (condition == 0) {
        return PARABOLA;
    } else if (condition > 0) {
        return HYPERBOLA;
    } else {
        return ELLIPSE;
    }
}

void EllipseInPolygon::paintParametric(QPainter &gc, const QRectF &updateRect, const QTransform &initialTransform)
{
    QPointF beginPoint = updateRect.topLeft();

    // go left and right from that point
    paintParametric(gc, updateRect, initialTransform, beginPoint, true);
    paintParametric(gc, updateRect, initialTransform, beginPoint, false);
}

void EllipseInPolygon::paintParametric(QPainter& gc, const QRectF& updateRect, const QTransform &initialTransform, const QPointF& begin, bool goesLeft)
{
    gc.save();
    gc.resetTransform();
    gc.setTransform(QTransform());

    // parametric formula for y:
    // if c == 0:
    // x = t
    // y = (f + at^2 + dt)/(e + bt)
    // if c != 0:
    // x = t
    // y = (-e -bt +- sqrt(e^2 + b^2*t^2 + 2ebt - 4cf - 4cat^2 - 4cdt)/(2c))
    // parametric formula for x is symmetric (a <=> c, d <=> e)


    qreal a = finalFormula[0];
    qreal b = finalFormula[1];
    qreal c = finalFormula[2];
    qreal d = finalFormula[3];
    qreal e = finalFormula[4];
    qreal f = finalFormula[5];

    //QPointF beingWidget = initialTransform.map(begin);


    qreal jump = 2; // for now

    //int sign = goesLeft ? 1 : -1;

    bool needsSecondLoop = false;

    if (!initialTransform.isInvertible()) {
        return;
    }

    QTransform reverseInitial = initialTransform.inverted();

    for (int i = 0; i < originalPoints.size(); i++) {
        gc.drawEllipse(initialTransform.map(originalPoints[i]), 10, 10); // good points!!!
        gc.drawEllipse(originalPoints[i], 15, 15); // they are in that bad rectangle
    }

    gc.drawEllipse(initialTransform.map(updateRect.topLeft()), 25, 25); // good points!!!
    gc.drawEllipse(updateRect.topLeft(), 25, 25); // they are in that bad rectangle
    gc.drawEllipse(initialTransform.map(updateRect.bottomRight()), 25, 25); // good points!!!
    gc.drawEllipse(updateRect.bottomRight(), 25, 25); // they are in that bad rectangle




    if (!formulaRepresentsAnEllipse(a, b, c)) {
        needsSecondLoop = true;
    }


    QRect viewport = gc.viewport();
    QPolygonF viewportMappedPoly = reverseInitial.map((QPolygonF)(QRectF)viewport);
    QRect viewportMapped = viewportMappedPoly.boundingRect().toRect();


    QPointF concentricMovingPoint = originalPoints[concentricDefiningPointIndex];

    QPointF concentricStaticPoint = originalTransform.map(QPointF(0.5, 0.5)); // this is the center of the original (TM) circle

    qreal horizonSideForMovingPoint = horizonLineSign(concentricMovingPoint);
    qreal horizonSideForStaticPoints = horizonLineSign(concentricStaticPoint);

    bool showMirrored = horizonSideForMovingPoint != horizonSideForStaticPoints;


    auto formulaWithSquare = [a, b, c, d, e, f, &needsSecondLoop, this, initialTransform, horizonSideForStaticPoints] (qreal x, int sign) mutable {

        qreal underSqrt = e*e + b*b*x*x + 2*e*b*x - 4*c*f - 4*c*a*x*x - 4*c*d*x;

        if (underSqrt < 0) {
            return QPointF();
        } else if (underSqrt > 0) {
            needsSecondLoop = true;
        }

        qreal y = (-e - b*x + sign*sqrt(underSqrt))/(2*c);

        int horizonSide = horizonLineSign(QPointF(x, y));
        if (horizonSide != horizonSideForStaticPoints) {
            return QPointF();
        }

        QPointF newPoint = initialTransform.map(QPointF(x, y));
        return newPoint;
    };

    auto formulaWithoutSquare = [a, b, c, d, e, f, &needsSecondLoop, this, initialTransform, horizonSideForStaticPoints] (qreal x, int sign) mutable {

        // y = (f + at^2 + dt)/(e + bt)
        qreal under = e + b*x;

        if (under == 0) {
            return QPointF();
        }

        qreal y = (f + a*x*x + d*x)/under;

        int horizonSide = horizonLineSign(QPointF(x, y));
        if (horizonSide != horizonSideForStaticPoints) {
            return QPointF();
        }

        QPointF newPoint = initialTransform.map(QPointF(x, y));
        return newPoint;
    };

    auto formula = formulaWithSquare;
    if (c == 0) {
        //formula = formulaWithoutSquare;
    }
    //auto formula = (c == 0 ? formulaWithoutSquare : formulaWithSquare);




    if (c != 0) {

        //QPointF lastPoint = QPointF();

        QPainterPath path;
        QVector<QPointF> points;

        QVector<int> signs = {-1, 1};



        Q_FOREACH(int sign, signs) {
            for (int i = viewport.left(); i <= viewport.right(); i += jump) {


                //QPointF converted = QPointF(i, begin.y());//reverseInitial.map(QPointF(i, begin.y()));
                QPointF converted = reverseInitial.map(QPointF(i, begin.y()));



                qreal x = converted.x();


                /*

                //qreal underSqrt = e*e + b*b*i*i + 2*e*b*i - 4*c*f - 4*c*a*i*i - 4*c*d*i;
                qreal underSqrt = e*e + b*b*x*x + 2*e*b*x - 4*c*f - 4*c*a*x*x - 4*c*d*x;

                if (underSqrt < 0) {
                    path.addPolygon(QPolygonF(points));
                    points.clear();
                    continue;
                } else if (underSqrt > 0) {
                    needsSecondLoop = true;
                }

                qreal y = (-e - b*i + sign*sqrt(underSqrt))/(2*c);

                int horizonSide = horizonLineSign(QPointF(x, y));
                if (horizonSide != horizonSideForStaticPoints) {
                    path.addPolygon(QPolygonF(points));
                    points.clear();
                    continue;
                }

                QPointF newPoint = initialTransform.map(QPointF(x, y));


                if (i == viewport.left()) {
                    gc.drawEllipse(newPoint, 5, 5);
                    gc.drawEllipse(QPointF(x, y), 5, 5);
                }

                */
                QPointF newPoint = formulaWithSquare(x, sign);

                if (newPoint.isNull()) {
                    path.addPolygon(points);
                    points.clear();
                } else {
                    points << newPoint;
                }
            }

            path.addPolygon(points);
            points.clear();

            if (!needsSecondLoop) {
                break;
            }

        }


        gc.drawPath(path);

        gc.drawRect(kisGrowRect(updateRect, -10));
        gc.drawRect(kisGrowRect(viewport, -10));


    }
    gc.restore();

}

int EllipseInPolygon::horizonLineSign(QPointF point)
{
    qreal result = horizonFormula[0]*point.x() + horizonFormula[1]*point.y() + horizonFormula[2];
    return (result == 0 ? 0 : (result < 0 ? -1 : 1));
}

QPointF EllipseInPolygon::mirrorPoint(QPointF point, QLineF horizon)
{
    QPointF p = point;
    QLineF l = horizon;
    // first translate
    // then mirror
    // then retranslate
    qreal cos = qCos(qDegreesToRadians(-2*l.angle()));
    qreal sin = qSin(qDegreesToRadians(-2*l.angle()));

    //ENTER_FUNCTION() << ppVar(l.angle()) << ppVar(cos) << ppVar(sin);
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

    //ENTER_FUNCTION() << ppVar(all);
    //ENTER_FUNCTION() << ppVar(t2);
    //ENTER_FUNCTION() << ppVar(t1.map(p)) << ppVar((t1*t2).map(p)) << ppVar((t1*t2*t3).map(p));

    return all.map(p);
}

void EllipseInPolygon::setFormula(QVector<double> &formula, double a, double b, double c, double d, double e, double f)
{
    if (formula.size() != 6) {
        formula.clear();
        formula << a << b << c << d << e << f;
    } else {
        formula[0] = a;
        formula[1] = b;
        formula[2] = c;
        formula[3] = d;
        formula[4] = e;
        formula[5] = f;
    }
}

void EllipseInPolygon::setPoint(QVector<double> &point, double x, double y)
{
    if (point.size() != 2) {
        point.clear();
        point << x << y;
    } else {
        point[0] = x;
        point[1] = y;
    }
}

bool EllipseInPolygon::updateToFivePoints(QVector<QPointF> points, QLineF _horizon)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(points.length() >= 5, false);

    QPointF pt1 = points[0];
    QPointF pt2 = points[1];
    QPointF pt3 = points[2];
    QPointF pt4 = points[3];

    QPointF ptR = points[4];

    originalPoints = points;


    horizon = _horizon;
    // horizon line formula:
    horizonFormula.clear();
    if (!horizon.isNull()) {
        // final formula should be: ax + by + c = 0
        // for two points, the line equation is:
        // (y - y0)*(x1 - x0) = (y1 - y0)*(x - x0)
        // (x1 - x0)*y + (-y0*(x1 - x0)) = (y1 - y0)*x + (-x0*(y1 - y0))
        // (x1 - x0)*y + (-y0*(x1 - x0)) + (y0 - y1)*x + (-x0*(y0 - y1)) = 0
        // a = (y0 - y1)
        // b = (x1 - x0)
        // c = -y0*b -x0*a
        qreal a = horizon.p1().y() - horizon.p2().y();
        qreal b = horizon.p2().x() - horizon.p1().x();
        qreal c = -horizon.p1().y()*b - horizon.p1().x()*a;
        horizonFormula << a << b << c;
    }



    // using the points from above (pt1-4 and ptR) we can construct a linear equation for the final ellipse formula
    // the general ellipse formula is: `ax^2 + bxy + cy^2 + dx + ey + f = 0`
    // but since a cannot ever be 0, we can temporarily reduce the formula to be `x^2 + Bxy + Cy^2 + Dx + Ey + F = 0`
    // where B = b/a etc.
    Eigen::MatrixXd A(5, 5);
    A <<          ptR.x() * ptR.y(), ptR.y() * ptR.y(), ptR.x(), ptR.y(), 1.0,
                  pt1.x() * pt1.y(), pt1.y() * pt1.y(), pt1.x(), pt1.y(), 1.0,
                  pt2.x() * pt2.y(), pt2.y() * pt2.y(), pt2.x(), pt2.y(), 1.0,
                  pt3.x() * pt3.y(), pt3.y() * pt3.y(), pt3.x(), pt3.y(), 1.0,
                  pt4.x() * pt4.y(), pt4.y() * pt4.y(), pt4.x(), pt4.y(), 1.0;

    Eigen::VectorXd bVector(5);
    bVector << - ptR.x() * ptR.x(), - pt1.x() * pt1.x(), - pt2.x() * pt2.x(),  - pt3.x() * pt3.x(), - pt4.x() * pt4.x();

    Eigen::VectorXd xSolution = A.fullPivLu().solve(bVector);

    // generic ellipse formula coefficients for the final formula
    // assigned to new variables to better see the calculations
    // (even with "x" as a solution vector variable, it would be difficult to spot error when everything looks like x(2)*x(4)/x(3)*x(1) etc.)
    qreal a = 1;
    qreal b = xSolution(0);

    qreal c = xSolution(1);
    qreal d = xSolution(2);

    qreal e = xSolution(3);
    qreal f = xSolution(4);

    // check if this is an ellipse
    if (!formulaRepresentsAnEllipse(a, b, c)) {
        ENTER_FUNCTION() << "Formula doesn't represent an ellipse:" << a << b << c << d << e << f;
        ENTER_FUNCTION() << "Is ellipse valid?" << isValid();
        // return false; moved later
    }

    setFormula(finalFormula, a, b, c, d, e, f);
    curveType = curveTypeForFormula(a, b, c);

    // check if this is an ellipse
    if (!formulaRepresentsAnEllipse(a, b, c)) {
        ENTER_FUNCTION() << "Formula doesn't represent an ellipse:" << a << b << c << d << e << f;
        ENTER_FUNCTION() << "Is ellipse valid?" << isValid();
        return false; //moved later
    }

    // x = (be - 2cd)/(4c - b^2)
    // y = (bd - 2e)/(4c - b^2)
    finalEllipseCenter.clear();
    finalEllipseCenter << ((double)b*e - 2*c*d)/(4*c - b*b) << ((double)b*d - 2*e)/(4*c - b*b);
    finalAxisAngle = qAtan2(b, a - c)/2;

    // use finalAxisAngle to find the cos and sin
    // and replace the final coordinate system with the rerotated one
    qreal K = qCos(-finalAxisAngle);
    qreal L = qSin(-finalAxisAngle);

    // this allows to calculate the formula for the rerotated ellipse
    qreal aprim = K*K*a - K*L*b + L*L*c;
    qreal bprim = 2*K*L*a + K*K*b - L*L*b - 2*K*L*c;
    qreal cprim = L*L*a + K*L*b + K*K*c;
    qreal dprim = K*d - L*e;
    qreal eprim = L*d + K*e;
    qreal fprim = f;

    if (!formulaRepresentsAnEllipse(aprim, bprim, cprim)) {
        //return false;
        // temporarily moved further away
    }

    finalAxisReverseAngleCos = K;
    finalAxisReverseAngleSin = L;

    setFormula(rerotatedFormula, aprim, bprim, cprim, dprim, eprim, fprim);


    if (!formulaRepresentsAnEllipse(aprim, bprim, cprim)) {
        return false;
    }

    // third attempt at new center:
    // K' = K
    // L' = -L
    // note that this will be in a different place, because the ellipse wasn't moved to have center in (0, 0), but still rotate around point (0,0)
    // and any point that is not (0, 0), when rotated around (0, 0) with an angle that isn't 0, 360 etc. degrees, will end up in a different place
    QPointF rerotatedCenter = QPointF(K*finalEllipseCenter[0] - L*finalEllipseCenter[1], K*finalEllipseCenter[1] + L*finalEllipseCenter[0]);

    qreal rx = sqrt(qPow(rerotatedCenter.x(), 2) + qPow(rerotatedCenter.y(), 2)*cprim/aprim - fprim/aprim);
    qreal ry = sqrt(rx*rx*aprim/cprim);

    axisXLength = rx;
    axisYLength = ry;

#if 0 // debug
    // they should be very close to cprim, dprim etc., when multiplied by aprim (since this only gives us a formula where aprim_recreated would be equal to 1)
    qreal cprim_recreated = (rx*rx)/(ry*ry);
    qreal dprim_recreated = -2*rerotatedCenter.x();
    qreal eprim_recreated = -2*rerotatedCenter.y()*(rx*rx)/(ry*ry);
    qreal fprim_recreated = qPow(rerotatedCenter.x(), 2) + qPow(rerotatedCenter.y(), 2)*(rx*rx)/(ry*ry) - (rx*rx);

    if (debug) qCritical() << "recreated equation (with 1): " << 1 << 0 << cprim_recreated << dprim_recreated << eprim_recreated << fprim_recreated;
    if (debug) qCritical() << "recreated equation: (actual)" << aprim << 0 << aprim*cprim_recreated << aprim*dprim_recreated << aprim*eprim_recreated << aprim*fprim_recreated;

    qreal eps = 0.00001;
    auto fuzzyCompareWithEps = [eps] (qreal a, qreal b) { return abs(a - b) < eps; };

    KIS_SAFE_ASSERT_RECOVER_NOOP(fuzzyCompareWithEps(aprim*cprim_recreated, cprim));
    KIS_SAFE_ASSERT_RECOVER_NOOP(fuzzyCompareWithEps(aprim*dprim_recreated, dprim));
    KIS_SAFE_ASSERT_RECOVER_NOOP(fuzzyCompareWithEps(aprim*eprim_recreated, eprim));
    KIS_SAFE_ASSERT_RECOVER_NOOP(fuzzyCompareWithEps(aprim*fprim_recreated, fprim));

#endif

    auto convertToPreviousCoordsSystem = [K, L] (QPointF p) { return QPointF(K*p.x() + L*p.y(), K*p.y() - L*p.x()); };

    // they most probably don't need a higher precision than float
    // (though they are used to calculate the brush position...)
    QPointF leftVertexRerotated = rerotatedCenter + QPointF(-rx, 0);
    QPointF rightVertedRerotated = rerotatedCenter + QPointF(rx, 0);
    QPointF topVertedRerotated = rerotatedCenter + QPointF(0, ry);

    QPointF leftVertexFinal = convertToPreviousCoordsSystem(leftVertexRerotated);
    QPointF rightVertexFinal = convertToPreviousCoordsSystem(rightVertedRerotated);
    QPointF topVertexFinal = convertToPreviousCoordsSystem(topVertedRerotated);

    QVector<QPointF> result;
    result << leftVertexFinal << rightVertexFinal << topVertexFinal;

    finalVertices = result;

    m_valid = true;
    return true;
}
