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

#include "kis_polygonal_gradient_shape_strategy.h"

#include "kis_debug.h"

#include "kis_algebra_2d.h"

#include <config-gsl.h>

#ifdef HAVE_GSL
#include <gsl/gsl_multimin.h>
#endif /* HAVE_GSL */


#include <boost/math/distributions/normal.hpp>

#include <QPainterPath>
#include "krita_utils.h"



namespace Private {

    qreal getDisnormedGradientValue(const QPointF &pt, const QPolygonF &selection, qreal exponent)
    {
        // FIXME: exponent = 2.0
        //        We explicitly use pow2() and sqrt() functions here
        //        for efficiency reasons.

        const qreal minHiLevel = std::pow(0.5, 1.0 / exponent);
        qreal ptWeightNode = 0.0;

        for (int i = 0; i < selection.size(); i++) {
            const int prevI = i > 0 ? i - 1 : selection.size() - 1;
            const QPointF edgeP1 = selection[prevI];
            const QPointF edgeP2 = selection[i];

            const QPointF edgeVec = edgeP1 - edgeP2;
            const QPointF q1 = pt - edgeP1;
            const QPointF q2 = pt - edgeP2;

            const qreal proj1 = KisAlgebra2D::dotProduct(edgeVec, q1);
            const qreal proj2 = KisAlgebra2D::dotProduct(edgeVec, q2);

            qreal hi = 1.0;

            // pt's projection lays outside the edge itself,
            // when the projections has the same sign

            if (proj1 * proj2 >= 0) {
                QPointF nearestPointVec =
                    qAbs(proj1) < qAbs(proj2) ? q1 : q2;

                hi = KisAlgebra2D::norm(nearestPointVec);
            } else {
                QLineF line(edgeP1, edgeP2);
                hi = kisDistanceToLine(pt, line);
            }

            hi = qMax(minHiLevel, hi);

            // disabled for efficiency reasons
            // ptWeightNode += 1.0 / std::pow(hi, exponent);

            ptWeightNode += 1.0 / pow2(hi);
        }

        // disabled for efficiency reasons
        // return 1.0 / std::pow(ptWeightNode, 1.0 / exponent);

        return 1.0 / std::sqrt(ptWeightNode);
    }

#ifdef HAVE_GSL

    struct GradientDescentParams {
        QPolygonF selection;
        qreal exponent;
        bool searchForMax;
    };

    double errorFunc (const gsl_vector * x, void *paramsPtr)
    {
        double vX = gsl_vector_get(x, 0);
        double vY = gsl_vector_get(x, 1);

        const GradientDescentParams *params =
            static_cast<const GradientDescentParams*>(paramsPtr);

        qreal weight = getDisnormedGradientValue(QPointF(vX, vY),
                                                 params->selection,
                                                 params->exponent);

        return params->searchForMax ? 1.0 / weight : weight;
    }

    qreal calculateMaxWeight(const QPolygonF &selection,
                             qreal exponent,
                             bool searchForMax)
    {
        const gsl_multimin_fminimizer_type *T =
            gsl_multimin_fminimizer_nmsimplex2;
        gsl_multimin_fminimizer *s = NULL;
        gsl_vector *ss, *x;
        gsl_multimin_function minex_func;

        size_t iter = 0;
        int status;
        double size;

        QPointF center;
        for (int i = 0; i < selection.size(); i++) {
            center += selection[i];
        }
        center /= selection.size();

        /* Starting point */
        x = gsl_vector_alloc (2);
        gsl_vector_set (x, 0, center.x());
        gsl_vector_set (x, 1, center.y());

        /* Set initial step sizes to 10 px */
        ss = gsl_vector_alloc (2);
        gsl_vector_set (ss, 0, 10);
        gsl_vector_set (ss, 1, 10);

        GradientDescentParams p;

        p.selection = selection;
        p.exponent = exponent;
        p.searchForMax = searchForMax;

        /* Initialize method and iterate */
        minex_func.n = 2;
        minex_func.f = errorFunc;
        minex_func.params = (void*)&p;

        s = gsl_multimin_fminimizer_alloc (T, 2);
        gsl_multimin_fminimizer_set (s, &minex_func, x, ss);

        qreal result = searchForMax ?
            getDisnormedGradientValue(center, selection, exponent) : 0.0;

        do
        {
            iter++;
            status = gsl_multimin_fminimizer_iterate(s);

            if (status)
                break;

            size = gsl_multimin_fminimizer_size (s);
            status = gsl_multimin_test_size (size, 1e-6);

            if (status == GSL_SUCCESS)
            {
                // qDebug() << "*******Converged to minimum";
                // qDebug() << gsl_vector_get (s->x, 0)
                //          << gsl_vector_get (s->x, 1)
                //          << "|" << s->fval << size;

                result = searchForMax ? 1.0 / s->fval : s->fval;
            }
        }
        while (status == GSL_CONTINUE && iter < 10000);

        gsl_vector_free(x);
        gsl_vector_free(ss);
        gsl_multimin_fminimizer_free (s);

        return result;
    }

#else /* HAVE_GSL */

    qreal calculateMaxWeight(const QPolygonF &selection,
                             qreal exponent,
                             bool searchForMax)
    {
        QPointF center;
        for (int i = 0; i < selection.size(); i++) {
            center += selection[i];
        }
        center /= selection.size();

        return searchForMax ?
            getDisnormedGradientValue(center, selection, exponent) : 0.0;
    }

#endif /* HAVE_GSL */

}

KisPolygonalGradientShapeStrategy::KisPolygonalGradientShapeStrategy(const QPolygonF &selection,
                                                                     qreal exponent)
    : m_exponent(exponent)
{
    QPainterPath p;
    p.addPolygon(selection);

    const qreal length = p.length();
    const qreal lengthStep =
        KritaUtils::maxDimensionPortion(selection.boundingRect(),
                                        0.01, 3.0);
    const qreal portionStep = qMin(lengthStep / length, 0.01);

    for (qreal t = 0.0; t < 1.0; t += portionStep) {
        m_selection << p.pointAtPercent(t);
    }

    m_maxWeight = Private::calculateMaxWeight(m_selection, m_exponent, true);
    m_minWeight = Private::calculateMaxWeight(m_selection, m_exponent, false);

    m_scaleCoeff = 1.0 / (m_maxWeight - m_minWeight);
}

KisPolygonalGradientShapeStrategy::~KisPolygonalGradientShapeStrategy()
{
}

double KisPolygonalGradientShapeStrategy::valueAt(double x, double y) const
{
    QPointF pt(x, y);
    qreal value = 0.0;

    if (m_selection.containsPoint(pt, Qt::OddEvenFill)) {
        value = Private::getDisnormedGradientValue(pt, m_selection, m_exponent);
        value =  (value - m_minWeight) * m_scaleCoeff;
    }

    return value;
}
