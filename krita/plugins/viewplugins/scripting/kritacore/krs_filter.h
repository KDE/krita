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

class KisFilter;

namespace Kross {
namespace KritaCore {

class FilterConfiguration;

class Filter : public QObject
{
        Q_OBJECT
    public:
        Filter(KisFilter*);
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
         * Deserialize the filter-configuration from XML.
         */
        void fromXML(const QString& xml);

        /**
         * Serialize filter-configuration to XML.
         */
        const QString toXML();

#if 0
        /**
         * This function will apply the filter.
         * It takes one argument :
         *  - the source layer
         * You can also use this four aguments :
         *  - x
         *  - y
         *  - width
         *  - height
         * 
         * (x,y, width, height) defines the rectangular area on which the filter will be computed.
         * If the rectangle is not defined, then the filter will be apply on alll the source layer.
         * 
         * For example (in ruby)
         * @code
         * doc = Krosskritacore::get("KritaDocument")
         * image = doc.getImage()
         * layer = image.getActivePaintLayer()
         * width = layer.getWidth()
         * height = layer.getHeight()
         * filter = Krosskritacore::getFilter("invert")
         * filter.process(layer, layer)
         * filter.process(layer, layer, 10, 10, 20, 20 )
         * @endcode
         */
        Kross::Api::Object::Ptr process(Kross::Api::List::Ptr args);
#endif

    private:
        KisFilter* m_filter;
};

}
}

#endif
