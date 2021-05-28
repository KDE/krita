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

#include "kis_free_transform_strategy_gsl_helpers.h"

#include "tool_transform_args.h"
#include "kis_transform_utils.h"

#include <QMessageBox>
#include <kis_algebra_2d.h>

#include <Eigen/Dense>
#include <kis_algebra_2d.h>

namespace KisAlgebra2D {

// TODO: avoid code-duplication
inline Eigen::Matrix3d fromQTransformStraight(const QTransform &t)
{
    Eigen::Matrix3d m;

    m << t.m11() , t.m12() , t.m13()
        ,t.m21() , t.m22() , t.m23()
        ,t.m31() , t.m32() , t.m33();

    return m;
}
}

#include <config-gsl.h>

#ifdef HAVE_GSL
#include <gsl/gsl_multimin.h>



namespace GSL
{

    struct YScaleStrategy {
        static qreal getScale(const ToolTransformArgs &args) {
            return args.scaleY();
        }

        static void setScale(ToolTransformArgs *args, qreal scale) {
            return args->setScaleY(scale);
        }
    };

    struct XScaleStrategy {
        static qreal getScale(const ToolTransformArgs &args) {
            return args.scaleX();
        }

        static void setScale(ToolTransformArgs *args, qreal scale) {
            return args->setScaleX(scale);
        }
    };

    struct Params1D {
        QPointF staticPointSrc;
        QPointF staticPointDst;
        QPointF movingPointSrc;
        QPointF movingPointDst;

        const ToolTransformArgs *srcArgs;
    };

    template <class Strategy>
        double scaleError1D (const gsl_vector * x, void *paramsPtr)
    {
        double scale = gsl_vector_get(x, 0);
        double tX = gsl_vector_get(x, 1);
        double tY = gsl_vector_get(x, 2);

        const Params1D *params = static_cast<const Params1D*>(paramsPtr);

        ToolTransformArgs args(*params->srcArgs);

        Strategy::setScale(&args, scale);
        args.setTransformedCenter(QPointF(tX, tY));

        KisTransformUtils::MatricesPack m(args);
        QTransform t = m.finalTransform();

        QPointF transformedStaticPoint = t.map(params->staticPointSrc);
        QPointF transformedMovingPoint = t.map(params->movingPointSrc);

        qreal result =
            qAbs((transformedMovingPoint - params->movingPointDst).manhattanLength()) +
            qAbs((transformedStaticPoint - params->staticPointDst).manhattanLength());

        return result;
    }

