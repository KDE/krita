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

#include "krs_filter.h"

#include <kis_filter.h>
#include <kis_paint_layer.h>

#include "krs_filter_configuration.h"
#include "krs_paint_layer.h"

namespace Kross {
namespace KritaCore {

Filter::Filter(KisFilter* filter)
    : Kross::Api::Class<Filter>("KritaFilter"), m_filter(filter), m_config( new FilterConfiguration(filter->configuration()) )
{
    addFunction("process", &Filter::process);
    addFunction("getFilterConfiguration", &Filter::getFilterConfiguration);

}

Filter::~Filter()
{
}

const QString Filter::getClassName() const {
    return "Kross::KritaCore::Filter";
}

Kross::Api::Object::Ptr Filter::getFilterConfiguration(Kross::Api::List::Ptr )
{
     return m_config;
}

Kross::Api::Object::Ptr Filter::process(Kross::Api::List::Ptr args)
{
    PaintLayer* src = (PaintLayer*)args->item(0).data();
    if(!m_filter->workWith( src->paintLayer()->paintDevice()->colorSpace()))
    {
        throw Kross::Api::Exception::Ptr( new Kross::Api::Exception( i18n("An error has occured in %1").arg("process") ) );
    }
    QRect rect;
    if( args->count() >1)
    {
        uint x = Kross::Api::Variant::toVariant(args->item(1)).toUInt();
        uint y = Kross::Api::Variant::toVariant(args->item(2)).toUInt();
        uint w = Kross::Api::Variant::toVariant(args->item(3)).toUInt();
        uint h = Kross::Api::Variant::toVariant(args->item(4)).toUInt();
        rect = QRect(x, y, w, h);
    } else {
        QRect r1 = src->paintLayer()->paintDevice()->extent();
        QRect r2 = src->paintLayer()->image()->bounds();
        rect = r1.intersect(r2);
    }
    m_filter->process( src->paintLayer()->paintDevice(), src->paintLayer()->paintDevice(), m_config->filterConfiguration(), rect );
    return 0;
}

}
}
