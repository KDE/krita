/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KOBAKEDSHAPERENDERER_H
#define KOBAKEDSHAPERENDERER_H

#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QTransform>

#include <kis_debug.h>
#include <kis_algebra_2d.h>


struct KoBakedShapeRenderer {
    KoBakedShapeRenderer(const QPainterPath &dstShapeOutline, const QTransform &dstShapeTransform,
                       const QTransform &bakedTransform,
                       const QRectF &referenceRect,
                       bool contentIsObb, const QRectF &bakedShapeBoundingRect,
                       bool referenceIsObb,
                       const QTransform &patternTransform)
        : m_dstShapeOutline(dstShapeOutline),
          m_dstShapeTransform(dstShapeTransform),
          m_contentIsObb(contentIsObb),
          m_patternTransform(patternTransform)
    {
        KIS_SAFE_ASSERT_RECOVER_NOOP(!contentIsObb || !bakedShapeBoundingRect.isEmpty());

        const QRectF dstShapeBoundingRect = dstShapeOutline.boundingRect();

        QTransform relativeToBakedShape;

        if (referenceIsObb || contentIsObb) {
            m_relativeToShape = KisAlgebra2D::mapToRect(dstShapeBoundingRect);
            relativeToBakedShape = KisAlgebra2D::mapToRect(bakedShapeBoundingRect);
        }


        m_referenceRectUser =
            referenceIsObb ?
            m_relativeToShape.mapRect(referenceRect).toAlignedRect() :
            referenceRect.toAlignedRect();

        m_patch = QImage(m_referenceRectUser.size(), QImage::Format_ARGB32);
        m_patch.fill(0);
        m_patchPainter.begin(&m_patch);

        m_patchPainter.translate(-m_referenceRectUser.topLeft());
        m_patchPainter.setClipRect(m_referenceRectUser);

        if (contentIsObb) {
            m_patchPainter.setTransform(m_relativeToShape, true);
            m_patchPainter.setTransform(relativeToBakedShape.inverted(), true);
        }

        m_patchPainter.setTransform(bakedTransform.inverted(), true);
    }

    QPainter* bakeShapePainter() {
        return &m_patchPainter;
    }

    void renderShape(QPainter &painter) {
        painter.save();

        painter.setTransform(m_dstShapeTransform, true);
        painter.setClipPath(m_dstShapeOutline);

        QTransform brushTransform;

        QPointF patternOffset = m_referenceRectUser.topLeft();

        brushTransform =
            brushTransform *
            QTransform::fromTranslate(patternOffset.x(), patternOffset.y());

        if (m_contentIsObb) {
            brushTransform = brushTransform * m_relativeToShape.inverted();
        }

        brushTransform = brushTransform * m_patternTransform;

        if (m_contentIsObb) {
            brushTransform = brushTransform * m_relativeToShape;
        }

        QBrush brush(m_patch);
        brush.setTransform(brushTransform);

        painter.setBrush(brush);
        painter.drawPath(m_dstShapeOutline);

        painter.restore();
    }

    QImage patchImage() const {
        return m_patch;
    }


private:
    QPainterPath m_dstShapeOutline;
    QTransform m_dstShapeTransform;

    bool m_contentIsObb;
    const QTransform &m_patternTransform;

    QImage m_patch;
    QPainter m_patchPainter;

    QTransform m_relativeToShape;
    QRect m_referenceRectUser;
};

#endif // KOBAKEDSHAPERENDERER_H
