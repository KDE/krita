/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *  Copyright (c) 2020 Sharaf Zaman <sharafzaz121@gmail.com>
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
#ifndef KOMESHPATCHESRENDERER_H
#define KOMESHPATCHESRENDERER_H

#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QVector>
#include <QRectF>

#include <KoColorSpaceRegistry.h>
#include <KoMixColorsOp.h>
#include <SvgMeshArray.h>
#include <SvgMeshGradient.h>

struct KoMeshPatchesRenderer {
public:

    KoMeshPatchesRenderer()
    {}

    void configure(QRectF gradientgRect, const QTransform& painterTransform) {

        // NOTE: This is a necessary step to prevent loss of quality, because painterTransform is scaled.

        // we wish to scale the patch, but not translate, because patch should stay inside
        // the boundingRect
        QTransform painterTransformShifted = painterTransform *
            QTransform::fromTranslate(painterTransform.dx(), painterTransform.dy()).inverted();

        // boundingRect of the scaled version
        QRectF scaledGradientRect = painterTransformShifted.mapRect(gradientgRect);

        m_patch = QImage(scaledGradientRect.size().toSize(), QImage::Format_ARGB32);
        m_patch.fill(Qt::transparent);

        m_patchPainter.begin(&m_patch);
        m_patchPainter.setRenderHint(QPainter::Antialiasing);

        // this ensures that the patch renders inside the boundingRect
        m_patchPainter.translate(-scaledGradientRect.topLeft());

        // upscale the patch to the same scaling factor as the painterTransform
        m_patchPainter.setTransform(painterTransformShifted, true);
    }

    void fillPatch(const SvgMeshPatch *patch,
                   SvgMeshGradient::Shading type,
                   const SvgMeshArray *mesharray = nullptr,
                   const int row = -1,
                   const int col = -1) {
        KoPathShape *patchPath = patch->getPath();

        QColor color0 = patch->getStop(SvgMeshPatch::Top).color;
        QColor color1 = patch->getStop(SvgMeshPatch::Right).color;
        QColor color2 = patch->getStop(SvgMeshPatch::Bottom).color;
        QColor color3 = patch->getStop(SvgMeshPatch::Left).color;

        const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();

        quint8 c[4][4];
        cs->fromQColor(color0, c[0]);
        cs->fromQColor(color1, c[1]);
        cs->fromQColor(color2, c[2]);
        cs->fromQColor(color3, c[3]);

        const quint8 threshold = 0;

        // TODO: No, this only checks logical coordinates, we have to first transform and then recheck.
        // check if color variation is acceptable and patch size is less than ~pixel width/heigh
        if ((cs->difference(c[0], c[1]) > threshold || cs->difference(c[1], c[2]) > threshold ||
             cs->difference(c[2], c[3]) > threshold || cs->difference(c[3], c[0]) > threshold) &&
            patch->size().width() > 1 && patch->size().height() > 1) {

            QVector<SvgMeshPatch*> patches;
            QVector<QColor> colors;
            if (type == SvgMeshGradient::BICUBIC) {

                // it is a parent patch, so calculate coefficients aka alpha
                if (mesharray) {
                    m_patchRect = patch->boundingRect();
                    calculateAlpha(mesharray, row, col, patch);
                }

                colors = getColorsBicubic(patch);
            } else {
                colors = getColorsBilinear(patch);
            }

            patch->subdivide(patches, colors);

            for (const auto& p: patches) {
                fillPatch(p, type);
            }

            for (auto& p: patches) {
                delete p;
            }
        } else {
            quint8 mixed[4];
            cs->mixColorsOp()->mixColors(c[0], 4, mixed);

            QColor average;
            cs->toQColor(mixed, &average);

            QPen pen(average);
            m_patchPainter.setPen(pen);

            m_patchPainter.drawPath(patchPath->outline());

            QBrush brush(average);
            m_patchPainter.fillPath(patchPath->outline(), brush);
            m_patchPainter.drawPath(patchPath->outline());
        }
    }


    QVector<qreal> difference(const QVector<qreal>& v1, const QVector<qreal>& v2)
    {
        QVector<qreal> v3(4, 0);
        for (int i = 0; i < 4; ++i) {
            v3[i] = v1[i] - v2[i];
        }
        return v3;
    }


    QVector<qreal> multiply(const QVector<qreal>& v1, qreal n)
    {
        QVector<qreal> v3(4, 0);
        for (int i = 0; i < 4; ++i) {
            v3[i] = v1[i] * n;
        }
        return v3;
    }

    QVector<qreal> split(QColor c)
    {
        return {c.redF(), c.greenF(), c.blueF(), c.alphaF()};
    }

