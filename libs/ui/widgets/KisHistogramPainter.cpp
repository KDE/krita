/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <cmath>
#include <algorithm>

#include <QHash>
#include <QPair>
#include <QPolygonF>

#include <kis_histogram.h>
#include <KoColorSpace.h>
#include <kis_algebra_2d.h>
#include <KoColor.h>
#include <KoColorModelStandardIds.h>

#include "KisHistogramPainter.h"

struct HistogramShapeInfo
{
    QPolygonF linearHistogram;
    QPolygonF logarithmicHistogram;
    quint32 highest;
    qreal linearBestCutOffHeight;
    qreal logarithmicBestCutOffHeight;
    QColor color;
    QPainter::CompositionMode compositionMode;
};

class KisHistogramPainter::Private
{
public:
    static constexpr qreal maximumOrientationDeviation = M_PI / 16.0;
    static constexpr int maximumNumberOfSimplifiedPoints = 3;
    static constexpr qreal histogramHeightFactor = 0.9;
    static constexpr qreal maximumNeighborWeight = 0.33;
    static constexpr qreal percentageForPercentile = 0.98;

    QHash<int, HistogramShapeInfo> histogramChannelShapeInfo;
    QVector<int> channelsToPaint;
    QColor defaultColor;
    qreal scale{1.0};
    bool isLogarithmic{false};

    QImage paintChannels(const QSize &imageSize, const QVector<int> &channels = {}, bool logarithmic = false);

    static qreal orientationDeviation(const QPointF &A, const QPointF &B, const QPointF &C);
    static QPair<QPolygonF, QPolygonF> computeHistogramShape(KisHistogram *histogram,
                                                             int channel,
                                                             quint32 highest);
    static qreal bestCutOffHeight(QPolygonF polygon);
    static void smoothHistogramShape(QPolygonF &polygon);
    static void simplifyHistogramShape(QPolygonF &polygon);
    static QPair<QColor, QPainter::CompositionMode> computeChannelPaintigInfo(const KoColorSpace *colorSpace,
                                                                              int channel);
    static void paintHistogramShape(QImage &image,
                                    const QPolygonF &polygon,
                                    qreal scale,
                                    const QColor &color,
                                    QPainter::CompositionMode compositionMode);
};

qreal KisHistogramPainter::Private::orientationDeviation(const QPointF &A,
                                                         const QPointF &B,
                                                         const QPointF &C)
{
    const QPointF AB = B - A;
    const QPointF BC = C - B;
    const qreal angle =KisAlgebra2D::angleBetweenVectors(AB, BC);
    return angle < -M_PI ? angle + 2.0 * M_PI : (angle > M_PI ? angle - 2.0 * M_PI : angle);
}

QPair<QPolygonF, QPolygonF>
KisHistogramPainter::Private::computeHistogramShape(KisHistogram *histogram,
                                                    int channel,
                                                    quint32 highest)
{
    Q_ASSERT(histogram);
    Q_ASSERT(channel >= 0);

    QPolygonF linearHistogramShape, logarithmicHistogramShape;
    const int bins = histogram->producer()->numberOfBins();
    const qreal heightfactor = 1.0 / static_cast<qreal>(highest);
    const qreal logHeightFactor = 1.0 / std::log(static_cast<qreal>(highest + 1));
    const qreal widthFactor = 1.0 / static_cast<qreal>(bins);

    // Add extra points at the beginning and end so that the shape is a bit
    // hidden on the bottom (hides the pen line on the bottom when painting)
    linearHistogramShape.append(QPointF(0.0, -0.1));
    linearHistogramShape.append(QPointF(0.0, 0.0));
    logarithmicHistogramShape.append(QPointF(0.0, -0.1));
    logarithmicHistogramShape.append(QPointF(0.0, 0.0));

    for (int i = 0; i < bins; ++i) {
        const QPointF p = QPointF((static_cast<qreal>(i) + 0.5) * widthFactor,
                                  static_cast<qreal>(histogram->getValue(i)) * heightfactor);
        const QPointF logP = QPointF(p.x(), std::log(static_cast<qreal>(histogram->getValue(i) + 1)) * logHeightFactor);
        linearHistogramShape.append(p);
        logarithmicHistogramShape.append(logP);
    }

    linearHistogramShape.append(QPointF(1.0, 0.0));
    linearHistogramShape.append(QPointF(1.0, -0.1));
    logarithmicHistogramShape.append(QPointF(1.0, 0.0));
    logarithmicHistogramShape.append(QPointF(1.0, -0.1));

    return {linearHistogramShape, logarithmicHistogramShape};
}