    template <class Strategy>
        ScaleResult1D calculateScale1D(const ToolTransformArgs &args,
                                       const QPointF &staticPointSrc,
                                       const QPointF &staticPointDst,
                                       const QPointF &movingPointSrc,
                                       const QPointF &movingPointDst)
    {
        const gsl_multimin_fminimizer_type *T =
            gsl_multimin_fminimizer_nmsimplex2;
        gsl_multimin_fminimizer *s = 0;
        gsl_vector *ss, *x;
        gsl_multimin_function minex_func;

        size_t iter = 0;
        int status;
        double size;

        /* Starting point */
        x = gsl_vector_alloc (3);
        gsl_vector_set (x, 0, Strategy::getScale(args));
        gsl_vector_set (x, 1, args.transformedCenter().x());
        gsl_vector_set (x, 2, args.transformedCenter().y());

        KisTransformUtils::MatricesPack m(args);
        QTransform t = m.finalTransform();

        /**
         * Approximate initial offset step by 10% of the moving point
         * offset. It means that the destination point will be reached
         * in at most 10 steps.
         */
        const QPointF transformedMovingPoint = t.map(movingPointSrc);
        const qreal initialStep = 0.1 * kisDistance(transformedMovingPoint, movingPointDst);

        /* Set initial step sizes to 0.1 */
        ss = gsl_vector_alloc (3);
        gsl_vector_set (ss, 0, 0.1);
        gsl_vector_set (ss, 1, initialStep);
        gsl_vector_set (ss, 2, initialStep);

        Params1D p;

        p.staticPointSrc = staticPointSrc;
        p.staticPointDst = staticPointDst;
        p.movingPointSrc = movingPointSrc;
        p.movingPointDst = movingPointDst;
        p.srcArgs = &args;

        /* Initialize method and iterate */
        minex_func.n = 3;
        minex_func.f = scaleError1D<Strategy>;
        minex_func.params = (void*)&p;

        s = gsl_multimin_fminimizer_alloc (T, 3);
        gsl_multimin_fminimizer_set (s, &minex_func, x, ss);

        ScaleResult1D result;
        result.scale = Strategy::getScale(args);
        result.transformedCenter = args.transformedCenter();

        do
        {
            iter++;
            status = gsl_multimin_fminimizer_iterate(s);

            if (status)
                break;

            size = gsl_multimin_fminimizer_size (s);
            status = gsl_multimin_test_size (size, 1e-6);

            /**
             * Sometimes the algorithm may converge to a wrond point,
             * they just try to force it search better or return invalid
             * result.
             */
            if (status == GSL_SUCCESS && scaleError1D<Strategy>(s->x, &p) > 0.5) {
                status = GSL_CONTINUE;
            }

            if (status == GSL_SUCCESS)
            {
                // dbgKrita << "*******Converged to minimum";
                // dbgKrita << gsl_vector_get (s->x, 0)
                //          << gsl_vector_get (s->x, 1)
                //          << gsl_vector_get (s->x, 2)
                //          << "|" << s->fval << size;
                result.scale = gsl_vector_get (s->x, 0);
                result.transformedCenter =
                    QPointF(gsl_vector_get (s->x, 1),
                            gsl_vector_get (s->x, 2));
                result.isValid = true;
            }
        }
        while (status == GSL_CONTINUE && iter < 10000);

        gsl_vector_free(x);
        gsl_vector_free(ss);
        gsl_multimin_fminimizer_free (s);

        return result;
    }

    struct Params2D {
        QPointF staticPointSrc;
        QPointF staticPointDst;

        QPointF movingPointSrc;
        QPointF movingPointDst;

        const ToolTransformArgs *srcArgs;
    };

    double scaleError2D (const gsl_vector * x, void *paramsPtr)
    {
        double scaleX = gsl_vector_get(x, 0);
        double scaleY = gsl_vector_get(x, 1);
        double tX = gsl_vector_get(x, 2);
        double tY = gsl_vector_get(x, 3);

        const Params2D *params = static_cast<const Params2D*>(paramsPtr);

        ToolTransformArgs args(*params->srcArgs);

        args.setScaleX(scaleX);
        args.setScaleY(scaleY);
        args.setTransformedCenter(QPointF(tX, tY));

        KisTransformUtils::MatricesPack m(args);
        QTransform t = m.finalTransform();

        QPointF transformedStaticPoint = t.map(params->staticPointSrc);
        QPointF transformedMovingPoint = t.map(params->movingPointSrc);

        qreal result =
            qAbs(transformedMovingPoint.x() - params->movingPointDst.x()) +
            qAbs(transformedMovingPoint.y() - params->movingPointDst.y()) +
            qAbs(transformedStaticPoint.x() - params->staticPointDst.x()) +
            qAbs(transformedStaticPoint.y() - params->staticPointDst.y());

        return result;
    }