    QVector<qreal> secant(SvgMeshStop stop1, SvgMeshStop stop2)
    {
        qreal distance = QLineF(stop1.point, stop2.point).length();

        if (distance == 0.0) {  // NaN
            return {0.0, 0.0, 0.0, 0.0};
        }

        qreal c1[4] = {stop1.color.redF(), stop1.color.greenF(), stop1.color.blueF(), stop1.color.alphaF()};
        qreal c2[4] = {stop2.color.redF(), stop2.color.greenF(), stop2.color.blueF(), stop2.color.alphaF()};

        QVector<qreal> result(4, 0.0);

        for (int i = 0; i < 4; ++i) {
            result[i] = (c2[i] - c1[i]) / distance;
        }

        return result;
    }

    qreal getValue(const QVector<qreal>& alpha, const qreal x, const qreal y)
    {
        KIS_ASSERT(alpha.size() == 16);
        qreal result = 0;

        qreal xx  = x * x;
        qreal xxx = xx * x;
        qreal yy  = y * y;
        qreal yyy = yy * y;

        result += alpha[0];
        result += alpha[1] * x;
        result += alpha[2] * xx;
        result += alpha[3] * xxx;

        result += alpha[4] * y;
        result += alpha[5] * y * x;
        result += alpha[6] * y * xx;
        result += alpha[7] * y * xxx;

        result += alpha[ 8] * yy;
        result += alpha[ 9] * yy * x;
        result += alpha[10] * yy * xx;
        result += alpha[11] * yy * xxx;

        result += alpha[12] * yyy;
        result += alpha[13] * yyy * x;
        result += alpha[14] * yyy * xx;
        result += alpha[15] * yyy * xxx;

        return result;
    }

    QColor getColorUsingAlpha(const QVector<QVector<qreal>>& alpha, const qreal x, const qreal y)
    {
        qreal r = getValue(alpha[0], x, y);
        qreal g = getValue(alpha[1], x, y);
        qreal b = getValue(alpha[2], x, y);
        qreal a = getValue(alpha[3], x, y);

        // Clamp
        r = r > 1.0 ? 1.0 : r;
        g = g > 1.0 ? 1.0 : g;
        b = b > 1.0 ? 1.0 : b;
        a = a > 1.0 ? 1.0 : a;

        r = r < 0.0 ? 0.0 : r;
        g = g < 0.0 ? 0.0 : g;
        b = b < 0.0 ? 0.0 : b;
        a = a < 0.0 ? 0.0 : a;

        QColor result;
        result.setRgbF(r, g, b);
        result.setAlphaF(a);
        return result;
    }

    QVector<qreal> derivative(const SvgMeshStop& stop0,
                              const SvgMeshStop& stop1,
                              const SvgMeshStop& stop2)
    {
        QVector<qreal> delta0 = secant(stop0, stop1);
        QVector<qreal> delta1 = secant(stop1, stop2);

        QVector<qreal> result(4);

        for (int i = 0; i < 4; ++i) {
            qreal slope = (delta0[i] + delta1[i]) / 2;

            // if sign changes
            if ((delta0[i] * delta1[i]) < 0 && delta0[i] > 0) {
                slope = 0;
            } else if (abs(slope) > 3 * abs(delta0[i])) {
                slope = 3 * delta0[i];
            } else if (abs(slope) > 3 * abs(delta1[i])) {
                slope = 3 * delta1[i];
            }

            result.push_back(slope);
        }
        return result;
    }

    QVector<QVector<qreal>> derivative(const SvgMeshPatch* patch0,
                                       const SvgMeshPatch* patch1,
                                       const SvgMeshPatch* patch2)
    {
        SvgMeshStop f10 = patch0->getStop(SvgMeshPatch::Top);
        SvgMeshStop f20 = patch0->getStop(SvgMeshPatch::Left);

        SvgMeshStop f11 = patch1->getStop(SvgMeshPatch::Top);
        SvgMeshStop f12 = patch1->getStop(SvgMeshPatch::Right);
        SvgMeshStop f21 = patch1->getStop(SvgMeshPatch::Left);
        SvgMeshStop f22 = patch1->getStop(SvgMeshPatch::Bottom);

        SvgMeshStop f13 = patch2->getStop(SvgMeshPatch::Right);
        SvgMeshStop f23 = patch2->getStop(SvgMeshPatch::Bottom);

        QVector<qreal> d11 = derivative(f10, f11, f12);
        QVector<qreal> d12 = derivative(f11, f12, f13);
        QVector<qreal> d21 = derivative(f20, f21, f22);
        QVector<qreal> d22 = derivative(f21, f22, f23);

        return {d11, d12, d21, d22};
    }

