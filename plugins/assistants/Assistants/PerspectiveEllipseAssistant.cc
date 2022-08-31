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

#include <math.h>
#include<QDebug>
#include <QtMath>

#include <functional>


// ################################## Ellipse in Polygon (in Perspective) #######################################

class EllipseInPolygon
{
public:

    EllipseInPolygon();

    // nomenclature:
    // "final ellipse" - ellipse that the user wants
    // "rerotated ellipse" - "final ellipse" that has been rotated (transformed) to have axes parallel to X and Y axes
    //    (called "rerotated" because now the rotation angle is 0)
    // "canonical ellipse" - "final ellipse" that has been rotated to have axes parallel to X and Y axes,
    //    *and* moved so that the center is in point (0, 0)
    // --- every "ellipse" above also means "coordination system for that ellipse"

    // "ellipse formula" - ax^2 + bxy + cy^2 + dx + ey + f = 0
    // "vertices" - points on axes

    // enum


    typedef enum CURVE_TYPE {
        ELLIPSE,
        HYPERBOLA,
        PARABOLA
    } CURVE_TYPE;



    // functions

    ///
    /// \brief updateToPolygon
    /// This function makes all the necessary calculations and updates all data, not just the polygon
    ///  according to the polygon that was provided as the parameter
    /// \param polygon polygon that contains the ellipse
    /// \returns whether the ellipse is valid or not
    ///
    bool updateToPolygon(QVector<QPointF> _polygon, QLineF horizonLine);

    bool updateToPointOnConcentricEllipse(QTransform _originalTransform, QPointF pointOnConcetric, QLineF horizonLine);

    ///
    /// \brief setSimpleEllipseVertices sets vertices of this ellipse to the "simple ellipse" class
    /// to be drawn and used
    /// \param ellipse
    /// \return
    ///
    bool setSimpleEllipseVertices(Ellipse& ellipse) const;

    bool isValid() const { return m_valid; }

    ///
    /// \brief formulaRepresentsAnEllipse
    /// parameters are first three coefficients from a formula: ax^2 + bxy + cy^2 + dx + ey + f = 0
    /// \param a - first coefficient
    /// \param b - second coefficient
    /// \param c - third coefficient
    /// \return true if the formula represents an ellipse, otherwise false
    ///
    static bool formulaRepresentsAnEllipse(double a, double b, double c);

    static CURVE_TYPE curveTypeForFormula(double a, double b, double c);

    void paintParametric(QPainter& gc, const QRectF& updateRect, const QTransform &initialTransform);

    // unused for now; will be used to move the ellipse towards any vanishing point
    // might need more info about vanishing points (for example, might need all points)
    // moveTowards(QPointF vanishingPoint, QPointF cursorStartPoint, QPointF cursorEndPoint);

    // ----- data -----
    // keep the known-size-vectors the same size!

    QVector<QPointF> polygon;
    QTransform originalTransform; // original square-to-polygon transform, including perspective


    QVector<double> finalFormula; // final ellipse formula using ax^2 + bxy + cy^2 + dx + ey + f = 0
    QVector<double> rerotatedFormula; // rerotated ellipse formula using ax^2 + bxy + cy^2 + dx + ey + f = 0

    double finalAxisAngle {0.0}; // theta - angle of the final ellipse's X axis
    double finalAxisReverseAngleCos {0.0}; // cos(-theta) -> used for calculating rerotatedFormula
    double finalAxisReverseAngleSin {0.0}; // sin(-theta) -> used for calculating rerotatedFormula

    QVector<double> finalEllipseCenter; // always just two values; QPointF could have too low of a precision for calculations

    double axisXLength {0.0}; // all "final", "rerotated" and "canonical" ellipses have the same axes lengths
    double axisYLength {0.0};

    QVector<QPointF> finalVertices; // used to draw ellipses and project the cursor points

    double originalCircleRadius {0.5};

    CURVE_TYPE curveType; // whether the formula represents an ellipse, parabola or hyperbola

    QVector<QPointF> originalPoints; // five points used for the ellipse

    QLineF horizon; // needed for painting
    QVector<double> horizonFormula; // needed for painting; ax + by + c = 0 represents the horizon