    ScaleResult2D calculateScale2D(const ToolTransformArgs &args,
                                   const QPointF &staticPointSrc,
                                   const QPointF &staticPointDst,
                                   const QPointF &movingPointSrc,
                                   const QPointF &movingPointDst)
    {
        const gsl_multimin_fminimizer_type *T =
            gsl_multimin_fminimizer_nmsimplex2;
        gsl_multimin_fminimizer *s = 0;
        gsl_vector *ss, *x;
        gsl_multimin_function minex_func;

        size_t iter = 0;
        int status;
        double size;

        /* Starting point */
        x = gsl_vector_alloc (4);
        gsl_vector_set (x, 0, args.scaleX());
        gsl_vector_set (x, 1, args.scaleY());
        gsl_vector_set (x, 2, args.transformedCenter().x());
        gsl_vector_set (x, 3, args.transformedCenter().y());

        /* Set initial step sizes to 0.1 */
        ss = gsl_vector_alloc (4);
        gsl_vector_set (ss, 0, 0.1);
        gsl_vector_set (ss, 1, 0.1);
        gsl_vector_set (ss, 2, 10);
        gsl_vector_set (ss, 3, 10);

        Params2D p;

        p.staticPointSrc = staticPointSrc;
        p.staticPointDst = staticPointDst;
        p.movingPointSrc = movingPointSrc;
        p.movingPointDst = movingPointDst;
        p.srcArgs = &args;

        /* Initialize method and iterate */
        minex_func.n = 4;
        minex_func.f = scaleError2D;
        minex_func.params = (void*)&p;

        s = gsl_multimin_fminimizer_alloc (T, 4);
        gsl_multimin_fminimizer_set (s, &minex_func, x, ss);

        ScaleResult2D result;
        result.scaleX = args.scaleX();
        result.scaleY = args.scaleY();
        result.transformedCenter = args.transformedCenter();

        do
        {
            iter++;
            status = gsl_multimin_fminimizer_iterate(s);

            if (status)
                break;

            size = gsl_multimin_fminimizer_size (s);
            status = gsl_multimin_test_size (size, 1e-6);

            /**
             * Sometimes the algorithm may converge to a wrond point,
             * they just try to force it search better or return invalid
             * result.
             */
            if (status == GSL_SUCCESS && scaleError2D(s->x, &p) > 0.5) {
                status = GSL_CONTINUE;
            }

            if (status == GSL_SUCCESS)
            {
                // dbgKrita << "*******Converged to minimum";
                // dbgKrita << gsl_vector_get (s->x, 0)
                //          << gsl_vector_get (s->x, 1)
                //          << gsl_vector_get (s->x, 2)
                //          << gsl_vector_get (s->x, 3)
                //          << "|" << s->fval << size;
                result.scaleX = gsl_vector_get (s->x, 0);
                result.scaleY = gsl_vector_get (s->x, 1);
                result.transformedCenter =
                    QPointF(gsl_vector_get (s->x, 2),
                            gsl_vector_get (s->x, 3));
                result.isValid = true;
            }
        }
        while (status == GSL_CONTINUE && iter < 10000);

        gsl_vector_free(x);
        gsl_vector_free(ss);
        gsl_multimin_fminimizer_free (s);

        return result;
    }

    ScaleResult2D calculateScale2DAffine(const ToolTransformArgs &args,
                                         const QPointF &staticPointSrc,
                                         const QPointF &staticPointDst,
                                         const QPointF &movingPointSrc,
                                         const QPointF &movingPointDst)
    {
        KisTransformUtils::MatricesPack m(args);

        /// We are solving an system of two equations:
        ///
        /// T' * projP' * S' * SC' * TS' * P_src1 = P_dst2
        /// T' * projP' * S' * SC' * TS' * P_src2 = P_dst2
        ///
        /// When projP is an affine transformation matrix,
        /// this system has a trivial solution. For the
        /// non-affine case, the things get much more
        /// complicated...

        Eigen::Matrix3d TS_t = KisAlgebra2D::fromQTransformStraight(m.TS.transposed());
        Eigen::Matrix3d S_t = KisAlgebra2D::fromQTransformStraight(m.S.transposed());
        Eigen::Matrix3d projP_t = KisAlgebra2D::fromQTransformStraight(m.projectedP.transposed());

        Eigen::Matrix3d M1 = projP_t * S_t;

        Eigen::Matrix<double, 3, 2> P_src;
        P_src << staticPointSrc.x(), movingPointSrc.x(),
                 staticPointSrc.y(), movingPointSrc.y(),
                 1, 1;

        P_src = TS_t * P_src;

        Eigen::Matrix<double, 3, 2> P_dst;
        P_dst << staticPointDst.x(), movingPointDst.x(),
                 staticPointDst.y(), movingPointDst.y(),
                 1, 1;

        Eigen::Matrix<double, 4, 4> A;
        A << M1(0,0) * P_src(0,0), M1(0,1) * P_src(1,0), 1, 0,
             M1(1,0) * P_src(0,0), M1(1,1) * P_src(1,0), 0, 1,
             M1(0,0) * P_src(0,1), M1(0,1) * P_src(1,1), 1, 0,
             M1(1,0) * P_src(0,1), M1(1,1) * P_src(1,1), 0, 1;

        Eigen::Matrix<double, 4, 1> B;
        B << P_dst(0,0), P_dst(1,0), P_dst(0,1), P_dst(1,1);


        ScaleResult2D result;

        Eigen::Matrix<double, 4, 1> X = A.inverse() * B;

        result.isValid = !qFuzzyIsNull(A.determinant());

        if (result.isValid) {
            result.scaleX = X(0);
            result.scaleY = X(1);
            result.transformedCenter.rx() = X(2);
            result.transformedCenter.ry() = X(3);
        }

        return result;
    }

