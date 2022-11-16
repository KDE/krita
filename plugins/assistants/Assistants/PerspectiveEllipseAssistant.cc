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

    // functions

    ///
    /// \brief updateToPolygon
    /// This function makes all the necessary calculations and updates all data, not just the polygon
    ///  according to the polygon that was provided as the parameter
    /// \param polygon polygon that contains the ellipse
    /// \returns whether the ellipse is valid or not
    ///
    bool updateToPolygon(QVector<QPointF> _polygon);

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

protected:

    void setFormula(QVector<double>& formula, double a, double b, double c, double d, double e, double f);
    void setPoint(QVector<double>& point, double x, double y);


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

bool EllipseInPolygon::updateToPolygon(QVector<QPointF> _polygon)
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
        return false;
    }

    setFormula(finalFormula, a, b, c, d, e, f);

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
        return false;
    }

    finalAxisReverseAngleCos = K;
    finalAxisReverseAngleSin = L;

    setFormula(rerotatedFormula, aprim, bprim, cprim, dprim, eprim, fprim);

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

bool EllipseInPolygon::setSimpleEllipseVertices(Ellipse &ellipse) const
{
    if (finalVertices.size() > 2) {
        return ellipse.set(finalVertices[0], finalVertices[1], finalVertices[2]);
    }
    return false;
}

bool EllipseInPolygon::formulaRepresentsAnEllipse(double a, double b, double c)
{
    return (b*b - 4*a*c) < 0;
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


// ################################## Perspective Ellipse Assistant #######################################


class PerspectiveEllipseAssistant::Private
{
public:
    EllipseInPolygon ellipseInPolygon;
    Ellipse simpleEllipse;

    bool cacheValid { false };

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
}

KisPaintingAssistantSP PerspectiveEllipseAssistant::clone(QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap) const
{
    return KisPaintingAssistantSP(new PerspectiveEllipseAssistant(*this, handleMap));
}

QPointF PerspectiveEllipseAssistant::project(const QPointF& pt, const QPointF& strokeBegin)
{
    Q_UNUSED(strokeBegin);
    Q_ASSERT(isAssistantComplete());

    d->ellipseInPolygon.setSimpleEllipseVertices(d->simpleEllipse);

    return d->simpleEllipse.project(pt);
}

QPointF PerspectiveEllipseAssistant::adjustPosition(const QPointF& pt, const QPointF& strokeBegin, const bool /*snapToAny*/)
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
    QPointF mousePos = QPointF(0, 0);

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

    if (canvas){
        // simplest, cheapest way to get the mouse-position
        isEditing = canvas->paintingAssistantsDecoration()->isEditingAssistants();
        mousePos = canvas->canvasWidget()->mapFromGlobal(QCursor::pos());
    }
    else {
        // ...of course, you need to have access to a canvas-widget for that.
        mousePos = QCursor::pos(); // this'll give an offset
        dbgFile<<"canvas does not exist, you may have passed arguments incorrectly:"<<canvas;
    }

    if (m_followBrushPosition && m_adjustedPositionValid) {
        mousePos = initialTransform.map(m_adjustedBrushPosition);
    }

    // draw ellipse and axes
    if (isAssistantComplete() && isEllipseValid() && (assistantVisible || previewVisible || isEditing)) { // ensure that you only draw the ellipse if it's valid - otherwise it would just show some outdated one
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

         if (assistantVisible) {
             drawPath(gc, touchingLine, isSnappingActive());
         }
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

    if (!PerspectiveBasedAssistantHelper::getTetragon(handles(), isAssistantComplete(), poly)) { // this function changes poly to some "standardized" version, or a triangle when it cannot be achieved

        poly = QPolygonF(d->cachedPoints);
        poly << d->cachedPoints[0];
        d->ellipseInPolygon.updateToPolygon(poly);
        d->cacheValid = true;
        return;
    }

    d->ellipseInPolygon.updateToPolygon(poly);
    if (d->ellipseInPolygon.isValid()) {
        d->ellipseInPolygon.setSimpleEllipseVertices(d->simpleEllipse);
    }

    PerspectiveBasedAssistantHelper::updateCacheData(d->cache, poly);
    d->cacheValid = true;

}

bool PerspectiveEllipseAssistant::isAssistantComplete() const
{   
    return handles().size() >= 4;
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