    int concentricDefiningPointIndex; // index of the point defining concentric ellipse; usually the mouse point




protected:

    void setFormula(QVector<double>& formula, double a, double b, double c, double d, double e, double f);
    void setPoint(QVector<double>& point, double x, double y);

    bool updateToFivePoints(QVector<QPointF> points, QLineF _horizon);

    void paintParametric(QPainter& gc, const QRectF& updateRect, const QTransform &initialTransform, const QPointF& begin, bool goesLeft);

    /**
     * @brief horizonLineSign says on which side of the horizon (using the same, saved formula) the specific point is
     * @param point point to determine on which side of the horizon line it is
     * @return -1 for one side and +1 on the other, if equal for two points, it means that they are on the same side of the line. 0 means on the line.
     */
    int horizonLineSign(QPointF point);

    QPointF mirrorPoint(QPointF point, QLineF horizon);


    bool m_valid {false};

};


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


// ################################## Perspective Ellipse Assistant #######################################


class PerspectiveEllipseAssistant::Private
{
public:
    EllipseInPolygon ellipseInPolygon;
    EllipseInPolygon concentricEllipseInPolygon;

    Ellipse simpleEllipse;
    Ellipse simpleConcentricEllipse;

    bool cacheValid { false };

    bool isConcentric {false};

    PerspectiveBasedAssistantHelper::CacheData cache;

    QVector<QPointF> cachedPoints; // points on the polygon

};

PerspectiveEllipseAssistant::PerspectiveEllipseAssistant(QObject *parent)
    : KisAbstractPerspectiveGrid(parent)
    , KisPaintingAssistant("perspective ellipse", i18n("Perspective Ellipse assistant"))
    , d(new Private())
{

}

PerspectiveEllipseAssistant::~PerspectiveEllipseAssistant() {}

PerspectiveEllipseAssistant::PerspectiveEllipseAssistant(const PerspectiveEllipseAssistant &rhs, QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap)
    : KisAbstractPerspectiveGrid(rhs.parent())
    , KisPaintingAssistant(rhs, handleMap)
    , d(new Private())
{
    d->isConcentric = rhs.isConcentric();
}

KisPaintingAssistantSP PerspectiveEllipseAssistant::clone(QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap) const
{
    return KisPaintingAssistantSP(new PerspectiveEllipseAssistant(*this, handleMap));
}

QPointF PerspectiveEllipseAssistant::project(const QPointF& pt, const QPointF& strokeBegin)
{
    Q_UNUSED(strokeBegin);
    Q_ASSERT(isAssistantComplete());

    if (d->isConcentric) {
        return d->simpleConcentricEllipse.project(pt);
    } else {
        d->ellipseInPolygon.setSimpleEllipseVertices(d->simpleEllipse);
        return d->simpleEllipse.project(pt);
    }
}

QPointF PerspectiveEllipseAssistant::adjustPosition(const QPointF& pt, const QPointF& strokeBegin, const bool /*snapToAny*/, qreal /*moveThresholdPt*/)
{
    return project(pt, strokeBegin);
}

void PerspectiveEllipseAssistant::adjustLine(QPointF &point, QPointF &strokeBegin)
{
    point = QPointF();
    strokeBegin = QPointF();
}

