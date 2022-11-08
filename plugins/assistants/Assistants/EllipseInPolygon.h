/*
 * SPDX-FileCopyrightText: 2022 Srirupa Datta <srirupa.sps@gmail.com>
 */

#ifndef _ELLIPSE_IN_POLYGON_H_
#define _ELLIPSE_IN_POLYGON_H_

#include "kis_abstract_perspective_grid.h"
#include "kis_painting_assistant.h"
#include "Ellipse.h"
#include <QObject>


class ConicFormula
{

public:
    // since some formulas in the article are explicitely in that "their" canonical form
    // it needs to remember that form
    // how should I call it...
    // Actual and Special?

    typedef enum TYPE {
        SPECIAL,
        ACTUAL, // or "true form"
    } TYPE;

    ConicFormula();
    ConicFormula(QVector<double> formula, QString name, TYPE Type);


    void setFormulaActual(QVector<double> formula);
    void setFormulaActual(qreal a, qreal b, qreal c, qreal d, qreal e, qreal f);

    void setFormulaSpecial(QVector<double> formula);
    void setFormulaSpecial(qreal a, qreal b, qreal c, qreal d, qreal e, qreal f);

    // return the formula in the special ("canonical" by the article authors) form
    // (with Ax^2 + 2Bxy + Cy^2... etc.)
    QVector<double> getFormulaSpecial();
    // return the formula in the actual form (with Ax^2 + Bxy + Cy^2... etc.)
    QVector<double> getFormulaActual();
    // return the data as they are stored (A, B, C, ...)
    QVector<double> getData();

    void convertTo(TYPE type);

    // functions helpful for debugging
    QString toWolframAlphaForm();
    void printOutInAllForms();

    bool isSpecial() { return Type == SPECIAL; }


    // data
    TYPE Type {SPECIAL};

    // if it's *not* special, then it contains actual true data
    // if it is special, then it contains those special coefficients
    qreal A {1.0};
    qreal B {0.0};
    qreal C {1.0};
    qreal D {0.0};
    qreal E {0.0};
    qreal F {0.0};

    QString Name {"formula"};

private:
    void setData(qreal a, qreal b, qreal c, qreal d, qreal e, qreal f);

};


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

    typedef enum DEGENERATE_TYPE {
        NON_DEGENERATE,
        DEGENERATE_HYPERBOLA_INTERSECTING_LINES,
        DEGENERATE_PARABOLA_PARALLEL_LINES,
        DEGENERATE_ELLIPSE_POINT,
        DEGENERATE_NOT_THIRD_DEGREE, // when A=B=C
    } DEGENERATE_TYPE;



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

    QPointF project(QPointF point);

    QPointF projectModifiedEberly(QPointF point);


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

    ///
    /// \brief calculateFormula
    /// the formula is: ax^2 + bxy + cy^2 + dx + ey + f = 0
    /// this returns the value of the left side for the (x, y) of the point
    /// \param point
    /// \return value of the left side of the formula (that should be 0 for points on the ellipse)
    ///
    qreal calculateFormula(QPointF point);

    ///
    /// \brief calculateFormula
    /// the formula is: ax^2 + bxy + cy^2 + dx + ey + f = 0
    /// this returns the value of the left side for the (x, y) of the point
    /// \param formula - the formula to use
    /// \param point
    /// \return value of the left side of the formula (that should be 0 for points on the ellipse)
    ///
    static qreal calculateFormula(QVector<double> formula, QPointF point);



    static QVector<double> getRotatedFormula(QVector<double> original, QPointF &pointToRotateTogether);


    bool m_valid {false};

};





#endif // _ELLIPSE_IN_POLYGON_H_
