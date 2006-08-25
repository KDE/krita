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

namespace Kross {

namespace KritaCore {

/**
 * This class allow to access the histogram of a PaintLayer.
 * 
 * Example (in Ruby) :
 * @code
 * doc = krosskritacore::get("KritaDocument")
 * image = doc.getImage()
 * layer = image.getActiveLayer()
 * histo = layer.createHistogram("RGB8HISTO",0)
 * min = layer.getMin() * 255
 * max = layer.getMax() * 255
 * for i in min..max
 *   print layer.getValue(i)
 *   print "\n"
 * end
 * @endcode
 */
class Histogram : public QObject
{
        //Q_OBJECT
    public:
        Histogram(KisPaintLayerSP layer, KisHistogramProducerSP producer, const enumHistogramType type);
        ~Histogram();

    //public slots:

#if 0
        /**
         * This function return the maximum bound of the histogram
         * (values at greater position than the maximum are null).
         * The value is in the range 0.0 - 1.0.
         */
        Kross::Api::Object::Ptr getMax(Kross::Api::List::Ptr);
        /**
         * This function return the minimum bound of the histogram
         * (values at smaller position than the minimum are null)
         * The value is in the range 0.0 - 1.0.
         */
        Kross::Api::Object::Ptr getMin(Kross::Api::List::Ptr);
        /**
         * This function return the highest value of the histogram
         */
        Kross::Api::Object::Ptr getHighest(Kross::Api::List::Ptr);
        /**
         * This function return the lowest value of the histogram
         */
        Kross::Api::Object::Ptr getLowest(Kross::Api::List::Ptr);
        /**
         * This function return the mean of the histogram
         */
        Kross::Api::Object::Ptr getMean(Kross::Api::List::Ptr);
        /**
         * This function return the number of pixels used by the histogram
         */
        Kross::Api::Object::Ptr getCount(Kross::Api::List::Ptr);
        /**
         * This function return the sum of all values of the histogram
         */
        Kross::Api::Object::Ptr getTotal(Kross::Api::List::Ptr);
        /**
         * Select the channel of the layer on which to get the result of the histogram.
         * This function takes one argument :
         *  - channel number
         */
        Kross::Api::Object::Ptr setChannel(Kross::Api::List::Ptr);
        /**
         * Return the selected channel
         */
        Kross::Api::Object::Ptr getChannel(Kross::Api::List::Ptr);
        /**
         * Return the value of a bin of the histogram.
         * This function takes one argument :
         *  - index, in the range [0..255], 
         */
        Kross::Api::Object::Ptr getValue(Kross::Api::List::Ptr);
        /**
         * Return the number of bins of this histogram.
         */
        Kross::Api::Object::Ptr getNumberOfBins(Kross::Api::List::Ptr);
#endif

    private:
        KisHistogram* m_histogram;
};

}

}

#endif
