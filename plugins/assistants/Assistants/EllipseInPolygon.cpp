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


#include <Eigen/Eigenvalues>
#include <functional>

#include <kis_canvas2.h>
#include <kis_coordinates_converter.h>
#include "kis_algebra_2d.h"
#include <kis_dom_utils.h>
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
    finalFormula << 1 << 0 << 1 << 0 << 0 << -1;
    rerotatedFormula << 1 << 0 << 1 << 0 << 0 << -1;

    finalEllipseCenter.clear();
    finalEllipseCenter << 0 << 0;

    finalVertices.clear();
    finalVertices << QPointF(-1, 0) << QPointF(1, 0) << QPointF(0, 1);
}

bool EllipseInPolygon::updateToPolygon(QVector<QPointF> _polygon, QLineF horizonLine)
{
    QTransform transform;

    m_valid = false; // let's make it false in case we return in the middle of the work

    ENTER_FUNCTION() << ppVar(_polygon);

    polygon = _polygon; // the assistant needs to know the polygon even when it doesn't allow for a correct ellipse


    // this calculates the perspective transform that represents the current quad (polygon)
    // this is the transform that changes the original (0, 0, 1, 1) square to the quad
    // that means that our "original" ellipse is the circle in that square (with center in (0.5, 0.5), and radius 0.5)
    ENTER_FUNCTION() << ppVar(polygon);
    if (!QTransform::squareToQuad(polygon, transform)) {
        ENTER_FUNCTION() << "SQUARE TO QUAD FAILED";
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


    ENTER_FUNCTION() << ppVar(pt1) << pt2 << pt3 << pt4 << ptR;


    return updateToFivePoints({pt1, pt2, pt3, pt4, ptR}, horizonLine);


}

bool EllipseInPolygon::updateToPointOnConcentricEllipse(QTransform _originalTransform, QPointF pointOnConcetric, QLineF horizonLine, bool mirrored)
{
    m_valid = false; // let's make it false in case we return in the middle of the work
    m_mirrored = mirrored;
    if (!_originalTransform.isInvertible()) {
        return false;
    }


    if (mirrored) {
        // let's mirror the transformation!
        qreal l = horizonLine.length();
        l = 1/(l*l);
        qreal lx = horizonLine.dx();
        qreal ly = horizonLine.dy();

        qreal a = l*(lx*lx - ly*ly);
        qreal b = 2*l*lx*ly;

        //qCritical() << "### Istotne, horizon was: " <<
        //qCritical() << "### Before the mirroring, the (0, 0.5) point would end up: " << _originalTransform.map(QPointF(0, 0.5));

        QTransform transpose = QTransform::fromTranslate(horizonLine.p1().x(), horizonLine.p1().y());
        QTransform justMirror = QTransform(a, b, 0, b, -a, 0, 0, 0, 1);
        QTransform toMultiply = transpose.inverted()*justMirror*transpose;
        //qCritical() << "### The transform to multiply would transform (200, 100) this way: " << toMultiply.map(QPointF(200, 100));
        //qCritical() << "### The transform to multiply would transform result of the previous transf. this way: " << toMultiply.map(_originalTransform.map(QPointF(0, 0.5)));

        //qCritical() << "### The transpose is: " << transpose;
        //qCritical() << "### The justMirror is: " << justMirror;
        //qCritical() << "### The full matrix is: " << toMultiply;


        _originalTransform = _originalTransform*toMultiply;
        //qCritical() << "### After the mirroring, the (0, 0.5) point would end up: " << _originalTransform.map(QPointF(0, 0.5));

    }

    // first check on which side of the horizon line the original ellipse is
    QPointF topPoint = QPointF(0.5, 1.0);
    QPointF topPointInFinalCoordinates = _originalTransform.map(topPoint);
    auto whichSideOnLine = [] (QLineF line, QPointF point) {
        return KisAlgebra2D::signZZ((line.p2().x() - line.p1().x()) * (point.y() - line.p1().y()) - (line.p2().y() - line.p1().y()) * (point.x() - line.p1().x()));
    };

    //int side1 = whichSideOnLine(horizonLine, pointOnConcetric);
    //int side2 = whichSideOnLine(horizonLine, topPointInFinalCoordinates);


    QTransform inverted = _originalTransform.inverted();

    QPointF pointInOriginalCoordinates = inverted.map(pointOnConcetric);

    QPointF center = QPointF(0.5, 0.5);
    double distanceFromCenter = kisDistance(pointInOriginalCoordinates, center);

    originalCircleRadius = distanceFromCenter;
    //qCritical() << "distance from center" << distanceFromCenter;

    originalTransform = _originalTransform;

    QVector<QPointF> points;
    if (mirrored) {
        points << originalTransform.map(pointInOriginalCoordinates)
               << originalTransform.map(center + QPointF(distanceFromCenter, 0))
               << originalTransform.map(center + QPointF(-distanceFromCenter, 0))
               << originalTransform.map(center + QPointF(0, distanceFromCenter))
               << originalTransform.map(center + QPointF(0, -distanceFromCenter));
    } else {
        points << originalTransform.map(pointInOriginalCoordinates)
               << originalTransform.map(center + QPointF(distanceFromCenter, 0))
               << originalTransform.map(center + QPointF(-distanceFromCenter, 0))
               << originalTransform.map(center + QPointF(0, distanceFromCenter))
               << originalTransform.map(center + QPointF(0, -distanceFromCenter));
    }

    concentricDefiningPointIndex = 0;

    //ENTER_FUNCTION() << "Points are: " << pointInOriginalCoordinates
    //                 << center + QPointF(distanceFromCenter, 0) << center + QPointF(-distanceFromCenter, 0);

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

QPointF EllipseInPolygon::projectModifiedEberlySecond(QPointF point)
{
    QPointF originalPoint = point;
    // method taken from https://www.sciencedirect.com/science/article/pii/S0377042713001398
    // "Algorithms for projecting points onto conics", authors: N.Chernova, S.Wijewickrema
    //

    // Stage 1. Canonize the formula.

    // "canonnical formula" is Ax^2 + 2Bxy + Cy^2 + 2Dx + 2Ey + F = 0
    // and A^2 + B^2 + C^2 + D^2 + E^2 + F^2 = 1
    // so we gotta change it first

    // NAMING
    // A - true formula, "final formula"
    // B - A, but normalized (so that A^2 + B^2 + C^2... = 1)
    // C - B, but canonized (convert from b to B=b/2 to be able to use Elberly correctly)
    // D -
    // E -
    // F -
    // G -
    // H -
    // I -

    // ***************

    bool debug = false;
    //debug = false;
    //debug = true;

    // *************

    struct Formulas {
        ConicFormula formA;
        ConicFormula formBNormalized;
        ConicFormula formCCanonized;

        ConicFormula formDRotated;
        ConicFormula formDRotatedSecondVersion;

        ConicFormula formEMovedToOrigin;
        ConicFormula formFSwappedXY;
        ConicFormula formGNegatedAllSigns;
        ConicFormula formHNegatedX;
        ConicFormula formINegatedY;

        ConicFormula current;
    };

    Formulas f;



    auto writeFormulaInWolframAlphaForm = [] (QVector<double> formula, bool trueForm = false) {
        auto writeOutNumber = [] (double n, bool first = false) {
            QString str;
            if (n >= 0 && !first) {
                str = QString("+ ") + QString::number(n);
            } else {
                str = (QString::number(n));
            }
            return str;
        };

        QString str = writeOutNumber(formula[0], true) + "x^2 "
                + writeOutNumber(trueForm? formula[1] : 2*formula[1]) + "xy "
                + writeOutNumber(formula[2]) + "y^2 "
                + writeOutNumber(trueForm? formula[3] : 2*formula[3]) + "x "
                + writeOutNumber(trueForm? formula[4] : 2*formula[4]) + "y "
                + writeOutNumber(formula[5]) + " = 0";
        return str;
    };


    auto writeFormulaInAllForms = [writeFormulaInWolframAlphaForm] (QVector<double> formula, QString name, bool trueForm = false) {
        QString octaveForm = "[";
        for (int i = 0; i < 6; i++) {
            bool shouldDouble = formula[i] && (i == 1 || i == 3 || i == 4);
            octaveForm += QString::number(shouldDouble ? 2*formula[i] : formula[i]);
            if (i != 5) {
                octaveForm += ",";
            }
        }
        octaveForm += "];";

        /*
        qCritical() << "ModifiedElberly +Sec+ | --- (QVector, not Conic) ";
        qCritical() << "ModifiedElberly       | formula name: " << name;
        qCritical() << "ModifiedElberly       | true form? " << trueForm;
        qCritical() << "ModifiedElberly       | WolframAlpha: " << writeFormulaInWolframAlphaForm(formula, trueForm);
        qCritical() << "ModifiedElberly       | Octave: " << octaveForm;
        qCritical() << "ModifiedElberly       | Actual data (A, B, C, D...): " << formula;
        qCritical() << "ModifiedElberly       | ---";
        */
    };


    auto stdDebug = [debug] (ConicFormula f) {
        if (debug) f.printOutInAllForms();
    };

    auto stdDebugVec = [debug, writeFormulaInAllForms] (QVector<double> f, QString name, bool trueForm = false) {
        if (debug) writeFormulaInAllForms(f, name, trueForm);
    };


    if (debug) ENTER_FUNCTION() << "################ ELBERLY SECOND ###################";
    if (debug) ENTER_FUNCTION() << ppVar(polygon) << ppVar(point);



    QVector<double> canonized;
    canonized = QVector<double>::fromList(finalFormula.toList());

    QVector<double> trueFormulaA = QVector<double>::fromList(finalFormula.toList());
    writeFormulaInAllForms(trueFormulaA, "formula A - final", true);
    ConicFormula formA(trueFormulaA, "formula A - final", ConicFormula::ACTUAL);
    if (debug) formA.printOutInAllForms();
    f.formA = ConicFormula(trueFormulaA, "formula A - final", ConicFormula::ACTUAL);
    f.current = f.formA;

    //if (debug)

    if (debug) ENTER_FUNCTION() << ppVar(finalFormula) << ppVar(finalFormula.toList());


    // I guess it should also rescale the ellipse for the points to be within 0-1?... uhhhhh...
    // TODO: make sure it's correct and finished
    // ACTUALLY no, it's the 'point' that should be normalized here, not the ellipse!

    double normalizeBy = 1.0;
    QPointF point3 = point;


    normalizeBy = qMax(point.x(), point.y());

    if (debug) ENTER_FUNCTION() << "NORMALIZATION VALUE HERE: " << ppVar(normalizeBy);
    if (debug) ENTER_FUNCTION() << ppVar(canonized) << ppVar(writeFormulaInWolframAlphaForm(canonized, true)) << ppVar(writeFormulaInWolframAlphaForm(finalFormula, true));
    // załóżmy x+y = 1 oraz nb = 5, bo mielismy x,y = 5,5 -> czyli robimy wszystko mniejsze
    // x => nb*x
    // (nb*x) + (nb*y) = 1
    // 5x + 5y = 1


    if (!qFuzzyCompare(normalizeBy, 0)) {
        point = point/normalizeBy;
    }



    auto normalizeFormula = [debug] (ConicFormula formula, QPointF& point, double normalizeBy) {

        normalizeBy = qMax(point.x(), point.y());
        if (debug) ENTER_FUNCTION() << ppVar(normalizeBy);
        if (debug) ENTER_FUNCTION() << ppVar(formula.getFormulaActual()) << ppVar(formula.toWolframAlphaForm());
        // załóżmy x+y = 1 oraz nb = 5, bo mielismy x,y = 5,5 -> czyli robimy wszystko mniejsze
        // x => nb*x
        // (nb*x) + (nb*y) = 1
        // 5x + 5y = 1

        ConicFormula normalized = formula;
        normalized.Name = "form B - normalized (CF)";

        // ok so,
        if (!qFuzzyCompare(normalizeBy, 0)) {
            normalized.A = normalizeBy*normalizeBy*formula.A; // x^2
            normalized.B = normalizeBy*normalizeBy*formula.B; // x*y
            normalized.C = normalizeBy*normalizeBy*formula.C; // y^2
            normalized.D = normalizeBy*formula.D; // x
            normalized.E = normalizeBy*formula.E; // y
            normalized.F = formula.F;

            point = point/normalizeBy;
        }

        if (debug) ENTER_FUNCTION() << ppVar(normalized.toWolframAlphaForm());

        return normalized;

    };


    f.formBNormalized = normalizeFormula(f.formA, point3, normalizeBy);

    if (debug) ENTER_FUNCTION() << "(*) after normalization:" << ppVar(point);

    if (debug) ENTER_FUNCTION() << "after normalization:" << ppVar(canonized) << ppVar(point);
    if (debug) ENTER_FUNCTION() << ppVar(writeFormulaInWolframAlphaForm(canonized, true));

    // Ax^2 + 2Bxy + Cy^2 = 0;
    // a' ^2 + 2b' xy + c' y^2 = 0;

    // 5x^2 + 8xy + 3y^2 = 0
    // 5x^2 + (2*4)xy + 3y^2 = 0
    // 5^2 + 4^2 + 3^2 = 25 + 16 + 9 = k^2 = 50 = 25*2 => 5*sqrt(2) = k
    // A = 5/(5sqrt(2)) = sqrt(2)/2
    // B = (b/2)/(k)


    auto canonizeFormula = [debug] (Formulas& f) {

        auto sq = [] (qreal a) {return a*a;};

        double canonizingNumber = sq(f.formBNormalized.A) + sq(f.formBNormalized.B/2) + sq(f.formBNormalized.C)
                + sq(f.formBNormalized.D/2) + sq(f.formBNormalized.E/2) + sq(f.formBNormalized.F);
        canonizingNumber = sqrt(canonizingNumber);


        if (debug) ENTER_FUNCTION() << ppVar(canonizingNumber);

        //if (debug) writeFormulaInAllForms(canonized, "canonized raw - before canonization");
        if (debug) f.formBNormalized.printOutInAllForms();


        // conversion actual -> special is included below
        f.formCCanonized = ConicFormula(f.formBNormalized.getFormulaActual(), "formula C - canonized", ConicFormula::SPECIAL);

        f.formCCanonized.A = f.formBNormalized.A/canonizingNumber;
        f.formCCanonized.B = f.formBNormalized.B/(2*canonizingNumber);
        f.formCCanonized.C = f.formBNormalized.C/canonizingNumber;
        f.formCCanonized.D = f.formBNormalized.D/(2*canonizingNumber);
        f.formCCanonized.E = f.formBNormalized.E/(2*canonizingNumber);
        f.formCCanonized.F = f.formBNormalized.F/canonizingNumber;

        //if (debug) ENTER_FUNCTION() << "Are those two values the same?? ___________ " << ppVar(canonized[0] - f.formCCanonized.A);
        //if (debug) writeFormulaInAllForms(canonized, "canonized raw");
        if (debug) f.formCCanonized.printOutInAllForms();

        qreal checkIfReallyCanonized = sq(f.formCCanonized.A) + sq(f.formCCanonized.B) + sq(f.formCCanonized.C)
                + sq(f.formCCanonized.D) + sq(f.formCCanonized.E) + sq(f.formCCanonized.F);

        if (debug) ENTER_FUNCTION() << ppVar(checkIfReallyCanonized == 1);

    };


    canonizeFormula(f);

    // ok so canonized is the article's version, as in, the true equation is with 2*canonized[1] for example,
    // and canonized = A, B, C, D...


    // TODO

    // now it should be canonized correctly

    // Stage 2. Rotate the coordinates system to make B = 0.
    // We need to decomposite the matrix:
    // | A B |
    // | B C | = Q*D*Q^T
    // Q - orthogonal, D - diagonal
    //


    Eigen::Matrix<long double, 2, 2> Q;

    auto rotateLikeArticleSays = [debug] (Formulas& f, Eigen::Matrix<long double, 2, 2>& Q) mutable {
        Eigen::Matrix<long double, 2, 2> M;
        M << f.formCCanonized.A, f.formCCanonized.B,
             f.formCCanonized.B, f.formCCanonized.C;
        if (debug) ENTER_FUNCTION() << "(1)";

        Eigen::EigenSolver<Eigen::Matrix<long double, 2, 2>> eigenSolver(M);
        if (debug) ENTER_FUNCTION() << "(2)";

        Eigen::Matrix<std::complex<long double>, 2, 2> QC = eigenSolver.eigenvectors();
        Eigen::Matrix<std::complex<long double>, 2, 2> DC;
        if (debug) ENTER_FUNCTION() << "(3)a";
        DC(0, 0) = eigenSolver.eigenvalues()(0);
        DC(1, 1) = eigenSolver.eigenvalues()(1);
        if (debug) ENTER_FUNCTION() << "(3)";

        Eigen::Matrix<std::complex<long double>, 2, 2> cRM = QC*DC*QC.transpose();
        if (debug) ENTER_FUNCTION() << "Check correctness of the eigen decomposition: ";
        if (debug) {
            for (int i = 0; i < 2; i++)  {
                for (int j = 0; j < 2; j++) {
                    ENTER_FUNCTION() << ppVar(i) << ppVar(j) << "|" << ppVar((double)M(i, j)) << ppVar((double)std::real(cRM(i, j)))
                                     << ppVar((bool)(M(i, j) == std::real(cRM(i, j)))) << ppVar((double)(qAbs(M(i, j) - std::real(cRM(i, j)))));
                }
            }
        }

        Eigen::Matrix<long double, 2, 2> Dm;

        if (debug) ENTER_FUNCTION() << "(4)";

        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                if (std::imag(DC(i, j)) == 0 && std::imag(QC(i, j)) == 0) {
                    Dm(i, j) = std::real(DC(i, j));
                    Q(i, j) = std::real(QC(i, j));
                } else { // if any of that is complex, then something went wrong
                    //return originalPoint;
                }
            }
        }

        if (debug) ENTER_FUNCTION() << "D = " << (double)Dm(0, 0) << (double)Dm(0, 1) << (double)Dm(1, 0) << (double)Dm(1, 1);
        if (debug) ENTER_FUNCTION() << "Q = " << (double)Q(0, 0) << (double)Q(0, 1) << (double)Q(1, 0) << (double)Q(1, 1);


        if (debug) ENTER_FUNCTION() << "(5)";



        auto vectorMultipliedByMatrix = [debug] (QVector<double> vector2, Eigen::Matrix<long double, 2, 2> matrix) {
            QVector<double> res;
            res << vector2[0]*matrix(0, 0) + vector2[1]*matrix(0, 1);
            res << vector2[0]*matrix(1, 0) + vector2[1]*matrix(1, 1);
            return res;
        };


        f.formDRotated.A = Dm(0, 0); // A
        f.formDRotated.B = 0; // D(1, 0); // B
        f.formDRotated.C = Dm(1, 1); // C

        QVector<double> DEvec;
        DEvec << f.formCCanonized.D << f.formCCanonized.E;
        DEvec = vectorMultipliedByMatrix(DEvec, Q);

        f.formDRotated.D = DEvec[0]; // D
        f.formDRotated.E = DEvec[1]; // E
        f.formDRotated.F = f.formCCanonized.F; // F

        f.formDRotated.Name = "form D - rotated (original article version) (cf)";
        f.formDRotated.Type = ConicFormula::SPECIAL;
    };


    rotateLikeArticleSays(f, Q);

    if (debug) ENTER_FUNCTION() << "Q = " << (double)Q(0, 0) << (double)Q(0, 1) << (double)Q(1, 0) << (double)Q(1, 1);

    QPointF pointRotatedSecondVersion = point;
    qreal K;
    qreal L;
    QVector<double> formulaDRotatedAlternativeTrue = getRotatedFormula(f.formCCanonized.getFormulaActual(), K, L);
    pointRotatedSecondVersion = getRotatedPoint(pointRotatedSecondVersion, K, L);
    f.formDRotatedSecondVersion = ConicFormula(formulaDRotatedAlternativeTrue, "formula D - rotated, alternative", ConicFormula::ACTUAL);
    f.formDRotatedSecondVersion.convertTo(ConicFormula::SPECIAL);

    if (debug) f.formDRotatedSecondVersion.printOutInAllForms();

    KIS_ASSERT_RECOVER_NOOP(qAbs(f.formDRotatedSecondVersion.B) < 1e-10);

    bool swap = true;
    if (swap) {
        ConicFormula tmp = f.formDRotated;
        f.formDRotated = f.formDRotatedSecondVersion;
        f.formDRotatedSecondVersion = tmp;
    }


    //if (debug) ENTER_FUNCTION() << "after rotation:" << ppVar(writeFormulaInWolframAlphaForm(rotated)); // still a circle, good

    if (debug) ENTER_FUNCTION() << "(6a)";

    // we gotta move the point as well:
    //point = QPointF(point.x()*Q(0, 0), point.y()*Q(1, 1));
    // point * Q?
    point = QPointF(Q(0, 0)*point.x() + Q(1, 0)*point.y(), Q(0, 1)*point.x() + Q(1, 1)*point.y());
    if (swap) {
        point = pointRotatedSecondVersion;
    }


    if (debug) ENTER_FUNCTION() << "(*) after rotation:" << ppVar(point);

    if (debug) ENTER_FUNCTION() << "(6)" << ppVar(point);

    // Stage 3. Moving point to the origin (x -> x - u, y -> y - v)
    qreal u = point.x();
    qreal v = point.y();

    ENTER_FUNCTION() << "SECOND: move using u, v: " << u << v << point;

    if (debug) ENTER_FUNCTION() << "(*) after moving:" << ppVar(QPointF(0, 0));

    auto moveEllipseSoPointIsInOrigin = [debug] (ConicFormula formula, qreal u, qreal v) {
        ConicFormula response = formula;
        if (debug) ENTER_FUNCTION() << "formula to copy from: " << formula.getFormulaSpecial();
        if (debug) ENTER_FUNCTION() << "formula copied: " << response.getFormulaSpecial();


        // note that F depends on old D and E
        response.D = formula.D + formula.A*u; // D -> D + A*u
        response.E = formula.E + formula.C*v; // E -> E + C*v
        response.F = formula.F + formula.A*u*u + 2*formula.D*u + formula.C*v*v + 2*formula.E*v;// F -> F + Au^2 + 2*Du + Cv^2 + 2Ev

        return response;
    };

    ConicFormula rotatedAfterMoving = moveEllipseSoPointIsInOrigin(f.formDRotated, u, v);

    if (debug) rotatedAfterMoving.printOutInAllForms();

    // Stage 4. Adjusting the coefficients to our needs (C, D, E >= 0)

    f.formEMovedToOrigin = rotatedAfterMoving;
    f.formEMovedToOrigin.Name = "form E - moved to origin (cf)";


    auto adjustCoefficients = [] (Formulas& f, bool& swapXandY, bool& negateX, bool& negateY) {

        swapXandY = false;
        negateX = false;
        negateY = false;

        ConicFormula r = f.formEMovedToOrigin;
        ConicFormula ff = f.formEMovedToOrigin;
        if (qAbs(r.C) < qAbs(r.A)) {
            swapXandY = true;
            r.A = ff.C;
            r.B = ff.B;
            r.C = ff.A;
            r.D = ff.E;
            r.E = ff.D;
            r.F = ff.F;
        }

        f.formFSwappedXY = r;
        f.formFSwappedXY.Name = "form F - swapped X and Y (cf)";

        if (r.C < 0) {
            // negate all signs
            r.A = -r.A;
            r.B = -r.B;
            r.C = -r.C;
            r.D = -r.D;
            r.E = -r.E;
            r.F = -r.F;
        }

        f.formGNegatedAllSigns = r;
        f.formGNegatedAllSigns.Name = "form G - negated all signs (cf)";


        if (r.D < 0) {
            negateX = true;
            r.D = -r.D;
            r.B = -r.B;
        }

        f.formHNegatedX = r;
        f.formHNegatedX.Name = "form H - negated X (cf)";


        if (r.E < 0) {
            negateY = true;
            r.E = -r.E;
            r.B = -r.B;
        }

        f.formINegatedY = r;
        f.formINegatedY.Name = "form H - negated X (cf)";


    };

    bool swapXandY = false;
    bool negateX = false;
    bool negateY = false;


    adjustCoefficients(f, swapXandY, negateX, negateY);
    if (qAbs(f.formINegatedY.C) < 1e-12) {
        ENTER_FUNCTION() << "Weird formula: " << "C should not be 0";
        writeFormulaInAllForms(f.formINegatedY.getData(), "form I negated Y");
        return originalPoint;
    }


    // ok, first we gotta find C being bigger than A (in absolute).

    // ok, now |C| >= |A|

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(f.formINegatedY.C >= 0 && f.formINegatedY.D >= 0 && f.formINegatedY.E >= 0, originalPoint);

    // Stage 5.
    // Thanks to math magic, we've got:
    // x = -Dt/(At + 1)
    // y = -Et/(Ct + 1)
    // hence, the function is:
    // P(t) = -D^2 *t*((At + 2)/(At + 1)^2) - E^2 * t * (Ct+2)/((Ct+1)^2) + F

    auto Pt = [f] (double t) {
        ConicFormula ff = f.formINegatedY;
        return -ff.D*ff.D*t*((ff.A*t + 2)/((ff.A*t + 1)*(ff.A*t + 1))) - ff.E*ff.E * t * (ff.C*t+2)/((ff.C*t+1)*(ff.C*t+1)) + ff.F;
    };

    auto Ptd = [f] (double t) {
        ConicFormula ff = f.formINegatedY;
        return - (2*ff.D*ff.D)/(qPow(ff.A*t*t + 1, 3)) - (2*ff.E*ff.E)/qPow(ff.C*t*t + 1, 3);
    };


    if ((qFuzzyCompare(f.formINegatedY.D, 0) || qFuzzyCompare(f.formINegatedY.E, 0))) {
        // special case
        if (debug) ENTER_FUNCTION() << "Ellipse: and SPECIAL CASE";
    }

    auto getStartingPoint = [Pt, Ptd, debug] (ConicFormula formula) {
        qreal A = formula.A;
        qreal C = formula.C;
        qreal D = formula.D;
        qreal E = formula.E;
        qreal F = formula.F;


        qreal t0 = 0; // starting point for newton method

        // will be useful for both cases
        qreal t0d = F/(D*D - A*F + D*sqrt(D*D - A*F));
        qreal t0dd = F/(E*E - C*F + E*sqrt(E*E - C*F));

        if (A >= 0) { // ellipses and parabolas
            // we got to find t0 where P(t0) >= 0
            if (F < 0) {
                t0 = qMax(t0d, t0dd);
            } // else t0 = 0
        } else { // hyperbolas

            qreal U = sqrt(sqrt(-A*D*D));
            qreal V = sqrt(sqrt(C*E*E));
            qreal tstar = - (U - V)/(C*U - A*V); // inflection point of Pt

            qreal Pstar = Pt(tstar);
            if (debug) ENTER_FUNCTION() << ppVar(Pstar);

            if (Pstar > 0) { // case (a)
                if (F <= 0) {
                    t0 = 0;
                } else {
                    t0 = qMin(t0d, t0dd);
                }
            } else if (Pstar < 0) {
                if (F >= 0) {
                    t0 = 0;
                } else {
                    t0 = qMax(t0d, t0dd);
                }
            } else {
                // we found it already!!!
                t0 = tstar;
            }
        }
        return t0;
    };

    qreal t0 = getStartingPoint(f.formINegatedY); // starting point for newton method

    // now we gotta find t, then when we find it, calculate x and y, undo all transformations and return the point!

    qreal previousT;
    bool overshoot;
    qreal tresult = AlgebraFunctions::newtonUntilOvershoot(Pt, Ptd, t0, previousT, overshoot, debug);
    if (overshoot) {
        if (debug) ENTER_FUNCTION() << "# NEWTON OVERSHOT # t: " << ppVar(tresult) << ppVar(previousT) << ppVar(Pt(tresult)) << ppVar(Pt(previousT));
        tresult = AlgebraFunctions::bisectionMethod(Pt, qMin(tresult, previousT), qMax(tresult, previousT), debug);
        if (debug) ENTER_FUNCTION() << "# AFTER BISECTION # Now it's: " << ppVar(tresult);
    } else {
        if (debug) ENTER_FUNCTION() << "# NO OVERSHOOT # values are: " << ppVar(tresult) << ppVar(previousT) << ppVar(Pt(tresult)) << ppVar(Pt(previousT));
    }


    qreal foundX = -f.formINegatedY.D*tresult/qPow(f.formINegatedY.A*tresult + 1, 1);
    qreal foundY = -f.formINegatedY.E*tresult/qPow(f.formINegatedY.C*tresult + 1, 1);



    qreal checkIfOnTheEllipse = f.formINegatedY.calculateFormulaForPoint(QPointF(foundX, foundY));

    if (debug) ENTER_FUNCTION() << ppVar(checkIfOnTheEllipse);
    if (qAbs(checkIfOnTheEllipse) >= 1e-6) {
        if (debug) ENTER_FUNCTION() << "#### WARNING! NOT ON THE ELLIPSE AFTER BISECTION!!! ####";
    }

    if (debug) ENTER_FUNCTION() << "(1) original" << ppVar(foundX) << ppVar(foundY) << ppVar(calculateFormula(QPointF(foundX, foundY)))
                                << ppVar(f.formINegatedY.calculateFormulaForPoint(QPointF(foundX, foundY)));
    if (debug) ENTER_FUNCTION() << "(1) reminder: " << ppVar(QPointF(foundX, foundY));
    if (debug) f.formINegatedY.printOutInAllForms();

    auto undoAllChanges = [] (QPointF p, bool negateX, bool negateY, bool swapXandY, qreal u, qreal v) {

        qreal foundX = p.x();
        qreal foundY = p.y();

        // ok, now undoing all the changes we did...
        if (negateX) {
            foundX = -foundX;
        }
        if (negateY) {
            foundY = -foundY;
        }

        QPointF result = QPointF(foundX, foundY);
        if (swapXandY){
            result = QPointF(foundY, foundX);
        }

        result = result + QPointF(u, v);

        return result;


    };

    QPointF result = undoAllChanges(QPointF(foundX, foundY), negateX, negateY, swapXandY, u, v);

    if (debug) ENTER_FUNCTION() << "(4) after moving to the origin" << ppVar(result) << ppVar(calculateFormula(result))
                                   << ppVar(f.formDRotated.calculateFormulaForPoint(result));

    if (debug) ENTER_FUNCTION() << "before the mistake" << ppVar(result) << ppVar(writeFormulaInWolframAlphaForm(f.formDRotated.getFormulaSpecial()))
                                << ppVar((double)Q(0, 0)) << ppVar((double)Q(1, 1));

    // un-rotating (with Q)

    // result = QPointF(result.x()/Q(0, 0), result.y()/Q(1, 1));
    // Q must be transposed now
    if (debug) ENTER_FUNCTION() << "Ok, now undoing the rotation for the point:";

    if (swap) {
        result = getRotatedPoint(result, K, L, true);
    } else {
        result = QPointF(Q(0, 0)*result.x() + Q(0, 1)*result.y(), Q(1, 0)*result.x() + Q(1, 1)*result.y());
    }


    if (debug) ENTER_FUNCTION() << "(5) after un-rotating" << ppVar(result) << ppVar(calculateFormula(result)) << ppVar(calculateFormula(canonized, result))
                                   << ppVar(f.formCCanonized.calculateFormulaForPoint(result))
                                   << ppVar(f.formBNormalized.calculateFormulaForPoint(result));


    // uncanonizing ????


    // now un-normalizing
    if (debug) ENTER_FUNCTION() << "NORMALIZATION VALUE: " << ppVar(normalizeBy);

    if (!qFuzzyCompare(normalizeBy, 0)) {
        if (debug) ENTER_FUNCTION() << "NORMALIZATION being done: " << ppVar(normalizeBy);
        result = result*normalizeBy;
    }
    if (debug) ENTER_FUNCTION() << "(6) after un-normalizing" << ppVar(result) << ppVar(calculateFormula(result)) << ppVar(calculateFormula(trueFormulaA, result));

    qreal eps = 0.0001;
    if (qAbs(calculateFormula(result)) >= eps) {
        ENTER_FUNCTION() << ppVar(qAbs(calculateFormula(result))) << ppVar(qAbs(calculateFormula(result)) < eps);
        ENTER_FUNCTION() << "The values were: " << ppVar(polygon) << ppVar(originalPoint) << "and unfortunate result:" << ppVar(result);
    }
    //KIS_ASSERT_RECOVER_RETURN_VALUE(qAbs(calculateFormula(result)) < eps, result);

    if (debug || true) {
        qCritical() << "**** SECOND ****";
        f.formA.printOutInAllForms();
        f.formBNormalized.printOutInAllForms();
        f.formCCanonized.printOutInAllForms();
        f.formDRotated.printOutInAllForms();
        f.formDRotatedSecondVersion.printOutInAllForms();
        f.formEMovedToOrigin.printOutInAllForms();
        f.formFSwappedXY.printOutInAllForms();
        f.formGNegatedAllSigns.printOutInAllForms();
        f.formHNegatedX.printOutInAllForms();
        f.formINegatedY.printOutInAllForms();
    }


    return result;

}

