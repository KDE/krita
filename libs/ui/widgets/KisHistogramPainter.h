/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_HISTOGRAM_PAINTER_H
#define KIS_HISTOGRAM_PAINTER_H

#include <QScopedPointer>
#include <QPainter>
#include <QVector>
#include <QRect>

#include "kritaui_export.h"

class KisHistogram;
class KoColorSpace;

/**
 * @brief A class that can render different KisHistograms. It renders a
 * somewhat smooth shape (with different colors for rgb, cmyk and xyz channels).
 * It can also render the different channels of a colorspace overlaped.
 */
class KRITAUI_EXPORT KisHistogramPainter
{
public:
    KisHistogramPainter();
    KisHistogramPainter(const KisHistogramPainter &other);
    KisHistogramPainter(KisHistogramPainter && other);
    ~KisHistogramPainter();

    /**
     * @brief Sets up the painter by passing a KisHistogram, the color space for
     * the histogram and the channels that must be set up for it
     * @param histogram A KisHistogram pointer to take the info from. The
     * histogram pointer is not stored or owned internally, so you can safely
     * delete it if you don't need it
     * @param colorSpaces The color space associated with the histogram. It is
     * used to color different channels. For example, the histogram for the red
     * channel of a rgb histogram will be painted as a red shape
     * @param channels A collection that contains the channel indices for the
     * channels that must be used to compute the histogram. For example, if the
     * histogram is a Lab histogram, and you are interested only in the
     * lightness channel, the vector should have only one element, the number 0
     * (index of the lightness channel). If the collection is empty, then all
     * the channels will be used
     */
    void setup(KisHistogram *histogram, const KoColorSpace *colorSpace, QVector<int> channels = {});

    /**
     * @brief Returns a RGBA image of size "imageSize" with the result of
     * rendering the selected histogram channels on it
     */
    QImage paint(const QSize &imageSize);
    /**
     * @brief Returns a RGBA image of size "w"x"h" with the result of
     * rendering the selected histogram channels on it
     */
    QImage paint(int w, int h);
    /**
     * @brief Paints the selected histogram channels in the given rect
     * using the given painter
     */
    void paint(QPainter &painter, const QRect &rect);
    /**
     * @brief Get the number of channels that are being used. It can be less
     * than the total number of channels in the KisHistogram. This can be set
     * by using the setup function
     * @see setup
     */
    int totalNumberOfAvailableChannels() const;
    /**
     * @brief Get a list containing all the indices of the channels that were
     * setup using the "setup" function. 
     * @see setup
     */
    QList<int> availableChannels() const;
    /**
     * @brief Get the list of channels that are currently activated (the only
     * ones that will be painted)
     * @see setChannel
     * @see setChannels
     */
    const QVector<int>& channels() const;
    /**
     * @brief Set currently active channel (the one that will be painted)
     * @see channels
     * @see setChannels
     */
    void setChannel(int channel);
    /**
     * @brief Set currently active channels (the ones that will be painted)
     * @see channels
     * @see setChannel
     */
    void setChannels(const QVector<int> &channels);
    /**
     * @brief Returns the color that is being used to paint a generic histogram. 
     * All the histogram channels will use this color with the exception of the
     * red, green and blue channels in a rgba histogram; the cyan, magenta,
     * yellow and black channels in a cmyka histogram; and the x, y, and z
     * channels in a xyz histogram
     * @see setDefaultColor
     */
    QColor defaultColor() const;
    /**
     * @brief Set the default color used when there is no preference to color
     * the selected histogram channel
     * @see defaultColor
     */
    void setDefaultColor(const QColor &newDefaultColor);
    /**
     * @brief Return the vertical scale that is used to paint the histogram
     * shape. A scale of 1 will paint all the histogram in the given rect.
     * Scales less that 1 are not allowed
     * @see setScale
     * @see setScaleToFit
     * @see setScaleToCutLongPeaks
     */
    qreal scale() const;
    /**
     * @brief Set the scale used to paint the histograms
     * @see scale 
     * @see setScaleToFit
     * @see setScaleToCutLongPeaks
     */
    void setScale(qreal newScale);
    /**
     * @brief Set the scale used to paint the histograms to 1.
     * @see scale
     * @see setScale
     * @see setScaleToCutLongPeaks
     */
    void setScaleToFit();
    /**
     * @brief Sometimes there can be some outliers in the histogram that show in
     * the painted shape as long vertical peaks. Since a scale of 1 will show
     * all the histogram, this can make its interesting parts too small to take
     * any valuable information from them. This function will try to come up
     * with a scale value that favors those interesting parts over the long
     * peaks, which will be pushed and will look cutted
     * @see scale
     * @see setScale
     * @see setScaleToFit
     */
    void setScaleToCutLongPeaks();
    /**
     * @brief Returns true if the histogram shape being painted is formed by the
     * logarithmic mapping of the original histogram values instead of the values
     * themselves
     * @see setLogarithmic
     */
    bool isLogarithmic() const;
    /**
     * @brief Set if the histogram shape being shown is formed by the
     * logarithmic mapping of the original histogram values instead of the
     * values themselves
     * @see isLogarithmic 
     */
    void setLogarithmic(bool logarithmic);

private:
    class Private;
    QScopedPointer<Private> m_d;
};

#endif