qreal KisHistogramPainter::Private::bestCutOffHeight(QPolygonF polygon)
{
    const int binOfPercentile = static_cast<int>(std::round((1.0 - percentageForPercentile) * (polygon.size() - 4 - 1)));
    std::nth_element(polygon.begin() + 2,
                     polygon.begin() + 2 + binOfPercentile,
                     polygon.end() - 2,
                     [](const QPointF &p1, const QPointF &p2) ->bool
                     {
                         return p1.y() > p2.y();
                     });
    const qreal percentile = polygon[binOfPercentile + 2].y();
    return qFuzzyIsNull(percentile) ? 1.0 : percentile;
}

void KisHistogramPainter::Private::smoothHistogramShape(QPolygonF &polygon)
{
    if (polygon.size() < 5) {
        return;
    }

    for (int i = 2; i < polygon.size() - 2; ++i) {
        const qreal leftValue = polygon[i - 1].y();
        const qreal centerValue = polygon[i].y();
        const qreal rightValue = polygon[i + 1].y();
        const qreal leftDelta = std::abs(centerValue - leftValue);
        const qreal rightDelta = std::abs(centerValue - rightValue);
        const qreal leftWeight = maximumNeighborWeight * std::exp(-pow2(10.0 * leftDelta));
        const qreal rightWeight = maximumNeighborWeight * std::exp(-pow2(10.0 * rightDelta));
        const qreal centerWeight = 1.0 - leftWeight - rightWeight;
        polygon[i].setY(leftValue * leftWeight + centerValue * centerWeight + rightValue * rightWeight);
    }
}

void KisHistogramPainter::Private::simplifyHistogramShape(QPolygonF &polygon)
{
    if (polygon.size() < 5) {
        return;
    }

    qreal accumulatedOrientationDeviation = 0.0;
    int numberOfSimplifiedPoints = 0;

    for (int i = polygon.size() - 3; i > 1; --i) {
        accumulatedOrientationDeviation += orientationDeviation(polygon[i + 1], polygon[i], polygon[i - 1]);
        ++numberOfSimplifiedPoints;
        if (std::abs(accumulatedOrientationDeviation) > maximumOrientationDeviation ||
            numberOfSimplifiedPoints > maximumNumberOfSimplifiedPoints) {
            accumulatedOrientationDeviation = 0.0;
            numberOfSimplifiedPoints = 0;
        } else {
            polygon.remove(i);
        }
    }
}

QPair<QColor, QPainter::CompositionMode>
KisHistogramPainter::Private::computeChannelPaintigInfo(const KoColorSpace *colorSpace,
                                                        int channel)
{
    Q_ASSERT(colorSpace);
    Q_ASSERT(channel >= 0);

    QColor color;
    QPainter::CompositionMode compositionMode = QPainter::CompositionMode_SourceOver;

    if (colorSpace->colorModelId() == RGBAColorModelID) {
        if (channel == 0) {
            color = Qt::blue;
        } else if (channel == 1) {
            color = Qt::green;
        } else if (channel == 2) {
            color = Qt::red;
        }
        compositionMode = QPainter::CompositionMode_Plus;
    } else if (colorSpace->colorModelId() == XYZAColorModelID) {
        if (channel == 0) {
            color = Qt::red;
        } else if (channel == 1) {
            color = Qt::green;
        } else if (channel == 2) {
            color = Qt::blue;
        }
        compositionMode = QPainter::CompositionMode_Plus;
    } else if (colorSpace->colorModelId() == CMYKAColorModelID) {
        if (channel == 0) {
            color = Qt::cyan;
        } else if (channel == 1) {
            color = Qt::magenta;
        } else if (channel == 2) {
            color = Qt::yellow;
        } else if (channel == 3) {
            color = Qt::black;
        }
        if (channel != 4) {
            color = KoColor(color, KoColorSpaceRegistry::instance()->rgb8())
                    .convertedTo(colorSpace,
                                KoColorConversionTransformation::IntentSaturation,
                                KoColorConversionTransformation::Empty)
                    .toQColor();
            compositionMode = QPainter::CompositionMode_Multiply;
        }
    }

    return {color, compositionMode};
}