QPointF EllipseInPolygon::projectModifiedEberlyThird(QPointF point)
{
    QPointF originalPoint = point;
    // method taken from https://www.sciencedirect.com/science/article/pii/S0377042713001398
    // "Algorithms for projecting points onto conics", authors: N.Chernova, S.Wijewickrema
    //

    // Stage 1. Canonize the formula.

    // "canonnical formula" is Ax^2 + 2Bxy + Cy^2 + 2Dx + 2Ey + F = 0
    // and A^2 + B^2 + C^2 + D^2 + E^2 + F^2 = 1
    // so we gotta change it first

    // NAMING
    // A - true formula, "final formula"
    // B - A, but normalized (so that A^2 + B^2 + C^2... = 1)
    // C - B, but canonized (convert from b to B=b/2 to be able to use Elberly correctly)
    // D -
    // E -
    // F -
    // G -
    // H -
    // I -

    // ***************

    bool debug = false;
    //debug = false;
    //debug = true;

    // *************

    struct Formulas {
        ConicFormula formA;
        ConicFormula formBNormalized;
        ConicFormula formCCanonized;

        ConicFormula formDRotated;
        ConicFormula formDRotatedSecondVersion;

        ConicFormula formEMovedToOrigin;
        ConicFormula formFSwappedXY;
        ConicFormula formGNegatedAllSigns;
        ConicFormula formHNegatedX;
        ConicFormula formINegatedY;

        ConicFormula current;
    };

    Formulas f;



    auto writeFormulaInWolframAlphaForm = [] (QVector<double> formula, bool trueForm = false) {
        auto writeOutNumber = [] (double n, bool first = false) {
            QString str;
            if (n >= 0 && !first) {
                str = QString("+ ") + QString::number(n);
            } else {
                str = (QString::number(n));
            }
            return str;
        };

        QString str = writeOutNumber(formula[0], true) + "x^2 "
                + writeOutNumber(trueForm? formula[1] : 2*formula[1]) + "xy "
                + writeOutNumber(formula[2]) + "y^2 "
                + writeOutNumber(trueForm? formula[3] : 2*formula[3]) + "x "
                + writeOutNumber(trueForm? formula[4] : 2*formula[4]) + "y "
                + writeOutNumber(formula[5]) + " = 0";
        return str;
    };


    auto writeFormulaInAllForms = [writeFormulaInWolframAlphaForm] (QVector<double> formula, QString name, bool trueForm = false) {
        QString octaveForm = "[";
        for (int i = 0; i < 6; i++) {
            bool shouldDouble = formula[i] && (i == 1 || i == 3 || i == 4);
            octaveForm += QString::number(shouldDouble ? 2*formula[i] : formula[i]);
            if (i != 5) {
                octaveForm += ",";
            }
        }
        octaveForm += "];";

        /*
        qCritical() << "ModifiedElberly +Sec+ | --- (QVector, not Conic) ";
        qCritical() << "ModifiedElberly       | formula name: " << name;
        qCritical() << "ModifiedElberly       | true form? " << trueForm;
        qCritical() << "ModifiedElberly       | WolframAlpha: " << writeFormulaInWolframAlphaForm(formula, trueForm);
        qCritical() << "ModifiedElberly       | Octave: " << octaveForm;
        qCritical() << "ModifiedElberly       | Actual data (A, B, C, D...): " << formula;
        qCritical() << "ModifiedElberly       | ---";
        */
    };


    auto stdDebug = [debug] (ConicFormula f) {
        if (debug) f.printOutInAllForms();
    };

    auto stdDebugVec = [debug, writeFormulaInAllForms] (QVector<double> f, QString name, bool trueForm = false) {
        if (debug) writeFormulaInAllForms(f, name, trueForm);
    };


    if (debug) ENTER_FUNCTION() << "################ ELBERLY THIRD ###################";
    if (debug) ENTER_FUNCTION() << ppVar(polygon) << ppVar(point);



    QVector<double> canonized;
    canonized = QVector<double>::fromList(finalFormula.toList());

    QVector<double> trueFormulaA = QVector<double>::fromList(finalFormula.toList());
    writeFormulaInAllForms(trueFormulaA, "formula A - final", true);
    ConicFormula formA(trueFormulaA, "formula A - final", ConicFormula::ACTUAL);
    if (debug) formA.printOutInAllForms();
    f.formA = ConicFormula(trueFormulaA, "formula A - final", ConicFormula::ACTUAL);
    f.current = f.formA;

    //if (debug)

    if (debug) ENTER_FUNCTION() << ppVar(finalFormula) << ppVar(finalFormula.toList());


    // I guess it should also rescale the ellipse for the points to be within 0-1?... uhhhhh...
    // TODO: make sure it's correct and finished
    // ACTUALLY no, it's the 'point' that should be normalized here, not the ellipse!

    double normalizeBy = 1.0;
    QPointF point3 = point;


    normalizeBy = qMax(point.x(), point.y());

    if (debug) ENTER_FUNCTION() << "NORMALIZATION VALUE HERE: " << ppVar(normalizeBy);
    if (debug) ENTER_FUNCTION() << ppVar(canonized) << ppVar(writeFormulaInWolframAlphaForm(canonized, true)) << ppVar(writeFormulaInWolframAlphaForm(finalFormula, true));
    // załóżmy x+y = 1 oraz nb = 5, bo mielismy x,y = 5,5 -> czyli robimy wszystko mniejsze
    // x => nb*x
    // (nb*x) + (nb*y) = 1
    // 5x + 5y = 1

    std::tuple<ConicFormula, QPointF, double, bool> resultA = ConicCalculations::normalizeFormula(f.formA, point3);
    f.formBNormalized = std::get<0>(resultA);
    normalizeBy = std::get<2>(resultA);
    point = std::get<1>(resultA);
    bool normalized = std::get<3>(resultA);

    if (debug) ENTER_FUNCTION() << "(*) after normalization:" << ppVar(point);

    if (debug) ENTER_FUNCTION() << "after normalization:" << ppVar(canonized) << ppVar(point);
    if (debug) ENTER_FUNCTION() << ppVar(writeFormulaInWolframAlphaForm(canonized, true));

    // Ax^2 + 2Bxy + Cy^2 = 0;
    // a' ^2 + 2b' xy + c' y^2 = 0;

    // 5x^2 + 8xy + 3y^2 = 0
    // 5x^2 + (2*4)xy + 3y^2 = 0
    // 5^2 + 4^2 + 3^2 = 25 + 16 + 9 = k^2 = 50 = 25*2 => 5*sqrt(2) = k
    // A = 5/(5sqrt(2)) = sqrt(2)/2
    // B = (b/2)/(k)

    f.formCCanonized = ConicCalculations::canonizeFormula(f.formBNormalized);

    // ok so canonized is the article's version, as in, the true equation is with 2*canonized[1] for example,
    // and canonized = A, B, C, D...


    // TODO

    // now it should be canonized correctly

    // Stage 2. Rotate the coordinates system to make B = 0.
    // We need to decomposite the matrix:
    // | A B |
    // | B C | = Q*D*Q^T
    // Q - orthogonal, D - diagonal
    //

    bool swap = true;

    auto resultD = ConicCalculations::rotateFormula(f.formCCanonized, point);
    f.formDRotated = std::get<0>(resultD);
    double K = std::get<1>(resultD);
    double L = std::get<2>(resultD);
    point = std::get<3>(resultD);

    KIS_ASSERT_RECOVER_NOOP(qAbs(f.formDRotated.B) < 1e-10);


    //if (debug) ENTER_FUNCTION() << "after rotation:" << ppVar(writeFormulaInWolframAlphaForm(rotated)); // still a circle, good

    if (debug) ENTER_FUNCTION() << "(6a)";


    if (debug) ENTER_FUNCTION() << "(*) after rotation:" << ppVar(point);

    if (debug) ENTER_FUNCTION() << "(6)" << ppVar(point);

    // Stage 3. Moving point to the origin (x -> x - u, y -> y - v)
    qreal u = point.x();
    qreal v = point.y();

    ENTER_FUNCTION() << "THIRD: move using u, v: " << u << v << point;

    if (debug) ENTER_FUNCTION() << "(*) after moving:" << ppVar(QPointF(0, 0));

    ConicFormula rotatedAfterMoving = ConicCalculations::moveToOrigin(f.formDRotated, u, v);

    if (debug) rotatedAfterMoving.printOutInAllForms();

    // Stage 4. Adjusting the coefficients to our needs (C, D, E >= 0)

    f.formEMovedToOrigin = rotatedAfterMoving;
    f.formEMovedToOrigin.Name = "form E - moved to origin (cf)";


    auto adjustCoefficients = [] (Formulas& f, bool& swapXandY, bool& negateX, bool& negateY) {

        swapXandY = false;
        negateX = false;
        negateY = false;

        ConicFormula r = f.formEMovedToOrigin;
        ConicFormula ff = f.formEMovedToOrigin;

        auto resultF = ConicCalculations::swapXY(ff);

        swapXandY = std::get<1>(resultF);

        f.formFSwappedXY = std::get<0>(resultF);
        f.formFSwappedXY.Name = "form F - swapped X and Y (cf)";

        r = f.formFSwappedXY;

        auto resultG = ConicCalculations::negateAllSigns(r);

        f.formGNegatedAllSigns = resultG;
        f.formGNegatedAllSigns.Name = "form G - negated all signs (cf)";

        r = resultG;

        auto resultH = ConicCalculations::negateX(r);



        f.formHNegatedX = std::get<0>(resultH);
        f.formHNegatedX.Name = "form H - negated X (cf)";

        r = f.formHNegatedX;
        negateX = std::get<1>(resultH);

        auto resultI = ConicCalculations::negateY(r);

        f.formINegatedY = std::get<0>(resultI);
        f.formINegatedY.Name = "form H - negated X (cf)";

        negateY = std::get<1>(resultI);


    };

    bool swapXandY = false;
    bool negateX = false;
    bool negateY = false;


    adjustCoefficients(f, swapXandY, negateX, negateY);
    if (qAbs(f.formINegatedY.C) < 1e-12) {
        ENTER_FUNCTION() << "Weird formula: " << "C should not be 0";
        writeFormulaInAllForms(f.formINegatedY.getData(), "form I negated Y");
        //KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(false && "weird formula in Eberly", originalPoint);
        return originalPoint;
    }


    // ok, first we gotta find C being bigger than A (in absolute).

    // ok, now |C| >= |A|

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(f.formINegatedY.C >= 0 && f.formINegatedY.D >= 0 && f.formINegatedY.E >= 0, originalPoint);

    // Stage 5.
    // Thanks to math magic, we've got:
    // x = -Dt/(At + 1)
    // y = -Et/(Ct + 1)
    // hence, the function is:
    // P(t) = -D^2 *t*((At + 2)/(At + 1)^2) - E^2 * t * (Ct+2)/((Ct+1)^2) + F

    auto Pt = [f] (double t) {
        ConicFormula ff = f.formINegatedY;
        return -ff.D*ff.D*t*((ff.A*t + 2)/((ff.A*t + 1)*(ff.A*t + 1))) - ff.E*ff.E * t * (ff.C*t+2)/((ff.C*t+1)*(ff.C*t+1)) + ff.F;
    };

    auto Ptd = [f] (double t) {
        ConicFormula ff = f.formINegatedY;
        return - (2*ff.D*ff.D)/(qPow(ff.A*t*t + 1, 3)) - (2*ff.E*ff.E)/qPow(ff.C*t*t + 1, 3);
    };


    if ((qFuzzyCompare(f.formINegatedY.D, 0) || qFuzzyCompare(f.formINegatedY.E, 0))) {
        // special case
        if (debug) ENTER_FUNCTION() << "Ellipse: and SPECIAL CASE";
        //KIS_SAFE_ASSERT_RECOVER(false && "ellipses have D = 0 or E = 0");
    }

    qreal t0 = ConicCalculations::getStartingPoint(f.formINegatedY, Pt, Ptd);

    // now we gotta find t, then when we find it, calculate x and y, undo all transformations and return the point!

    qreal previousT;
    bool overshoot;
    qreal tresult = AlgebraFunctions::newtonUntilOvershoot(Pt, Ptd, t0, previousT, overshoot, debug);
    if (overshoot) {
        if (debug) ENTER_FUNCTION() << "# NEWTON OVERSHOT # t: " << ppVar(tresult) << ppVar(previousT) << ppVar(Pt(tresult)) << ppVar(Pt(previousT));
        tresult = AlgebraFunctions::bisectionMethod(Pt, qMin(tresult, previousT), qMax(tresult, previousT), debug);
        if (debug) ENTER_FUNCTION() << "# AFTER BISECTION # Now it's: " << ppVar(tresult);
    } else {
        if (debug) ENTER_FUNCTION() << "# NO OVERSHOOT # values are: " << ppVar(tresult) << ppVar(previousT) << ppVar(Pt(tresult)) << ppVar(Pt(previousT));
    }


    qreal foundX = -f.formINegatedY.D*tresult/qPow(f.formINegatedY.A*tresult + 1, 1);
    qreal foundY = -f.formINegatedY.E*tresult/qPow(f.formINegatedY.C*tresult + 1, 1);



    qreal checkIfOnTheEllipse = f.formINegatedY.calculateFormulaForPoint(QPointF(foundX, foundY));

    if (debug) ENTER_FUNCTION() << ppVar(checkIfOnTheEllipse);
    if (qAbs(checkIfOnTheEllipse) >= 1e-6) {
        if (debug) ENTER_FUNCTION() << "#### WARNING! NOT ON THE ELLIPSE AFTER BISECTION!!! ####";
    }

    if (debug) ENTER_FUNCTION() << "(1) original" << ppVar(foundX) << ppVar(foundY) << ppVar(calculateFormula(QPointF(foundX, foundY)))
                                << ppVar(f.formINegatedY.calculateFormulaForPoint(QPointF(foundX, foundY)));
    if (debug) ENTER_FUNCTION() << "(1) reminder: " << ppVar(QPointF(foundX, foundY));
    if (debug) f.formINegatedY.printOutInAllForms();

    QPointF result = ConicCalculations::undoConicFormulaCoefficientsChanges(QPointF(foundX, foundY), swapXandY, negateX, negateY, u, v);


    if (debug) ENTER_FUNCTION() << "(4) after moving to the origin" << ppVar(result) << ppVar(calculateFormula(result))
                                   << ppVar(f.formDRotated.calculateFormulaForPoint(result));

    //if (debug) ENTER_FUNCTION() << "before the mistake" << ppVar(result) << ppVar(writeFormulaInWolframAlphaForm(f.formDRotated.getFormulaSpecial()))
    //                             << ppVar((double)Q(0, 0)) << ppVar((double)Q(1, 1));

    // un-rotating (with Q)

    // result = QPointF(result.x()/Q(0, 0), result.y()/Q(1, 1));
    // Q must be transposed now
    if (debug) ENTER_FUNCTION() << "Ok, now undoing the rotation for the point:";

    result = ConicCalculations::undoRotatingFormula(result, K, L);

    if (debug) ENTER_FUNCTION() << "(5) after un-rotating" << ppVar(result) << ppVar(calculateFormula(result)) << ppVar(calculateFormula(canonized, result))
                                   << ppVar(f.formCCanonized.calculateFormulaForPoint(result))
                                   << ppVar(f.formBNormalized.calculateFormulaForPoint(result));


    // uncanonizing ????


    // now un-normalizing
    if (debug) ENTER_FUNCTION() << "NORMALIZATION VALUE: " << ppVar(normalizeBy);

    if (!qFuzzyCompare(normalizeBy, 0)) {
        if (debug) ENTER_FUNCTION() << "NORMALIZATION being done: " << ppVar(normalizeBy);
        result = result*normalizeBy;
    }
    if (debug) ENTER_FUNCTION() << "(6) after un-normalizing" << ppVar(result) << ppVar(calculateFormula(result)) << ppVar(calculateFormula(trueFormulaA, result));

    qreal eps = 0.0001;
    if (qAbs(calculateFormula(result)) >= eps) {
        ENTER_FUNCTION() << ppVar(qAbs(calculateFormula(result))) << ppVar(qAbs(calculateFormula(result)) < eps);
        ENTER_FUNCTION() << "The values were: " << ppVar(polygon) << ppVar(originalPoint) << "and unfortunate result:" << ppVar(result);
        //KIS_SAFE_ASSERT_RECOVER(false); // << it catches here! // "Entering "EllipseInPolygon::projectModifiedEberlyThird()" The values were:  polygon = QVector() originalPoint = QPointF(1285.92,84.726) and unfortunate result: result = QPointF(1283.45,83.2824)"
    }
    //KIS_ASSERT_RECOVER_RETURN_VALUE(qAbs(calculateFormula(result)) < eps, result);

    if (debug || true) {
        qCritical() << "**** THIRD ****";
        f.formA.printOutInAllForms();
        f.formBNormalized.printOutInAllForms();
        f.formCCanonized.printOutInAllForms();
        f.formDRotated.printOutInAllForms();
        f.formDRotatedSecondVersion.printOutInAllForms();
        f.formEMovedToOrigin.printOutInAllForms();
        f.formFSwappedXY.printOutInAllForms();
        f.formGNegatedAllSigns.printOutInAllForms();
        f.formHNegatedX.printOutInAllForms();
        f.formINegatedY.printOutInAllForms();
    }


    return result;

}