    ScaleResult1D calculateScaleX(const ToolTransformArgs &args,
                                  const QPointF &staticPointSrc,
                                  const QPointF &staticPointDst,
                                  const QPointF &movingPointSrc,
                                  const QPointF &movingPointDst)
    {
        return calculateScale1D<XScaleStrategy>(args,
                                                staticPointSrc,
                                                staticPointDst,
                                                movingPointSrc,
                                                movingPointDst);
    }

    ScaleResult1D calculateScaleY(const ToolTransformArgs &args,
                                  const QPointF &staticPointSrc,
                                  const QPointF &staticPointDst,
                                  const QPointF &movingPointSrc,
                                  const QPointF &movingPointDst)
    {
        return calculateScale1D<YScaleStrategy>(args,
                                                staticPointSrc,
                                                staticPointDst,
                                                movingPointSrc,
                                                movingPointDst);
    }

}

#else /* HAVE_GSL */

namespace GSL
{

    void warnNoGSL()
    {
        QMessageBox::warning(0,
                             i18nc("@title:window", "Krita"),
                             i18n("Sorry, Krita was built without the support "
                                  "of GNU Scientific Library, so you cannot scale "
                                  "the selection with handles. Please compile "
                                  "Krita with GNU Scientific Library support, or use "
                                  "options widget for editing scale values manually."));
    }

    ScaleResult2D calculateScale2D(const ToolTransformArgs &args,
                                   const QPointF &staticPointSrc,
                                   const QPointF &staticPointDst,
                                   const QPointF &movingPointSrc,
                                   const QPointF &movingPointDst)
    {
        warnNoGSL();

        ScaleResult2D result;
        result.scaleX = args.scaleX();
        result.scaleY = args.scaleY();
        result.transformedCenter = args.transformedCenter();
        return result;
    }

    ScaleResult2D calculateScale2DAffine(const ToolTransformArgs &args,
                                         const QPointF &staticPointSrc,
                                         const QPointF &staticPointDst,
                                         const QPointF &movingPointSrc,
                                         const QPointF &movingPointDst)
    {
        return calculateScale2D(args, staticPointSrc, staticPointDst, movingPointSrc, movingPointDst);
    }

    ScaleResult1D calculateScaleX(const ToolTransformArgs &args,
                                  const QPointF &staticPointSrc,
                                  const QPointF &staticPointDst,
                                  const QPointF &movingPointSrc,
                                  const QPointF &movingPointDst)
    {
        warnNoGSL();

        ScaleResult1D result;
        result.scale = args.scaleX();
        result.transformedCenter = args.transformedCenter();
        return result;
    }

    ScaleResult1D calculateScaleY(const ToolTransformArgs &args,
                                  const QPointF &staticPointSrc,
                                  const QPointF &staticPointDst,
                                  const QPointF &movingPointSrc,
                                  const QPointF &movingPointDst)
    {
        warnNoGSL();

        ScaleResult1D result;
        result.scale = args.scaleY();
        result.transformedCenter = args.transformedCenter();
        return result;
    }
}

#endif /* HAVE_GSL */
