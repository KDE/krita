/*
 *  Copyright (c) 2004 Boudewijn Rempt
 *            (c) 2005 Bart Coppens <kde@bartcoppens.be>
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
#ifndef KIS_HISTOGRAM_
#define KIS_HISTOGRAM_

#include <QVector>

#include "kis_types.h"
#include "KoColorSpace.h"
#include "kis_histogram_producer.h"

enum enumHistogramType {
    LINEAR,
    LOGARITHMIC
};
/**
 * The histogram class computes the histogram data from the specified layer
 * for the specified channel, through the use of a KisHistogramProducer.
 * This class is only for layers and paintdevices. KisImages are not supported,
 * but you can use the mergedImage function to create a paintdevice and feed that to this class.
 *
 * A Histogram also can have a selection: this is a specific range in the current histogram
 * that will get calculations done on it as well. If the range's begin and end are the same,
 * it is supposed to specify a single bin in the histogram.
 *
 * The calculations are done in the range 0 - 1, instead of the native range that a pixel
 * might have, so it's not always as precise as it could be. But you can't have it all...
 */
class KRITAIMAGE_EXPORT KisHistogram : public KShared {

public:
    /**
     * Class that stores the result of histogram calculations.
     * Doubles are in the 0-1 range, use the producer's positionToString function to display it.
     **/
    class Calculations {

        double m_max, m_min, m_mean, m_total, m_median, m_stddev;

        quint32 m_high, m_low, m_count;

        friend class KisHistogram;

    public:

        Calculations() : m_max(0.0), m_min(0.0), m_mean(0.0), m_total(0.0), m_median(0.0),
            m_stddev(0.0), m_high(0), m_low(0), m_count(0) {}
        /**
         * This function return the maximum bound of the histogram
         * (values at greater position than the maximum are null)
         */
        inline double getMax() { return m_max; }
        /**
         * This function return the minimum bound of the histogram
         * (values at smaller position than the minimum are null)
         */
        inline double getMin() { return m_min; }
        /// This function return the highest value of the histogram
        inline quint32 getHighest() { return m_high; }
        /// This function return the lowest value of the histogram
        inline quint32 getLowest() { return m_low; }
        /// This function return the mean of value of the histogram
        inline double getMean() { return m_mean; }
        //double getMedian() { return m_median; }
        //double getStandardDeviation() { return m_stddev; }
        /// This function return the number of pixels used by the histogram
        inline quint32 getCount() { return m_count; }
        /** The sum of (the contents of every bin * the double value of that bin)*/
        inline double getTotal() { return m_total; }
        //quint8 getPercentile() { return m_percentile; } // What is this exactly? XXX
    };

    KisHistogram(KisPaintLayerSP layer,
                 KisHistogramProducerSP producer,
                 const enumHistogramType type);

    KisHistogram(KisPaintDeviceSP paintdev,
                 KisHistogramProducerSP producer,
                 const enumHistogramType type);

    virtual ~KisHistogram();

    /** Updates the information in the producer */
    void updateHistogram();

    /**
     * (Re)computes the mathematical information from the information currently in the producer.
     * Needs to be called when you change the selection and want to get that information
     **/
    void computeHistogram();

    /** The information on the entire view for the current channel */
    Calculations calculations();
    /** The information on the current selection for the current channel */
    Calculations selectionCalculations();

    inline quint32 getValue(quint8 i) { return m_producer->getBinAt(m_channel, i); }

    inline enumHistogramType getHistogramType() { return m_type; }
    inline void setHistogramType(enumHistogramType type) { m_type = type; }
    inline void setProducer(KisHistogramProducerSP producer) { m_producer = producer; }
    inline void setChannel(qint32 channel) { m_channel = channel; }
    inline KisHistogramProducerSP producer() { return m_producer; }
    inline qint32 channel() { return m_channel; }

    inline bool hasSelection() { return m_selection; }
    inline double selectionFrom() { return m_selFrom; }
    inline double selectionTo() { return m_selTo; }
    inline void setNoSelection() { m_selection = false; }
    /** Sets the current selection */
    inline void setSelection(double from, double to)
        { m_selection = true; m_selFrom = from; m_selTo = to; }


private:
    // Dump the histogram to debug.
    void dump();
    QVector<Calculations> calculateForRange(double from, double to);
    Calculations calculateSingleRange(int channel, double from, double to);

    KisPaintDeviceSP m_device;
    KisHistogramProducerSP m_producer;

    enumHistogramType m_type;

    qint32 m_channel;
    double m_selFrom, m_selTo;
    bool m_selection;

    KisPaintDeviceSP m_dev;

    QVector<Calculations> m_completeCalculations, m_selectionCalculations;
};


#endif // KIS_HISTOGRAM_WIDGET_
