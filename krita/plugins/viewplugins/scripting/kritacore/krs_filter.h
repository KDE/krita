/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#ifndef KROSS_KRITACOREKRS_FILTER_H
#define KROSS_KRITACOREKRS_FILTER_H

#include <QObject>
#include <QVariant>

class KisFilter;

namespace Kross { namespace KritaCore {

class KritaCoreModule;

/**
 * This class enables access to the filters Krita provides.
 *
 * For example (in Ruby)
 * @code
 * require "Krita"
 *
 * # fetch the image.
 * image = Krita.image()
 *
 * # we like to operate on the active painting layer.
 * layer = image.activePaintLayer()
 *
 * # get the height and the width the layer has.
 * width = layer.width()
 * height = layer.height()
 *
 * # we like to use the progressbar
 * progress = Krita.progress()
 * progress.setProgressTotalSteps( (width / 20) * (height / 20) )
 *
 * # apply the invert filter each 20x20 pixels at a 10x10 rect.
 * invertfilter = Krita.filter("invert")
 * 0.step(width - 10, 20) do |x|
 *     0.step(height - 10, 20) do |y|
 *         invertfilter.process(layer, x, y, x + 10, y + 10)
 *         progress.incProgress()
 *     end
 * end
 * @endcode
 */
class Filter : public QObject
{
        Q_OBJECT
    public:
        Filter(KritaCoreModule* module, KisFilter*);
        ~Filter();

    public slots:

        /**
         * Return the unique name the filter has.
         */
        const QString name();

        /**
         * This function return the value of a parameter of the associated
         * Filter. The properties define the filters-configuration and you
         * are able to use \a property and \a setProperty to manipulate them.
         *
         * It takes one argument :
         *  - the name of the parameter
         * It returns the properties value.
         */
        const QVariant property(const QString& name);

        /**
         * This function define a parameter of the associated Filter.
         *
         * It takes two arguments :
         *  - the name of the parameter
         *  - the value, whose type depends of the Filter
         */
        void setProperty(const QString& name, const QVariant& value);

        /**
         * This function returns all properties the filter has
         * as name=value dictonary.
         *
         * For example (in Python)
         * @code
         * require "Krita"
         * filter = Krita.filter("invert")
         * for propname in filter.properties().keys():
         *     propvalue = filter.property(propname)
         *     filter.setProperty(propname, propvalue)
         * @endcode
         */
        QVariantMap properties();

        /**
         * Deserialize the filter-configuration from XML.
         */
        void fromXML(const QString& xml);

        /**
         * Serialize filter-configuration to XML.
         */
        const QString toXML();

        /**
         * This function will apply the filter.
         * It takes one argument :
         *  - the source layer
         *
         * For example (in Ruby)
         * @code
         * require "Krita"
         * layer = Krita.image().activePaintLayer()
         * filter = Krita.filter("invert")
         * filter.process(layer)
         * @endcode
         */
        bool process(QObject* layer);

        /**
         * This function will apply the filter.
         * It takes five argument :
         *  - the source layer
         *  - x
         *  - y
         *  - width
         *  - height
         *
         * (x,y, width, height) defines the rectangular area on which the filter will be computed.
         * If the rectangle is not defined, then the filter will be apply on alll the source layer.
         * 
         * For example (in Ruby)
         * @code
         * require "Krita"
         * layer = Krita.image().activePaintLayer()
         * filter = Krita.filter("invert")
         * filter.process(layer, 10, 10, 20, 20)
         * @endcode
         */
        bool process(QObject* layer, int x, int y, int width, int height);

    private:
        KisFilter* m_filter;
};

}}

#endif
