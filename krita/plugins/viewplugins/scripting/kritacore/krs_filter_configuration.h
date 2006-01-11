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

#include <api/class.h>

class KisFilterConfiguration;

namespace Kross {
namespace KritaCore {

/**
	@author Cyrille Berger <cberger@cberger.net>
*/
class FilterConfiguration : public Kross::Api::Class<FilterConfiguration>
{
    public:
        FilterConfiguration(const QString name, Q_INT32 version);
        ~FilterConfiguration();
    public:
        virtual const QString getClassName() const;
        inline KisFilterConfiguration* filterConfiguration() { return m_fConfig; };
    private:
        Kross::Api::Object::Ptr setProperty(Kross::Api::List::Ptr args);
        Kross::Api::Object::Ptr getProperty(Kross::Api::List::Ptr args);
    private:
        KisFilterConfiguration* m_fConfig;
};

}
}

#endif