void PerspectiveEllipseAssistant::drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool cached, KisCanvas2* canvas, bool assistantVisible, bool previewVisible)
{
    gc.save();
    gc.resetTransform();

    bool isEditing = false;
    
    QTransform initialTransform = converter->documentToWidgetTransform();

    // need to update ellipse cache
    updateCache();

    QPolygonF poly = d->ellipseInPolygon.polygon;
    QTransform transform = d->ellipseInPolygon.originalTransform; // unused, but computed for caching purposes


    if (isEllipseValid() && assistantVisible==true) {
        // draw vanishing points
        if (d->cache.vanishingPoint1) {
            drawX(gc, initialTransform.map(d->cache.vanishingPoint1.get()));
        }
        if (d->cache.vanishingPoint2) {
            drawX(gc, initialTransform.map(d->cache.vanishingPoint2.get()));
        }
    }
    
    QPointF mousePos = effectiveBrushPosition(converter, canvas);

    if (d->isConcentric && initialTransform.isInvertible()) {
        ENTER_FUNCTION() << "Mouse pos was " << mousePos << "anty-transformed: " << initialTransform.inverted().map(mousePos);

        d->concentricEllipseInPolygon.updateToPointOnConcentricEllipse(d->ellipseInPolygon.originalTransform, initialTransform.inverted().map(mousePos), d->cache.horizon);
        d->concentricEllipseInPolygon.setSimpleEllipseVertices(d->simpleConcentricEllipse);
        ENTER_FUNCTION() << "Set points to simple ellipse:" << d->concentricEllipseInPolygon.finalVertices[0]
                         << d->concentricEllipseInPolygon.finalVertices[1] << d->concentricEllipseInPolygon.finalVertices[2];
        ENTER_FUNCTION() << "Is transform identity? " << d->simpleConcentricEllipse.getTransform().isIdentity();
        ENTER_FUNCTION() << "Transform:" << d->simpleConcentricEllipse.getTransform();
    }

    // draw ellipse and axes
    if (isAssistantComplete() && isEllipseValid() && (assistantVisible || previewVisible || isEditing)) { // ensure that you only draw the ellipse if it's valid - otherwise it would just show some outdated one

        if (!isEditing && d->isConcentric && d->concentricEllipseInPolygon.isValid()) {
            gc.setTransform(initialTransform);
            gc.setTransform(d->simpleConcentricEllipse.getTransform().inverted(), true);

            QPainterPath path2;

            ENTER_FUNCTION() << "conc. ell. axis: " << d->simpleConcentricEllipse.semiMajor() << d->simpleConcentricEllipse.semiMinor()
                             << "normal ones: " << d->simpleEllipse.semiMajor() << d->simpleEllipse.semiMinor();
            ENTER_FUNCTION() << "original radius: " << d->concentricEllipseInPolygon.originalCircleRadius;

            path2.addEllipse(QPointF(0.0, 0.0), d->simpleConcentricEllipse.semiMajor(), d->simpleConcentricEllipse.semiMinor());
            path2.addEllipse(d->simpleConcentricEllipse.getTransform().map(d->concentricEllipseInPolygon.finalVertices[0]), 5, 5);
            path2.addEllipse(d->concentricEllipseInPolygon.finalVertices[1], 5, 5);
            path2.addEllipse(d->concentricEllipseInPolygon.finalVertices[2], 5, 5);

            drawPath(gc, path2, isSnappingActive());

            QPen pen2(QBrush(Qt::blue), 3);
            gc.save();
            gc.setPen(pen2);

            //gc.setTransform(initialTransform);
            gc.setTransform(QTransform());
            gc.drawRect(kisGrowRect(updateRect, -85));

            gc.restore();

            QPen pen(QBrush(Qt::red), 3);
            gc.save();
            gc.setPen(pen);

            d->concentricEllipseInPolygon.paintParametric(gc, updateRect, initialTransform);
            gc.restore();
        }
        if (!isEditing && d->isConcentric) {
            QPen pen2(QBrush(Qt::blue), 3);
            gc.save();
            gc.setPen(pen2);

            //gc.setTransform(initialTransform);
            gc.setTransform(QTransform());
            gc.drawRect(kisGrowRect(updateRect, -85));

            gc.restore();

            QPen pen(QBrush(Qt::red), 3);
            gc.save();
            gc.setPen(pen);

            d->concentricEllipseInPolygon.paintParametric(gc, updateRect, initialTransform);
            gc.restore();
        }



        gc.setTransform(initialTransform);
        gc.setTransform(d->simpleEllipse.getTransform().inverted(), true);

        QPainterPath path;

        path.addEllipse(QPointF(0.0, 0.0), d->simpleEllipse.semiMajor(), d->simpleEllipse.semiMinor());
        drawPath(gc, path, isSnappingActive());

        if (isEditing) {
            QPainterPath axes;
            axes.moveTo(QPointF(-d->simpleEllipse.semiMajor(), 0));
            axes.lineTo(QPointF(d->simpleEllipse.semiMajor(), 0));

            axes.moveTo(QPointF(0, -d->simpleEllipse.semiMinor()));
            axes.lineTo(QPointF(0, d->simpleEllipse.semiMinor()));

            gc.save();

            QPen p(gc.pen());
            p.setCosmetic(true);
            p.setStyle(Qt::DotLine);
            QColor color = effectiveAssistantColor();
            if (!isSnappingActive()) {
                color.setAlpha(color.alpha()*0.2);
            }
            p.setWidthF(1.5);
            p.setColor(color);
            gc.setPen(p);

            gc.drawPath(axes);

            gc.restore();
        }

        gc.setTransform(converter->documentToWidgetTransform());
        gc.setTransform(d->ellipseInPolygon.originalTransform, true);

        // drawing original axes ("lines to touching points")
        QPointF pt1 = QPointF(0.5, 1.0);
        QPointF pt2 = QPointF(1.0, 0.5);
        QPointF pt3 = QPointF(0.5, 0.0);
        QPointF pt4 = QPointF(0.0, 0.5);

        QPainterPath touchingLine;

        touchingLine.moveTo(pt1);
        touchingLine.lineTo(pt3);

        touchingLine.moveTo(pt2);
        touchingLine.lineTo(pt4);
        
        touchingLine.moveTo(pt2);
        touchingLine.lineTo(pt4);

        if (assistantVisible) {
            drawPath(gc, touchingLine, isSnappingActive());
        }

        //gc.drawEllipse(QPointF(0.5, 0.5), d->concentricEllipseInPolygon.originalCircleRadius, d->concentricEllipseInPolygon.originalCircleRadius);
    }


    gc.setTransform(converter->documentToWidgetTransform());

    if (assistantVisible || isEditing) {
        if (!isEllipseValid()) {
            // color red for an invalid transform, but not for an incomplete one
            if(isAssistantComplete()) {
                QPainterPath path;
                QPolygonF polyAllConnected;
                // that will create a triangle with a point inside connected to all vertices of the triangle
                polyAllConnected << *handles()[0] << *handles()[1] << *handles()[2] << *handles()[3] << *handles()[0] << *handles()[2] << *handles()[1] << *handles()[3];
                path.addPolygon(polyAllConnected);
                drawError(gc, path);
            } else {
                QPainterPath path;
                path.addPolygon(poly);
                drawPath(gc, path, isSnappingActive());
            }
        } else {
            gc.setPen(QColor(0, 0, 0, 125));
            gc.setTransform(transform, true);
            QPainterPath path;
            for (int y = 0; y <= 1; ++y)
            {
                QLineF line = QLineF(QPointF(0.0, y), QPointF(1.0, y));
                KisAlgebra2D::cropLineToRect(line, gc.window(), false, false);
                path.moveTo(line.p1());
                path.lineTo(line.p2());
            }
            for (int x = 0; x <= 1; ++x)
            {
                QLineF line = QLineF(QPointF(x, 0.0), QPointF(x, 1.0));
                KisAlgebra2D::cropLineToRect(line, gc.window(), false, false);
                path.moveTo(line.p1());
                path.lineTo(line.p2());
            }

            drawPath(gc, path, isSnappingActive());
        }
    }
    
    gc.restore();
    KisPaintingAssistant::drawAssistant(gc, updateRect, converter, cached, canvas, assistantVisible, previewVisible);

}


