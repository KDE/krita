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
        : imageData(data),
        numColors(numberOfColors),
        parent(parentObj)
    {}

    void run();
    QList<KoColor> extractColors();
    QList<QRgb> getColors();

private:
    QImage imageData;
    int numColors;
    KisCommonColors* parent;
};

#endif // KIS_COMMON_COLORS_RECALCULATION_RUNNER_H
