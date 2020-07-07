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

#include <KoColorSpaceRegistry.h>
#include <KoMixColorsOp.h>
#include <SvgMeshPatch.h>

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

    void fillPatch(const SvgMeshPatch *patch) {
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

        // check if color variation is acceptable and patch size is less than ~pixel width/heigh
        if ((cs->difference(c[0], c[1]) > threshold || cs->difference(c[1], c[2]) > threshold ||
             cs->difference(c[2], c[3]) > threshold || cs->difference(c[3], c[0]) > threshold) &&
            patch->size().width() > 1 && patch->size().height() > 1) {

            QVector<SvgMeshPatch*> patches;
            patch->subdivide(patches);

            for (const auto& p: patches) {
                fillPatch(p);
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

    QImage* patchImage() {
        return &m_patch;
    }

private:
    QImage m_patch;
    QPainter m_patchPainter;
};

#endif // KOMESHPATCHESRENDERER_H