void KisHistogramPainter::Private::paintHistogramShape(QImage &image,
                                                       const QPolygonF &polygon,
                                                       qreal scale,
                                                       const QColor &color,
                                                       QPainter::CompositionMode compositionMode)
{
    const qreal w = static_cast<qreal>(image.width());
    const qreal h = static_cast<qreal>(image.height());
    const qreal maxH = h * histogramHeightFactor;

    QPainter p(&image);
    p.setRenderHint(QPainter::Antialiasing);
    p.translate(0.0, h);
    p.scale(w, -scale * maxH);
    QPen pen(color, 2);
    pen.setCosmetic(true);
    QBrush brush(QColor(color.red(), color.green(), color.blue(), 200));
    p.setPen(pen);
    p.setBrush(brush);
    p.setCompositionMode(compositionMode);
    p.drawPolygon(polygon);
}

QImage KisHistogramPainter::Private::paintChannels(const QSize &imageSize,
                                                   const QVector<int> &channels,
                                                   bool logarithmic)
{
    QImage image(imageSize, QImage::Format_ARGB32);
    image.fill(0);

    const int nChannels = histogramChannelShapeInfo.size();

    if (nChannels == 0 || channels.size() == 0) {
        return image;
    }

    qreal overallHighest = 0.0;
    for (int channel : channels) {
        if (!histogramChannelShapeInfo.contains(channel)) {
            continue;
        }

        const qreal channelHighest = static_cast<qreal>(histogramChannelShapeInfo[channel].highest);
        if (channelHighest > overallHighest) {
            overallHighest = channelHighest;
        }
    }

    for (int channel : channels) {
        if (!histogramChannelShapeInfo.contains(channel)) {
            continue;
        }

        const HistogramShapeInfo &info = histogramChannelShapeInfo[channel];
        Private::paintHistogramShape(
            image,
            logarithmic ? info.logarithmicHistogram : info.linearHistogram,
            logarithmic
                ? scale * std::log(info.highest + 1.0) / std::log(overallHighest + 1.0)
                : scale * info.highest / overallHighest,
            info.color.isValid() ? info.color : defaultColor,
            info.compositionMode
        );
    }

    return image;
}

KisHistogramPainter::KisHistogramPainter()
    : m_d(new Private)
{
    m_d->defaultColor = Qt::gray;
}

KisHistogramPainter::KisHistogramPainter(const KisHistogramPainter &other)
    : m_d(new Private(*other.m_d))
{}

KisHistogramPainter::KisHistogramPainter(KisHistogramPainter && other)
    : m_d(other.m_d.take())
{}

KisHistogramPainter::~KisHistogramPainter()
{}