bool EllipseInPolygon::onTheCorrectSideOfHorizon(QPointF point)
{
    // point is the new potential start of the concentric ellipse

    int here = horizonLineSign(point);
    ENTER_FUNCTION() << "Concentric point is: " << point << "and it's horizon side is: " << here;

    Q_FOREACH(QPointF p, originalPoints) {
        int pSign = horizonLineSign(p);
        ENTER_FUNCTION() << "Point on the ellipse is: " << p << "and it's horizon sign is: " << pSign;
    }
    ENTER_FUNCTION() << "----";

    if (here == 0) {
        return !m_mirrored; // let's keep points on the horizon on the not-mirrored side
    }
    return here >= 0;
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
    //ENTER_FUNCTION() << "(b*b - 4*a*c)" << (b*b - 4*a*c) << " < 0 (should be for ellipse)" << a << b << c;
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

qreal EllipseInPolygon::calculateFormula(QPointF point)
{
    return calculateFormula(finalFormula, point);
}

qreal EllipseInPolygon::calculateFormula(QVector<double> formula, QPointF point)
{
    return    formula[0]*point.x()*point.x() // a
            + formula[1]*point.x()*point.y() // b
            + formula[2]*point.y()*point.y() // c
            + formula[3]*point.x() // d
            + formula[4]*point.y() // e
            + formula[5]; // f
}

QVector<double> EllipseInPolygon::getRotatedFormula(QVector<double> original, qreal& K, qreal& L)
{
    qreal a = original[0];
    qreal b = original[1];
    qreal c = original[2];
    qreal d = original[3];
    qreal e = original[4];
    qreal f = original[5];

    qreal angle = qAtan2(b, a - c)/2;

    // use finalAxisAngle to find the cos and sin
    // and replace the final coordinate system with the rerotated one
    K = qCos(-angle);
    L = qSin(-angle);

    // this allows to calculate the formula for the rerotated ellipse
    qreal aprim = K*K*a - K*L*b + L*L*c;
    qreal bprim = 2*K*L*a + K*K*b - L*L*b - 2*K*L*c;
    qreal cprim = L*L*a + K*L*b + K*K*c;
    qreal dprim = K*d - L*e;
    qreal eprim = L*d + K*e;
    qreal fprim = f;

    //finalAxisReverseAngleCos = K;
    //finalAxisReverseAngleSin = L;

    // third attempt at new center:
    // K' = K
    // L' = -L
    // note that this will be in a different place, because the ellipse wasn't moved to have center in (0, 0), but still rotate around point (0,0)
    // and any point that is not (0, 0), when rotated around (0, 0) with an angle that isn't 0, 360 etc. degrees, will end up in a different place
    //QPointF rerotatedCenter = QPointF(K*finalEllipseCenter[0] - L*finalEllipseCenter[1], K*finalEllipseCenter[1] + L*finalEllipseCenter[0]);


    QVector<double> response;
    response << aprim << bprim << cprim << dprim << eprim << fprim;
    return response;

}

QPointF EllipseInPolygon::getRotatedPoint(QPointF point, qreal K, qreal L, bool unrotate)
{
    qreal k = K;
    qreal l = unrotate ? -L : L;
    QPointF response = QPointF(k*point.x() - l*point.y(), k*point.y() + l*point.x());
    return response;
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
        //ENTER_FUNCTION() << "Formula doesn't represent an ellipse:" << a << b << c << d << e << f;
        //ENTER_FUNCTION() << "Is ellipse valid?" << isValid();
        //return false; //moved later
    }

    setFormula(finalFormula, a, b, c, d, e, f);
    curveType = curveTypeForFormula(a, b, c);

    // check if this is an ellipse
    if (!formulaRepresentsAnEllipse(a, b, c)) {
        //ENTER_FUNCTION() << "Formula doesn't represent an ellipse:" << a << b << c << d << e << f;
        //ENTER_FUNCTION() << "Is ellipse valid?" << isValid();
        //return false; //moved later
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

// ********* CONIC FORMULA CLASS *********

ConicFormula::ConicFormula()
{

}

ConicFormula::ConicFormula(QVector<double> formula, QString name, TYPE type = SPECIAL)
{
    if (type == SPECIAL) {
        setFormulaSpecial(formula);
    } else {
        setFormulaActual(formula);
    }
    Name = name;
}

qreal ConicFormula::calculateFormulaForPoint(QPointF point)
{
    if (isSpecial()) {
        return A*point.x()*point.x() + 2*B*point.x()*point.y() + C*point.y()*point.y()
                + 2*D*point.x() + 2*E*point.y() + F;
    } else {
        return A*point.x()*point.x() + B*point.x()*point.y() + C*point.y()*point.y()
                + D*point.x() + E*point.y() + F;
    }

}

void ConicFormula::setFormulaActual(QVector<double> formula)
{
    KIS_ASSERT_RECOVER_RETURN(formula.size() >= 6);
    setFormulaActual(formula[0], formula[1], formula[2], formula[3], formula[4], formula[5]);
}

void ConicFormula::setFormulaActual(qreal a, qreal b, qreal c, qreal d, qreal e, qreal f)
{
    Type = ACTUAL;
    setData(a, b, c, d, e, f);
}

void ConicFormula::setFormulaSpecial(QVector<double> formula)
{
    KIS_ASSERT_RECOVER_RETURN(formula.size() >= 6);
    setFormulaSpecial(formula[0], formula[1], formula[2], formula[3], formula[4], formula[5]);
}

void ConicFormula::setFormulaSpecial(qreal a, qreal b, qreal c, qreal d, qreal e, qreal f)
{
    Type = SPECIAL;
    setData(a, b, c, d, e, f);
}

QVector<double> ConicFormula::getFormulaSpecial()
{
    QVector<double> response;
    if (isSpecial()) {
        response << A << B << C << D << E << F;
    } else {
        response << A << B/2 << C << D/2 << E/2 << F;
    }
    return response;
}

QVector<double> ConicFormula::getFormulaActual()
{
    QVector<double> response;
    if (isSpecial()) {
        response << A << 2*B << C << 2*D << 2*E << F;
    } else {
        response << A << B << C << D << E << F;
    }
    return response;
}

QVector<double> ConicFormula::getData()
{
    QVector<double> response;
    response << A << B << C << D << E << F;
    return response;
}

void ConicFormula::convertTo(TYPE type)
{
    if (Type != type) {
        if (Type == SPECIAL) { // to "actual"
            QVector<double> actual = getFormulaActual();
            setFormulaActual(actual);
        } else { // to "special"
            QVector<double> special = getFormulaSpecial();
            setFormulaSpecial(special);
        }
    }
}

QString ConicFormula::toWolframAlphaForm()
{
    auto writeOutNumber = [] (double n, bool first = false) {
        QString str;
        if (n >= 0 && !first) {
            str = QString("+ ") + QString::number(n);
        } else {
            str = (QString::number(n));
        }
        return str;
    };

    QString str = writeOutNumber(A, true) + "x^2 "
            + writeOutNumber(isSpecial()? 2*B : B) + "xy "
            + writeOutNumber(C) + "y^2 "
            + writeOutNumber(isSpecial()? 2*D : D) + "x "
            + writeOutNumber(isSpecial()? 2*E : E) + "y "
            + writeOutNumber(F) + " = 0";
    return str;
}

void ConicFormula::printOutInAllForms()
{

    QVector<double> actualForm = getFormulaActual();
    QString octaveForm = "[";
    for (int i = 0; i < 6; i++) {
        //bool shouldDouble = isSpecial && (i == 1 || i == 3 || i == 4);
        octaveForm += QString::number(actualForm[i]);
        if (i != 5) {
            octaveForm += ",";
        }
    }
    octaveForm += "];";

    qCritical() << "ModifiedElberly | ---";
    qCritical() << "ModifiedElberly | formula name: " << Name;
    qCritical() << "ModifiedElberly | true form? " << Type;
    qCritical() << "ModifiedElberly | WolframAlpha: " << toWolframAlphaForm();
    qCritical() << "ModifiedElberly | Octave: " << octaveForm;
    qCritical() << "ModifiedElberly | Actual data (A, B, C, D...): " << getData();
    qCritical() << "ModifiedElberly | ---";
}

bool ConicFormula::operator ==(ConicFormula f)
{
    return f.Type == Type
            && f.A == A
            && f.B == B
            && f.C == C
            && f.D == D
            && f.E == E
            && f.F == F;
}

bool ConicFormula::operator ==(std::pair<QVector<double>, bool> f)
{
    return isSpecial() == f.second
            && f.first[0] == A
            && f.first[1] == B
            && f.first[2] == C
            && f.first[3] == D
            && f.first[4] == E
            && f.first[5] == F;
}

void ConicFormula::setData(qreal a, qreal b, qreal c, qreal d, qreal e, qreal f)
{
    A = a;
    B = b;
    C = c;
    D = d;
    E = e;
    F = f;
}


// -------------------------------


std::tuple<ConicFormula, QPointF, double, bool> ConicCalculations::normalizeFormula(ConicFormula formula, QPointF point)
{
    bool debug = false;
    double normalizeBy = qMax(point.x(), point.y());
    if (debug) ENTER_FUNCTION() << ppVar(normalizeBy);
    if (debug) ENTER_FUNCTION() << ppVar(formula.getFormulaActual()) << ppVar(formula.toWolframAlphaForm());
    // załóżmy x+y = 1 oraz nb = 5, bo mielismy x,y = 5,5 -> czyli robimy wszystko mniejsze
    // x => nb*x
    // (nb*x) + (nb*y) = 1
    // 5x + 5y = 1

    ConicFormula normalized = formula;
    normalized.Name = "form B - normalized (CF)";

    bool wasNormalized = false;

    // ok so,
    if (!qFuzzyCompare(normalizeBy, 0)) {
        normalized.A = normalizeBy*normalizeBy*formula.A; // x^2
        normalized.B = normalizeBy*normalizeBy*formula.B; // x*y
        normalized.C = normalizeBy*normalizeBy*formula.C; // y^2
        normalized.D = normalizeBy*formula.D; // x
        normalized.E = normalizeBy*formula.E; // y
        normalized.F = formula.F;

        point = point/normalizeBy;
        wasNormalized = true;
    }

    if (debug) ENTER_FUNCTION() << ppVar(normalized.toWolframAlphaForm());

    return std::make_tuple(normalized, point, normalizeBy, wasNormalized);

}

ConicFormula ConicCalculations::canonizeFormula(ConicFormula f)
{
    auto sq = [] (qreal a) {return a*a;};
    ConicFormula result;

    double canonizingNumber = sq(f.A) + sq(f.B/2) + sq(f.C) + sq(f.D/2) + sq(f.E/2) + sq(f.F);
    canonizingNumber = sqrt(canonizingNumber);

    bool debug = false;

    if (debug) ENTER_FUNCTION() << ppVar(canonizingNumber);

    //if (debug) writeFormulaInAllForms(canonized, "canonized raw - before canonization");
    if (debug) f.printOutInAllForms();


    // conversion actual -> special is included below
    result = ConicFormula(f.getFormulaActual(), "formula C - canonized", ConicFormula::SPECIAL);

    result.A = f.A/canonizingNumber;
    result.B = f.B/(2*canonizingNumber);
    result.C = f.C/canonizingNumber;
    result.D = f.D/(2*canonizingNumber);
    result.E = f.E/(2*canonizingNumber);
    result.F = f.F/canonizingNumber;

    //if (debug) ENTER_FUNCTION() << "Are those two values the same?? ___________ " << ppVar(canonized[0] - f.formCCanonized.A);
    //if (debug) writeFormulaInAllForms(canonized, "canonized raw");
    if (debug) result.printOutInAllForms();

    qreal checkIfReallyCanonized = sq(result.A) + sq(result.B) + sq(result.C)
            + sq(result.D) + sq(result.E) + sq(result.F);

    if (debug) ENTER_FUNCTION() << ppVar(checkIfReallyCanonized == 1);

    return result;
}

std::tuple<ConicFormula, double, double, QPointF> ConicCalculations::rotateFormula(ConicFormula formula, QPointF point)
{
    ConicFormula result;
    QPointF pointRotatedSecondVersion = point;
    qreal K;
    qreal L;
    QVector<double> formulaDRotatedAlternativeTrue = EllipseInPolygon::getRotatedFormula(formula.getFormulaActual(), K, L);
    pointRotatedSecondVersion = EllipseInPolygon::getRotatedPoint(pointRotatedSecondVersion, K, L);
    result = ConicFormula(formulaDRotatedAlternativeTrue, "formula D - rotated, alternative", ConicFormula::ACTUAL);
    result.convertTo(ConicFormula::SPECIAL);
    return std::make_tuple(result, K, L, pointRotatedSecondVersion);

}

ConicFormula ConicCalculations::moveToOrigin(ConicFormula formula, double u, double v)
{
    ConicFormula response = formula;
    bool debug = true;
    if (debug) ENTER_FUNCTION() << "formula to copy from: " << formula.getFormulaSpecial();
    if (debug) ENTER_FUNCTION() << "formula copied: " << response.getFormulaSpecial();


    // note that F depends on old D and E
    response.D = formula.D + formula.A*u; // D -> D + A*u
    response.E = formula.E + formula.C*v; // E -> E + C*v
    response.F = formula.F + formula.A*u*u + 2*formula.D*u + formula.C*v*v + 2*formula.E*v;// F -> F + Au^2 + 2*Du + Cv^2 + 2Ev

    if (debug) ENTER_FUNCTION() << "D = " << formula.D << " + " << formula.A << "*" << u;
    if (debug) ENTER_FUNCTION() << "E = " << formula.E << " + " << formula.C << "*" << v;
    if (debug) ENTER_FUNCTION() << "F = " << formula.F << " + " << formula.A << "*" << u << "*" << u << " + 2*" << formula.D << "*" << u << " + "
                                << formula.C << "*" << v << "*" << v << " + 2*" << formula.E <<  "*" << v;



    return response;
}

std::tuple<ConicFormula, bool> ConicCalculations::swapXY(ConicFormula ff)
{
    ConicFormula r;
    bool swapXandY = false;
    if (qAbs(ff.C) < qAbs(ff.A)) {
        swapXandY = true;
        r.A = ff.C;
        r.B = ff.B;
        r.C = ff.A;
        r.D = ff.E;
        r.E = ff.D;
        r.F = ff.F;
    }
    return std::make_tuple(r, swapXandY);
}

ConicFormula ConicCalculations::negateAllSigns(ConicFormula formula)
{
    ConicFormula response = formula;
    if (formula.C < 0) {
        // negate all signs
        response.A = -formula.A;
        response.B = -formula.B;
        response.C = -formula.C;
        response.D = -formula.D;
        response.E = -formula.E;
        response.F = -formula.F;
    }
    // note: we don't need to remember whether the formula had all signs negated or not
    return response;
}

std::tuple<ConicFormula, bool> ConicCalculations::negateX(ConicFormula formula)
{
    ConicFormula response = formula;
    bool negated = false;
    if (formula.D < 0) {
        response.D = -formula.D;
        response.B = -formula.B;
        negated = true;
    }
    return std::make_tuple(response, negated);
}

std::tuple<ConicFormula, bool> ConicCalculations::negateY(ConicFormula formula)
{
    ConicFormula response = formula;
    bool negated = false;
    if (formula.E < 0) {
        response.E = -formula.E;
        response.B = -formula.B;
        negated = true;
    }
    return std::make_tuple(response, negated);
}

QPointF ConicCalculations::undoConicFormulaCoefficientsChanges(QPointF p, bool swapXY, bool negateX, bool negateY, double u, double v)
{
    qreal foundX = p.x();
    qreal foundY = p.y();

    // ok, now undoing all the changes we did...


    if (negateX) {
        foundX = -foundX;
    }

    if (negateY) {
        foundY = -foundY;
    }

    QPointF result = QPointF(foundX, foundY);
    if (swapXY){
        result = QPointF(foundY, foundX);
    }

    result = result + QPointF(u, v);

    return result;
}

QPointF ConicCalculations::undoRotatingFormula(QPointF point, double K, double L)
{
    return EllipseInPolygon::getRotatedPoint(point, K, L, true);
}

double ConicCalculations::getStartingPoint(ConicFormula formula, std::function<double (double)> Pt, std::function<double (double)> Ptd)
{
    qreal A = formula.A;
    qreal C = formula.C;
    qreal D = formula.D;
    qreal E = formula.E;
    qreal F = formula.F;
    bool debug = false;


    qreal t0 = 0; // starting point for newton method

    // will be useful for both cases
    qreal t0d = F/(D*D - A*F + D*sqrt(D*D - A*F));
    qreal t0dd = F/(E*E - C*F + E*sqrt(E*E - C*F));

    if (A >= 0) { // ellipses and parabolas
        // we got to find t0 where P(t0) >= 0
        if (F < 0) {
            t0 = qMax(t0d, t0dd);
        } // else t0 = 0
    } else { // hyperbolas

        qreal U = sqrt(sqrt(-A*D*D));
        qreal V = sqrt(sqrt(C*E*E));
        qreal tstar = - (U - V)/(C*U - A*V); // inflection point of Pt

        qreal Pstar = Pt(tstar);
        if (debug) ENTER_FUNCTION() << ppVar(Pstar);

        if (Pstar > 0) { // case (a)
            if (F <= 0) {
                t0 = 0;
            } else {
                t0 = qMin(t0d, t0dd);
            }
        } else if (Pstar < 0) {
            if (F >= 0) {
                t0 = 0;
            } else {
                t0 = qMax(t0d, t0dd);
            }
        } else {
            // we found it already!!!
            t0 = tstar;
        }
    }
    return t0;
}


// -------------------------------

qreal AlgebraFunctions::newtonUntilOvershoot(std::function<qreal (qreal)> f, std::function<qreal (qreal)> fd, qreal t0, qreal &previousT, bool &overshoot, bool debug)
{
    overshoot = false;
    if (debug) ENTER_FUNCTION() << "+++ Newton until overshoot +++";
    if (debug) ENTER_FUNCTION() << "+++ Start t0: +++" << ppVar(t0) << ppVar(previousT);
    qreal t = t0;
    qreal val = f(t);
    qreal valSign = KisAlgebra2D::signPZ(val);

    qreal errorEps = 1e-14;
    qreal derivEps = 1e-14;
    int maxSteps = 1000;
    int i = 0;
    if (debug) ENTER_FUNCTION() << ppVar(t0) << ppVar(val);

    previousT = t0;

    while (qAbs(val) > errorEps && qAbs(fd(t)) > derivEps && i < maxSteps) {
        previousT = t;

        if (debug) ENTER_FUNCTION() << "before changing t: " << ppVar(t) << ppVar(f(t)) << ppVar(fd(t)) << ppVar(-f(t)/fd(t)) << "so new value will be: " << ppVar(t - f(t)/fd(t));
        if (qAbs(fd(t)) < errorEps) {

        }

        t = t - f(t)/fd(t);
        val = f(t);

        if (debug) ENTER_FUNCTION() << ppVar(i) << ppVar(t) << ppVar(val) << ppVar(qAbs(fd(t)));

        if (debug) ENTER_FUNCTION() << "after changing t: " << ppVar(t) << ppVar(f(t)) << ppVar(fd(t)) << ppVar(-f(t)/fd(t)) << "so new value will be: " << ppVar(t - f(t)/fd(t));


        if (KisAlgebra2D::signPZ(val) != valSign) {
            // overshoot!
            if (debug) ENTER_FUNCTION() << "++++++ OVERSHOOT HAPPENED; " << ppVar(val) << ppVar(valSign);

            overshoot = true;
            return t;
        }


        i++;
    }
    return t;
}

qreal AlgebraFunctions::bisectionMethod(std::function<qreal (qreal)> f, qreal ta, qreal tb, bool debug)
{
    if (debug) ENTER_FUNCTION() << "+++ Bisection method +++";
    if (debug) ENTER_FUNCTION() << "Data:" << ppVar(ta) << ppVar(QString::number(tb, 'g', 10));
    qreal errorEps = 1e-14;
    qreal t;
    qreal val;

    qreal fa;
    qreal fb;

    if (qAbs(f(ta)) < errorEps) {
        if (debug) ENTER_FUNCTION() << "Finishing early, because" << ppVar(f(ta));
        return ta;
    }
    if (qAbs(f(tb)) < errorEps) {
        if (debug) ENTER_FUNCTION() << "Finishing early, because" << ppVar(f(tb));
        return tb;
    }

    /*
    if (tb - ta < errorEps) {
        return (tb - ta)/2.0;
    }
    */

    int maxSteps = 100000;
    int expectedSteps = qCeil(log2((tb - ta)/errorEps)); // it's not a correct formula, since it only checks for difference between ta and tb, instead of f(t)
    int i = 0;
    if (debug) ENTER_FUNCTION() << "Expected steps = " << expectedSteps << ppVar(ta) << ppVar(tb) << ppVar(f(ta)) << ppVar(f(tb));

    do {
        t = (ta + tb)/2;
        val = f(t);
        fa = f(ta);
        fb = f(tb);

        if (fa*val < 0) {
            tb = t;
        } else if (fb*val < 0) {
            ta = t;
        } else {
            if (debug) ENTER_FUNCTION() << "Weird situation! but hopefully fixed!";
            if (qAbs(fa) < errorEps) {
                return ta;
            } else if (qAbs(fb) < errorEps) {
                return tb;
            }
            if (debug) ENTER_FUNCTION() << "Weird situation!" << ppVar(i) << ppVar(t) << ppVar(ta) << ppVar(tb) << ppVar(val) << ppVar(fa) << ppVar(fb);
            if (debug) ENTER_FUNCTION() << "Finishing with" << ppVar(ta + (tb - ta)/2) << "and" << ppVar(f(ta + (tb - ta)/2));
            return ta + (tb - ta)/2;
        }
        i++;
        if (debug) ENTER_FUNCTION() << ppVar(i) << "|" << ppVar(t) << " | " << ppVar(val);

    } while (qAbs(val) > errorEps && i < maxSteps);

    if (debug) ENTER_FUNCTION() << "Actual steps = " << i;

    return t;
}