void PerspectiveEllipseAssistant::drawCache(QPainter& gc, const KisCoordinatesConverter *converter, bool assistantVisible)
{
    Q_UNUSED(converter);
    Q_UNUSED(gc);
    Q_UNUSED(assistantVisible);
}

QRect PerspectiveEllipseAssistant::boundingRect() const
{
     if (!isAssistantComplete()) {
       return KisPaintingAssistant::boundingRect();
    }

    if (d->ellipseInPolygon.setSimpleEllipseVertices(d->simpleEllipse)) {
       return d->simpleEllipse.boundingRect().adjusted(-2, -2, 2, 2).toAlignedRect();
    } else {
       return QRect();
    }
}

QPointF PerspectiveEllipseAssistant::getDefaultEditorPosition() const
{
    QPointF centroid(0, 0);
    for (int i = 0; i < 4; ++i) {
        centroid += *handles()[i];
    }

    return centroid * 0.25;
}

bool PerspectiveEllipseAssistant::isEllipseValid()
{
    return isAssistantComplete() && d->ellipseInPolygon.isValid();
}

void PerspectiveEllipseAssistant::updateCache()
{
    // handles -> points -> polygon
    d->cacheValid = false;
    // check the cached points, whether they are the same as handles
    if (d->cachedPoints.size() == handles().size()) {
        for (int i = 0; i < handles().size(); ++i) {
            if (d->cachedPoints[i] != *handles()[i]) break;
            if (i == handles().size() - 1) {
                // that means the cache is up to date, because the loop was still going
                d->cacheValid = true;
                return;
            }
        }
    }

    d->cachedPoints = QVector<QPointF>();
    for (int i = 0; i < handles().size(); ++i) {
        d->cachedPoints << *handles()[i];
    }


    QPolygonF poly = QPolygonF(d->cachedPoints);

    if (!PerspectiveBasedAssistantHelper::getTetragon(handles(), isAssistantComplete(), poly)) { // this function changes poly to some "standarized" version, or a triangle when it cannot be achieved

        poly = QPolygonF(d->cachedPoints);
        poly << d->cachedPoints[0];

        PerspectiveBasedAssistantHelper::updateCacheData(d->cache, poly);

        d->ellipseInPolygon.updateToPolygon(poly, d->cache.horizon);
        d->cacheValid = true;
        return;
    }

    PerspectiveBasedAssistantHelper::updateCacheData(d->cache, poly);

    d->ellipseInPolygon.updateToPolygon(poly, d->cache.horizon);
    if (d->ellipseInPolygon.isValid()) {
        d->ellipseInPolygon.setSimpleEllipseVertices(d->simpleEllipse);
    }


    d->cacheValid = true;

}