void KisHistogramPainter::setup(KisHistogram *histogram, const KoColorSpace *colorSpace, QVector<int> channels)
{
    Q_ASSERT(histogram);
    Q_ASSERT(colorSpace);

    const int nChannels = static_cast<int>(colorSpace->channelCount());
    
    if (channels.size() == 0) {
        for (int i = 0; i < nChannels; ++i) {
            channels.append(i);
        }
    }

    m_d->histogramChannelShapeInfo.clear();

    for (int channel : channels) {
        if (channel < 0 || channel >= nChannels || m_d->histogramChannelShapeInfo.contains(channel)) {
            continue;
        }
        histogram->setChannel(channel);
        const quint32 highest = histogram->calculations().getHighest();
        QPair<QPolygonF, QPolygonF> shapes = Private::computeHistogramShape(histogram, channel, highest);
        const QPair<QColor, QPainter::CompositionMode> channelPaintingInfo =
            Private::computeChannelPaintigInfo(colorSpace, channel);
        const qreal linearBestCutOffHeight = Private::bestCutOffHeight(shapes.first);
        const qreal logarithmicBestCutOffHeight = Private::bestCutOffHeight(shapes.second);

        Private::smoothHistogramShape(shapes.first);
        Private::smoothHistogramShape(shapes.second);
        Private::simplifyHistogramShape(shapes.first);
        Private::simplifyHistogramShape(shapes.second);

        m_d->histogramChannelShapeInfo.insert(
            channel,
            {
                shapes.first,
                shapes.second,
                highest,
                linearBestCutOffHeight,
                logarithmicBestCutOffHeight,
                channelPaintingInfo.first,
                channelPaintingInfo.second
            }
        );
    }
}

QImage KisHistogramPainter::paint(const QSize &imageSize)
{
    return m_d->paintChannels(imageSize, m_d->channelsToPaint, m_d->isLogarithmic);
}

QImage KisHistogramPainter::paint(int w, int h)
{
    return paint(QSize(w, h));
}

void KisHistogramPainter::paint(QPainter &painter, const QRect &rect)
{
    const QImage image = m_d->paintChannels(rect.size(), m_d->channelsToPaint, m_d->isLogarithmic);
    painter.drawImage(rect.topLeft(), image);
}

int KisHistogramPainter::totalNumberOfAvailableChannels() const
{
    return m_d->histogramChannelShapeInfo.size();
}

QList<int> KisHistogramPainter::availableChannels() const
{
    return m_d->histogramChannelShapeInfo.keys();
}

const QVector<int>& KisHistogramPainter::channels() const
{
    return m_d->channelsToPaint;
}

void KisHistogramPainter::setChannel(int channel)
{
    setChannels({channel});
}

void KisHistogramPainter::setChannels(const QVector<int> &channels)
{
    m_d->channelsToPaint = channels;
}

QColor KisHistogramPainter::defaultColor() const
{
    return m_d->defaultColor;
}

void KisHistogramPainter::setDefaultColor(const QColor &newDefaultColor)
{
    m_d->defaultColor = newDefaultColor;
}

qreal KisHistogramPainter::scale() const
{
    return m_d->scale;
}

void KisHistogramPainter::setScale(qreal newScale)
{
    m_d->scale = newScale;
}

void KisHistogramPainter::setScaleToFit()
{
    setScale(1.0);
}

void KisHistogramPainter::setScaleToCutLongPeaks()
{
    qreal overallHighest = 0.0;
    qreal fittedChannelHighest = 0.0;
    qreal bestCutOffHeight = 0.0;
    for (int channel : m_d->channelsToPaint) {
        if (!m_d->histogramChannelShapeInfo.contains(channel)) {
            continue;
        }

        const qreal channelBestCutOffHeight =
            isLogarithmic()
            ? m_d->histogramChannelShapeInfo[channel].logarithmicBestCutOffHeight
            : m_d->histogramChannelShapeInfo[channel].linearBestCutOffHeight;
        const qreal channelHighest = static_cast<qreal>(m_d->histogramChannelShapeInfo[channel].highest);

        if (channelBestCutOffHeight * channelHighest > bestCutOffHeight * fittedChannelHighest) {
            bestCutOffHeight = channelBestCutOffHeight;
            fittedChannelHighest = channelHighest;
        }

        if (channelHighest > overallHighest) {
            overallHighest = channelHighest;
        }
    }
    const qreal overallBestCutOffHeight = bestCutOffHeight * fittedChannelHighest / overallHighest;
    if (overallBestCutOffHeight < 0.8) {
        setScale(1.0 / overallBestCutOffHeight);
    } else {
        setScale(1.0);
    }
}

bool KisHistogramPainter::isLogarithmic() const
{
    return m_d->isLogarithmic;
}

void KisHistogramPainter::setLogarithmic(bool logarithmic)
{
    m_d->isLogarithmic = logarithmic;
}
