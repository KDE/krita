/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoClipMaskPainter.h"

#include <QPainter>
#include <QPainterPath>
#include <QRectF>

#include "kis_assert.h"

struct Q_DECL_HIDDEN KoClipMaskPainter::Private
{
    QPainter *globalPainter;

    QImage shapeImage;
    QImage maskImage;

    QPainter shapePainter;
    QPainter maskPainter;

    QRect alignedGlobalClipRect;
};

KoClipMaskPainter::KoClipMaskPainter(QPainter *painter, const QRectF &globalClipRect)
    : m_d(new Private)
{
    m_d->globalPainter = painter;
    m_d->alignedGlobalClipRect = globalClipRect.toAlignedRect();

    m_d->shapeImage = QImage(m_d->alignedGlobalClipRect.size(), QImage::Format_ARGB32);
    m_d->maskImage = QImage(m_d->alignedGlobalClipRect.size(), QImage::Format_ARGB32);

    m_d->shapeImage.fill(0);
    m_d->maskImage.fill(0);

    QTransform moveToBufferTransform =
        QTransform::fromTranslate(-m_d->alignedGlobalClipRect.x(),
                                  -m_d->alignedGlobalClipRect.y());

    m_d->shapePainter.begin(&m_d->shapeImage);
    m_d->shapePainter.setTransform(moveToBufferTransform);
    m_d->shapePainter.setTransform(painter->transform(), true);
    if (painter->hasClipping()) {
        m_d->shapePainter.setClipPath(painter->clipPath());
    }
    m_d->shapePainter.setOpacity(painter->opacity());
    m_d->shapePainter.setBrush(painter->brush());
    m_d->shapePainter.setPen(painter->pen());

    m_d->maskPainter.begin(&m_d->maskImage);
    m_d->maskPainter.setTransform(moveToBufferTransform);
    m_d->maskPainter.setTransform(painter->transform(), true);
    if (painter->hasClipping()) {
        m_d->maskPainter.setClipPath(painter->clipPath());
    }
    m_d->maskPainter.setOpacity(painter->opacity());
    m_d->maskPainter.setBrush(painter->brush());
    m_d->maskPainter.setPen(painter->pen());
}

KoClipMaskPainter::~KoClipMaskPainter()
{
}

QPainter *KoClipMaskPainter::shapePainter()
{
    return &m_d->shapePainter;
}

QPainter *KoClipMaskPainter::maskPainter()
{
    return &m_d->maskPainter;
}

void KoClipMaskPainter::renderOnGlobalPainter()
{
    KIS_ASSERT_RECOVER_RETURN(m_d->maskImage.size() == m_d->shapeImage.size());

    for (int y = 0; y < m_d->maskImage.height(); y++) {
        QRgb *shapeData = reinterpret_cast<QRgb*>(m_d->shapeImage.scanLine(y));
        QRgb *maskData = reinterpret_cast<QRgb*>(m_d->maskImage.scanLine(y));

        for (int x = 0; x < m_d->maskImage.width(); x++) {

            const qreal normCoeff = 1.0 / 255.0 * 255.0;

            qreal maskValue = qreal(qAlpha(*maskData)) *
                (0.2125 * qRed(*maskData) +
                 0.7154 * qGreen(*maskData) +
                 0.0721 * qBlue(*maskData));

            int alpha = qRound(maskValue * qAlpha(*shapeData) * normCoeff);

            *shapeData = (alpha << 24) | (*shapeData & 0x00ffffff);

            shapeData++;
            maskData++;
        }
    }

    KIS_ASSERT_RECOVER_RETURN(m_d->shapeImage.size() == m_d->alignedGlobalClipRect.size());
    QPainterPath globalClipPath;

    if (m_d->globalPainter->hasClipping()) {
        globalClipPath = m_d->globalPainter->transform().map(m_d->globalPainter->clipPath());
    }

    m_d->globalPainter->save();

    m_d->globalPainter->setTransform(QTransform());

    if (!globalClipPath.isEmpty()) {
        m_d->globalPainter->setClipPath(globalClipPath);
    }

    m_d->globalPainter->drawImage(m_d->alignedGlobalClipRect.topLeft(), m_d->shapeImage);
    m_d->globalPainter->restore();
}

