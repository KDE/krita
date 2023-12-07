/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoClipMaskPainter.h"

#include <QPainter>
#include <QPainterPath>
#include <QRectF>
#include <KoStreamedMath.h>

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

    if (!m_d->alignedGlobalClipRect.isValid()) {
        m_d->alignedGlobalClipRect = QRect();
    }
    m_d->shapeImage = QImage(m_d->alignedGlobalClipRect.size(), QImage::Format_ARGB32);
    m_d->maskImage = QImage(m_d->alignedGlobalClipRect.size(), QImage::Format_ARGB32);

    QTransform moveToBufferTransform =
        QTransform::fromTranslate(-m_d->alignedGlobalClipRect.x(),
                                  -m_d->alignedGlobalClipRect.y());

    m_d->shapePainter.begin(&m_d->shapeImage);

    m_d->shapePainter.save();
    m_d->shapePainter.setCompositionMode(QPainter::CompositionMode_Source);
    m_d->shapePainter.fillRect(QRect(QPoint(), m_d->alignedGlobalClipRect.size()), Qt::transparent);
    m_d->shapePainter.restore();

    m_d->shapePainter.setTransform(moveToBufferTransform);
    m_d->shapePainter.setTransform(painter->transform(), true);
    if (painter->hasClipping()) {
        m_d->shapePainter.setClipPath(painter->clipPath());
    }
    m_d->shapePainter.setOpacity(painter->opacity());
    m_d->shapePainter.setBrush(painter->brush());
    m_d->shapePainter.setPen(painter->pen());

    m_d->maskPainter.begin(&m_d->maskImage);

    m_d->maskPainter.save();
    m_d->maskPainter.setCompositionMode(QPainter::CompositionMode_Source);
    m_d->maskPainter.fillRect(QRect(QPoint(), m_d->alignedGlobalClipRect.size()), Qt::transparent);
    m_d->maskPainter.restore();

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

using uint_v = typename KoStreamedMath<xsimd::current_arch>::uint_v;
using float_v = typename KoStreamedMath<xsimd::current_arch>::float_v;


void KoClipMaskPainter::renderOnGlobalPainter()
{
    KIS_ASSERT_RECOVER_RETURN(m_d->maskImage.size() == m_d->shapeImage.size());


    const int nPixels = m_d->maskImage.height() * m_d->maskImage.width();
    const int block = nPixels / static_cast<int>(float_v::size);
    const int block2 = nPixels % static_cast<int>(float_v::size);
    const int vectorPixelStride = 4 * static_cast<int>(float_v::size);

    const uint_v mask(0xFF);
    const quint32 colorChannelsMask = 0x00FFFFFF;
    const float redLum = 0.2125f;
    const float greenLum = 0.7154f;
    const float blueLum = 0.0721f;

    quint8 *pixels = m_d->shapeImage.bits();
    quint8 *maskPixels = m_d->maskImage.bits();

    for (int i = 0; i < block; i++) {
        auto shapeData = uint_v::load_unaligned(reinterpret_cast<const quint32 *>(pixels));
        const auto maskData = uint_v::load_unaligned(reinterpret_cast<const quint32 *>(maskPixels));

        const float_v maskAlpha = xsimd::to_float(xsimd::bitwise_cast_compat<int>((maskData >> 24) & mask));
        const float_v maskRed   = xsimd::to_float(xsimd::bitwise_cast_compat<int>((maskData >> 16) & mask));
        const float_v maskGreen = xsimd::to_float(xsimd::bitwise_cast_compat<int>((maskData >> 8) & mask));
        const float_v maskBlue = xsimd::to_float(xsimd::bitwise_cast_compat<int>((maskData) & mask));
        const float_v maskValue = maskAlpha * ((redLum * maskRed) + (greenLum * maskGreen) + (blueLum * maskBlue));

        const auto pixelAlpha = xsimd::to_float(xsimd::bitwise_cast_compat<int>(shapeData >> 24U)) * maskValue;
        const uint_v pixelAlpha_i = xsimd::bitwise_cast_compat<unsigned int>(xsimd::nearbyint_as_int(pixelAlpha));
        shapeData = (shapeData & colorChannelsMask) | (pixelAlpha_i << 24);

        shapeData.store_unaligned(reinterpret_cast<typename uint_v::value_type *>(pixels));

        pixels += vectorPixelStride;
        maskPixels += vectorPixelStride;
    }

    const float normCoeff = 1.0f / 255.0f;

    for (int i = 0; i < block2; i++) {
        const QRgb mask = *maskPixels;
        const QRgb shape = *pixels;

        const float maskValue = qAlpha(mask) * (redLum* qRed(mask) + greenLum * qGreen(mask) + blueLum * qBlue(mask)) * normCoeff;

        const QRgb alpha = static_cast<QRgb>(qRound(maskValue * (qAlpha(shape) * normCoeff)));

        *pixels = (alpha << 24) | (shape & colorChannelsMask);

        pixels++;
        maskPixels++;
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

