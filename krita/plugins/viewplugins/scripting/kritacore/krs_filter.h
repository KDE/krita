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

#include <api/class.h>

class KisFilter;

namespace Kross {
namespace KritaCore {
    class FilterConfiguration;
/**
	@author Cyrille Berger <cberger@cberger.net>
*/
class Filter : public Kross::Api::Class<Filter>
{
    public:
        Filter(KisFilter*);
        ~Filter();
    private:
        Kross::Api::Object::Ptr getFilterConfiguration(Kross::Api::List::Ptr args);
        Kross::Api::Object::Ptr process(Kross::Api::List::Ptr args);
    public:
        virtual const QString getClassName() const;
    private:
        KisFilter* m_filter;
        FilterConfiguration* m_config;
};

}
}

#endif