bool PerspectiveEllipseAssistant::isAssistantComplete() const
{   
    return handles().size() >= 4;
}

void PerspectiveEllipseAssistant::saveCustomXml(QXmlStreamWriter *xml)
{
    xml->writeStartElement("isConcentric");
    ENTER_FUNCTION() << ppVar(this->isConcentric()) << ppVar((int)this->isConcentric()) << ppVar(KisDomUtils::toString( (int)this->isConcentric()));
    xml->writeAttribute("value", KisDomUtils::toString( (int)this->isConcentric()));
    xml->writeEndElement();
}

bool PerspectiveEllipseAssistant::loadCustomXml(QXmlStreamReader *xml)
{
    if (xml && xml->name() == "isConcentric") {
        this->setConcentric((bool)KisDomUtils::toInt(xml->attributes().value("value").toString()));
        ENTER_FUNCTION() << ppVar(xml->attributes().value("value").toString())
                         << ppVar(KisDomUtils::toInt(xml->attributes().value("value").toString()))
                         << ppVar((bool)KisDomUtils::toInt(xml->attributes().value("value").toString()));
        ENTER_FUNCTION() << "Therefore, the assistant is now: " << ppVar(isConcentric());
    }
    return true;
}

bool PerspectiveEllipseAssistant::isConcentric() const
{
    return d->isConcentric;
}

void PerspectiveEllipseAssistant::setConcentric(bool isConcentric)
{
    d->isConcentric = isConcentric;
}

bool PerspectiveEllipseAssistant::contains(const QPointF &point) const
{

    QPolygonF poly;
    if (!PerspectiveBasedAssistantHelper::getTetragon(handles(), isAssistantComplete(), poly)) return false;
    return poly.containsPoint(point, Qt::OddEvenFill);
}

qreal PerspectiveEllipseAssistant::distance(const QPointF &point) const
{
    return PerspectiveBasedAssistantHelper::distanceInGrid(d->cache, point);
}

bool PerspectiveEllipseAssistant::isActive() const
{
    return isSnappingActive();
}

PerspectiveEllipseAssistantFactory::PerspectiveEllipseAssistantFactory()
{
}

PerspectiveEllipseAssistantFactory::~PerspectiveEllipseAssistantFactory()
{
}

QString PerspectiveEllipseAssistantFactory::id() const
{
    return "perspective ellipse";
}

QString PerspectiveEllipseAssistantFactory::name() const
{
    return i18n("Perspective Ellipse");
}

KisPaintingAssistant* PerspectiveEllipseAssistantFactory::createPaintingAssistant() const
{
    return new PerspectiveEllipseAssistant;
}

