/*
 *  SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoClipMaskApplicatorBase.h"

void KoClipMaskApplicatorBase::fallbackLuminanceMask(quint8 *pixels, quint8 *maskPixels, const int nPixels) const{
    const quint32 colorChannelsMask = 0x00FFFFFF;
    const float redLum = 0.2125f;
    const float greenLum = 0.7154f;
    const float blueLum = 0.0721f;
    const float normCoeff = 1.0f / 255.0f;

    const QRgb *mP = reinterpret_cast<const QRgb*>(maskPixels);
    QRgb *sP = reinterpret_cast<QRgb*>(pixels);

    for (int i = 0; i < nPixels; i++) {
        const QRgb mask = *mP;
        const QRgb shape = *sP;

        const float maskValue = qAlpha(mask) * (redLum * qRed(mask) + greenLum * qGreen(mask) + blueLum * qBlue(mask)) * normCoeff;

        const quint8 alpha = OptiRound<xsimd::generic, quint8>::roundScalar(maskValue * float(qAlpha(shape) * normCoeff));

        *sP = (alpha << 24) | (shape & colorChannelsMask);

        sP++;
        mP++;
    }
}
