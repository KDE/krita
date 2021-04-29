/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisColorfulBrush.h"


KisColorfulBrush::KisColorfulBrush(const QString &filename)
    : KisScalingSizeBrush(filename)
{
}

#include <KoColorSpaceMaths.h>

QImage KisColorfulBrush::brushTipImage() const
{
    QImage image = KisBrush::brushTipImage();
    if (isImageType() && brushApplication() != IMAGESTAMP) {
        if (m_adjustmentMidPoint != 127 ||
            !qFuzzyIsNull(m_brightnessAdjustment) ||
            !qFuzzyIsNull(m_contrastAdjustment)) {

            const int half = KoColorSpaceMathsTraits<quint8>::halfValue;
            const int unit = KoColorSpaceMathsTraits<quint8>::unitValue;

            const qreal midX = m_adjustmentMidPoint;
            const qreal midY = m_brightnessAdjustment > 0 ?
                        KoColorSpaceMaths<qreal>::blend(unit, half, m_brightnessAdjustment) :
                        KoColorSpaceMaths<qreal>::blend(0, half, -m_brightnessAdjustment);

            qreal loA = 0.0;
            qreal hiA = 0.0;

            qreal loB = 0.0;
            qreal hiB = 255.0;

            if (!qFuzzyCompare(m_contrastAdjustment, 1.0)) {
                loA = midY / (1.0 - m_contrastAdjustment) / midX;
                hiA = (unit - midY) / (1.0 - m_contrastAdjustment) / (unit - midX);

                loB = midY - midX * loA;
                hiB = midY - midX * hiA;
            }

            for (int y = 0; y < image.height(); y++) {
                QRgb *pixel = reinterpret_cast<QRgb *>(image.scanLine(y));
                for (int x = 0; x < image.width(); x++) {
                    QRgb c = pixel[x];

                    int v = qGray(c);

                    if (v >= midX) {
                        v = qMin(unit, qRound(hiA * v + hiB));
                    } else {
                        v = qMax(0, qRound(loA * v + loB));
                    }

                    pixel[x] = qRgba(v, v, v, qAlpha(c));
                }
            }
        } else {
            for (int y = 0; y < image.height(); y++) {
                QRgb *pixel = reinterpret_cast<QRgb *>(image.scanLine(y));
                for (int x = 0; x < image.width(); x++) {
                    QRgb c = pixel[x];

                    int v = qGray(c);
                    pixel[x] = qRgba(v, v, v, qAlpha(c));
                }
            }
        }
    }
    return image;
}

void KisColorfulBrush::setAdjustmentMidPoint(quint8 value)
{
    if (m_adjustmentMidPoint != value) {
        m_adjustmentMidPoint = value;
        clearBrushPyramid();
    }
}

void KisColorfulBrush::setBrightnessAdjustment(qreal value)
{
    if (m_brightnessAdjustment != value) {
        m_brightnessAdjustment = value;
        clearBrushPyramid();
    }
}

void KisColorfulBrush::setContrastAdjustment(qreal value)
{
    if (m_contrastAdjustment != value) {
        m_contrastAdjustment = value;
        clearBrushPyramid();
    }
}

bool KisColorfulBrush::isImageType() const
{
    return brushType() == IMAGE || brushType() == PIPE_IMAGE;
}

quint8 KisColorfulBrush::adjustmentMidPoint() const
{
    return m_adjustmentMidPoint;
}

qreal KisColorfulBrush::brightnessAdjustment() const
{
    return m_brightnessAdjustment;
}

qreal KisColorfulBrush::contrastAdjustment() const
{
    return m_contrastAdjustment;
}

#include <QDomElement>

void KisColorfulBrush::toXML(QDomDocument& d, QDomElement& e) const
{
    // legacy setting, now 'brushApplication' is used instead
    e.setAttribute("ColorAsMask", QString::number((int)(brushApplication() != IMAGESTAMP)));

    e.setAttribute("AdjustmentMidPoint", QString::number(m_adjustmentMidPoint));
    e.setAttribute("BrightnessAdjustment", QString::number(m_brightnessAdjustment));
    e.setAttribute("ContrastAdjustment", QString::number(m_contrastAdjustment));
    e.setAttribute("AdjustmentVersion", QString::number(2));
    KisBrush::toXML(d, e);
}

void KisColorfulBrush::setHasColorAndTransparency(bool value)
{
    m_hasColorAndTransparency = value;
}

bool KisColorfulBrush::hasColorAndTransparency() const
{
    return m_hasColorAndTransparency;
}
