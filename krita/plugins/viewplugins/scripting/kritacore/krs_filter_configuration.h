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

#ifndef KROSS_KRITACOREKRS_FILTER_CONFIGURATION_H
#define KROSS_KRITACOREKRS_FILTER_CONFIGURATION_H

#include <QObject>

class KisFilterConfiguration;

namespace Kross {
namespace KritaCore {

/**
	@author Cyrille Berger <cberger@cberger.net>
*/
class FilterConfiguration : public QObject
{
        Q_OBJECT
    public:
        FilterConfiguration(KisFilterConfiguration*);
        ~FilterConfiguration();

        inline KisFilterConfiguration* filterConfiguration() { return m_fConfig; };

    public slots:

        /**
         * This function define a parameter of the associated Filter.
         * It takes two arguments :
         *  - the name of the parameter
         *  - the value, whose type depends of the Filter
         */
        void setProperty(const QString& name, const QVariant& value);

        /**
         * This function return the value of a parameter of the associated Filter.
         * It takes one argument :
         *  - the name of the parameter
         */
        const QVariant getProperty(const QString& name);

        /**
         * Deserialize from XML.
         */
        void fromXML(const QString& xml);

#if 0
        /**
         * Serialize to XML.
         */
        const QString toXML();
#endif

    private:
        KisFilterConfiguration* m_fConfig;
};

}
}

#endif
