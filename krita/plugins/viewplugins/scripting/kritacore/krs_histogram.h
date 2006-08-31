/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KROSS_KRITACOREHISTOGRAM_H
#define KROSS_KRITACOREHISTOGRAM_H

#include <QObject>

#include <kis_types.h>
#include <kis_histogram.h>

namespace Kross { namespace KritaCore {

class PaintLayer;

/**
 * This class allow to access the histogram of a \a PaintLayer object.
 *
 * Example (in Ruby) :
 * @code
 * require "Krita"
 * image = Krita.image()
 * layer = image.activePaintLayer()
 * histo = layer.createHistogram("RGB8HISTO",0)
 * min = histo.min() * 255
 * max = histo.max() * 255
 * for i in min..max
 *     print layer.getValue(i), "\n"
 * end
 * @endcode
 */
class Histogram : public QObject
{
        Q_OBJECT
    public:
        Histogram(PaintLayer* layer, KisHistogramProducerSP producer, const enumHistogramType type);
        ~Histogram();

    public slots:

        /**
         * Return the selected channel.
         */
        uint channel();

        /**
         * Select the channel of the layer on which to get the result of the histogram.
         * This function takes one argument :
         *  - channel number
         */
        void setChannel(uint channelnr);

        /**
         * This function return the maximum bound of the histogram
         * (values at greater position than the maximum are null).
         * The value is in the range 0.0 - 1.0.
         */
        double max();

        /**
         * This function return the minimum bound of the histogram
         * (values at smaller position than the minimum are null)
         * The value is in the range 0.0 - 1.0.
         */
        double min();

        /**
         * This function return the highest value of the histogram.
         */
        uint highest();

        /**
         * This function return the lowest value of the histogram.
         */
        uint lowest();

        /**
         * This function return the mean of the histogram.
         */
        double mean();

        /**
         * This function return the number of pixels used by the histogram.
         */
        uint count();

        /**
         * This function return the sum of all values of the histogram.
         */
        double total();

        /**
         * Return the value of a bin of the histogram.
         * This function takes one argument :
         *  - index, in the range [0..255], 
         */
        uint value(int index);

        /**
         * Return the number of bins of this histogram.
         */
        int numberOfBins();

    private:
        KisHistogram* m_histogram;
};

}}

#endif