    QVector<QVector<qreal>> derivativeEdge(const SvgMeshPatch* patch0,
                                            const SvgMeshPatch* patch1)
    {
        // FIXME shouldn't we move this to SvgMeshArray class?
        SvgMeshStop f00 = patch0->getStop(SvgMeshPatch::Top);
        SvgMeshStop f01 = patch0->getStop(SvgMeshPatch::Right);
        SvgMeshStop f10 = patch0->getStop(SvgMeshPatch::Left);
        SvgMeshStop f11 = patch0->getStop(SvgMeshPatch::Bottom);

        SvgMeshStop f02 = patch1->getStop(SvgMeshPatch::Right);
        SvgMeshStop f12 = patch1->getStop(SvgMeshPatch::Bottom);

        // TODO check otherwise, if results aren't good enough
        QVector<qreal> d00 = multiply(secant(f00, f01), 2) /* - del_0 */;
        QVector<qreal> d01 = derivative(f00, f01, f02);
        QVector<qreal> d10 = multiply(secant(f10, f11), 2) /* - del_0 */;
        QVector<qreal> d11 = derivative(f10, f11, f12);

        return {d00, d01, d10, d11};
    }

    QVector<qreal> getAlpha(const QVector<qreal>& X)
    {
        QVector<QVector<qreal>> A = {
            { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
            { 0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
            {-3, 3, 0, 0, -2,-1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
            { 2,-2, 0, 0,  1, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0},
            { 0, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0},
            { 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0},
            { 0, 0, 0, 0,  0, 0, 0, 0, -3, 3, 0, 0, -2,-1, 0, 0},
            { 0, 0, 0, 0,  0, 0, 0, 0,  2,-2, 0, 0,  1, 1, 0, 0},
            {-3, 0, 3, 0,  0, 0, 0, 0, -2, 0,-1, 0,  0, 0, 0, 0},
            { 0, 0, 0, 0, -3, 0, 3, 0,  0, 0, 0, 0, -2, 0,-1, 0},
            { 9,-9,-9, 9,  6, 3,-6,-3,  6,-6, 3,-3,  4, 2, 2, 1},
            {-6, 6, 6,-6, -3,-3, 3, 3, -4, 4,-2, 2, -2,-2,-1,-1},
            { 2, 0,-2, 0,  0, 0, 0, 0,  1, 0, 1, 0,  0, 0, 0, 0},
            { 0, 0, 0, 0,  2, 0,-2, 0,  0, 0, 0, 0,  1, 0, 1, 0},
            {-6, 6, 6,-6, -4,-2, 4, 2, -3, 3,-3, 3, -2,-1,-2,-1},
            { 4,-4,-4, 4,  2, 2,-2,-2,  2,-2, 2,-2,  1, 1, 1, 1}
            };


        QVector<qreal> alpha(16, 0);
        for (int i = 0; i < 16; ++i) {
            alpha[i] = 0;
            for (int j = 0; j < 16; ++j) {
                alpha[i] += A[i][j] * X[j];
            }
        }

        return alpha;
    }

    // TODO: Make this private
    void calculateAlpha(const SvgMeshArray* mesharray, const int row, const int col, const SvgMeshPatch* patch)
    {
        SvgMeshStop f00 = patch->getStop(SvgMeshPatch::Top);
        SvgMeshStop f01 = patch->getStop(SvgMeshPatch::Right);
        SvgMeshStop f10 = patch->getStop(SvgMeshPatch::Left);
        SvgMeshStop f11 = patch->getStop(SvgMeshPatch::Bottom);

        QVector<QVector<qreal>> dx(4, QVector<qreal>(4, 0));
        QVector<QVector<qreal>> dy(4, QVector<qreal>(4, 0));

        // dx
        if (!mesharray || mesharray->numColumns() < 2) {

            // NOTE: they're zero here: from trial and error
            dx[1] = secant(f00, f00);
            dx[0] = multiply(secant(f00, f01), 2) /* - del_0 */;
            dx[3] = secant(f10, f10);
            dx[2] = multiply(secant(f10, f11), 2) /* - del_0 */;

        } else if (col == 0) {
            // TODO this might or might not work, specs sheet is very vague about this...
            dx = derivativeEdge(patch, patch);

        } else if (col == mesharray->numColumns() - 1) {
            // Because of symmetry, we just reversed
            dx = derivativeEdge(mesharray->getPatch(row, col),
                                 mesharray->getPatch(row, col - 1));
        } else {
            dx = derivative(mesharray->getPatch(row, col - 1),
                            mesharray->getPatch(row, col),
                            mesharray->getPatch(row, col + 1));
        }

        // dy
        if (!mesharray || mesharray->numRows() < 2) {

            // NOTE: they're zero here: from trial and error
            dy[1] = secant(f00, f00);
            dy[0] = multiply(secant(f00, f10), 2) /* - del_0 */;
            dy[3] = secant(f01, f01);
            dy[2] = multiply(secant(f01, f11), 2) /* - del_0 */;

        } else if (row == 0) {
            // TODO this might or might not work, specs sheet is very vague about this...
            dy = derivativeEdge(patch, patch);

        } else if (row == mesharray->numRows() - 1) {
            // Because of symmetry, we just reversed
            dy = derivativeEdge(mesharray->getPatch(row, col),
                                 mesharray->getPatch(row - 1, col));
        } else {
            dy = derivative(mesharray->getPatch(row - 1, col),
                            mesharray->getPatch(row, col),
                            mesharray->getPatch(row + 1, col));
        }

        // once we have the derivatives we find the colors at five points

        QVector<QVector<qreal>> c = {split(f00.color), split(f01.color), split(f10.color), split(f11.color)};
        QVector<QVector<qreal>> alpha(4, QVector<qreal>(16, 0));

        qreal width01 = QLineF(f00.point, f01.point).length();
        qreal width23 = QLineF(f10.point, f11.point).length();

        qreal height01 = QLineF(f00.point, f10.point).length();
        qreal height23 = QLineF(f01.point, f11.point).length();

        for (int i = 0; i < 4; ++i) {
            QVector<qreal> X {
                c[0][i],
                c[1][i],
                c[2][i],
                c[3][i],

                // TODO: Use spacing because normalized...
                dx[0][i], // * width01,
                dx[1][i], // * width01,
                dx[2][i], // * width23,
                dx[3][i], // * width23,

                dy[0][i], // * height01,
                dy[1][i], // * height23,
                dy[2][i], // * height01,
                dy[3][i], // * height23,

                0,
                0,
                0,
                0};

            alpha[i] = getAlpha(X);
        }

        m_alpha = alpha;
    }

    QVector<QColor> getColorsBicubic(const SvgMeshPatch* patch)
    {
        QRectF patchRect = patch->boundingRect();

        patchRect.translate(-m_patchRect.topLeft());

        QPointF topLeft = patchRect.topLeft();
        QPointF bottomRight = patchRect.bottomRight();

        qreal hw = (patch->pointAt(0.5, 0.5).x() - m_patchRect.topLeft().x()) / m_patchRect.width();
        qreal hh = (patch->pointAt(0.5, 0.5).y() - m_patchRect.topLeft().y()) / m_patchRect.height();

        qreal nlx = topLeft.x() / m_patchRect.width();
        qreal nly = topLeft.y() / m_patchRect.height();

        qreal nrx = bottomRight.x() / m_patchRect.width();
        qreal nry = bottomRight.y() / m_patchRect.height();

        QVector<QColor> result(5);
        result[0] = getColorUsingAlpha(m_alpha, hw,  nly);
        result[1] = getColorUsingAlpha(m_alpha, nrx, hh);
        result[2] = getColorUsingAlpha(m_alpha, hw,  nry);
        result[3] = getColorUsingAlpha(m_alpha, nlx, hh);
        result[4] = getColorUsingAlpha(m_alpha, hw,  hh);

        return result;
    }

    QColor midPointColor(QColor first, QColor second)
    {
        int r = (first.red() + second.red()) / 2;
        int g = (first.green() + second.green()) / 2;
        int b = (first.blue() + second.blue()) / 2;
        int a = (first.alpha() + second.alpha()) / 2;

        return QColor(r, g, b, a);
    }

    QVector<QColor> getColorsBilinear(const SvgMeshPatch* patch)
    {
        QColor c1     = patch->getStop(SvgMeshPatch::Top).color;
        QColor c2     = patch->getStop(SvgMeshPatch::Right).color;
        QColor c3     = patch->getStop(SvgMeshPatch::Bottom).color;
        QColor c4     = patch->getStop(SvgMeshPatch::Left).color;

        QVector<QColor> result(5);
        result[0] = midPointColor(c1, c2);
        result[1] = midPointColor(c2, c3);
        result[2] = midPointColor(c3, c4);
        result[3] = midPointColor(c4, c1);
        result[4] = midPointColor(midPointColor(result[0], result[2]), midPointColor(result[1], result[3]));

        return result;
    }

    QImage* patchImage() {
        return &m_patch;
    }

private:
    QImage m_patch;
    QPainter m_patchPainter;
    // TODO: make them local
    QVector<QVector<qreal>> m_alpha;
    QRectF m_patchRect;
};

#endif // KOMESHPATCHESRENDERER_H
