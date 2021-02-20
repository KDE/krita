/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef KIS_COMMON_COLORS_RECALCULATION_RUNNER_H
#define KIS_COMMON_COLORS_RECALCULATION_RUNNER_H

#include <QRunnable>
#include <QColor>
#include <QImage>

class KoColor;
class KisCommonColors;


class KisCommonColorsRecalculationRunner : public QRunnable
{
public:
    KisCommonColorsRecalculationRunner(QImage data, int numberOfColors, KisCommonColors* parentObj)
        : m_imageData(data)
        , m_numColors(numberOfColors)
        , m_commonColors(parentObj)
    {}

    void run() override;
    QList<KoColor> extractColors();
    QList<QRgb> getColors();

private:
    QImage m_imageData;
    int m_numColors;
    KisCommonColors* m_commonColors;
};

#endif // KIS_COMMON_COLORS_RECALCULATION_RUNNER_H
