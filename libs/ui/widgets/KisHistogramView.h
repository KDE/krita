/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_HISTOGRAM_VIEW_H
#define KIS_HISTOGRAM_VIEW_H

#include <QScopedPointer>
#include <QWidget>

#include "kritaui_export.h"

class KisHistogram;
class KoColorSpace;

/**
 * @brief A widget that can display different KisHistograms. It renders a
 * somewhat smooth shape with different colors (for rgb, cmyk and xyz channels).
 * It can also display the different channels of a colorspace overlaped.
 */
class KRITAUI_EXPORT KisHistogramView : public QWidget
{
    Q_OBJECT

public:
    KisHistogramView(QWidget *parent);
    ~KisHistogramView();

    /**
     * @brief Sets up the widget by passing a collection of KisHistograms, the
     * color space for each histogram and the channels that must be set up for
     * each histogram
     * @param histograms A collection of KisHistogram pointers to take the info
     * from. You can add several histograms and later select which histogram is
     * used at any given moment. This prevents calling multiple times the setup
     * function with different histograms. That way the shaped to draw are
     * computed only once. The histogram pointers are not stored or owned
     * internally, so you can safely delete them if you don't need them
     * @param colorSpaces The collection of color spaces associated with each
     * histogram. They are used to color different channels. For example, the
     * histogram for the red channel of a rgb histogram will be painted as a red
     * shape
     * @param channels A collection of vectors that contain the channel number
     * for the channels that must be used to compute the histograms. For example,
     * if the second histogram is a Lab histogram, and you are interested only
     * in the lightness channel, the second vector in the collection should have
     * only one element, the number 0 (index of the lightness channel). If the
     * collection is empty or if the vector of channels for any given histogram
     * is empty, then all the channels will be used
     */
    void setup(const QVector<KisHistogram*> &histograms,
               const QVector<const KoColorSpace*> &colorSpaces,
               const QVector<QVector<int>> &channels = {});
    /**
     * @brief Returns the channels that are available for the currently selected
     * histogram
     * @see setChannel
     * @see setChannels
     */
    const QVector<int>& channels() const;
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
     * @brief Return the vertical scale that is used to paint the histogram
     * shape. A scale of 1 will show all the histogram in the widget area.
     * Scales less that 1 are not allowed
     * @see setScale
     * @see setScaleToFit
     * @see setScaleToCutLongPeaks
     */
    qreal scale() const;
    /**
     * @brief Returns true if the histogram shape being shown is formed by the
     * logarithmic mapping of the original histogram values instead of the values
     * themselves
     * @see setLogarithmic
     */
    bool isLogarithmic() const;

public Q_SLOTS:
    /**
     * @brief Activates the given channel of the given histogram. This allows to
     * change the view between the different histograms and channels passed to
     * the setup function
     * @see channel
     * @see setChannels
     * @see setup
     */
    void setChannel(int channel, int histogramIndex = 0);
    /**
     * @brief Activates the given channels of the given histogram. This allows to
     * change the view between the different histograms and channels passed to
     * the setup function. This functionactivates multiple channels that will be
     * displayed at once in the widget
     * @see channel
     * @see setChannel
     * @see setup
     */
    void setChannels(const QVector<int> &channels, int histogramIndex = 0);
    /**
     * @brief This clears the channel selection. The widget will show a null
     * symbol in it's center when there are no activated channels
     */
    void clearChannels();
    /**
     * @brief Set the default color used when there is no preference to color
     * the selected histogram
     * @see defaultColor
     */
    void setDefaultColor(const QColor &newDefaultColor);
    /**
     * @brief Set the scale used to paint the widget. The scale can also be
     * changed by clicking and dragging up/down in the widget
     * @see scale 
     * @see setScaleToFit
     * @see setScaleToCutLongPeaks
     */
    void setScale(qreal newScale);
    /**
     * @brief Set the scale used to paint the widget to 1. You can also return
     * to a scale of 1 by double clicking on the widget if the current scale is
     * other than 1. If you double click when the scale is one the effect will
     * be the same as calling setScaleToCutLongPeaks
     * @see scale
     * @see setScale
     * @see setScaleToCutLongPeaks
     */
    void setScaleToFit();
    /**
     * @brief Sometimes there can be some outliers in the histogram that show in
     * the widget as long vertical peaks. Since a scale of 1 will show all the
     * histogram, this can make its interesting parts too small to take any
     * valuable information from them. This function will try to come up with a
     * scale value that favors those interesting parts over the long peaks,
     * which will be pushed and will look cutted
     * @see scale
     * @see setScale
     * @see setScaleToFit
     */
    void setScaleToCutLongPeaks();
    /**
     * @brief Set if the histogram shape being shown is formed by the
     * logarithmic mapping of the original histogram values instead of the
     * values themselves
     * @see isLogarithmic 
     */
    void setLogarithmic(bool logarithmic);

protected:
    void paintEvent(QPaintEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;

private:
    class Private;
    QScopedPointer<Private> m_d;
};

#endif
