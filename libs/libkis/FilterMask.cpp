/*
 *  Copyright (c) 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "FilterMask.h"
#include <kis_filter_mask.h>
#include <kis_image.h>
#include <kis_filter_configuration.h>
#include <kis_filter_registry.h>
#include <InfoObject.h>

FilterMask::FilterMask(KisImageSP image, QString name, Filter &filter, QObject *parent) :
    Node(image, new KisFilterMask(image, name), parent)
{
    KisFilterMask *mask = dynamic_cast<KisFilterMask*>(this->node().data());
    KIS_SAFE_ASSERT_RECOVER_RETURN(mask);
    
    mask->setFilter(filter.filterConfig()->cloneWithResourcesSnapshot());
}

FilterMask::FilterMask(KisImageSP image, KisFilterMaskSP mask, QObject *parent):
    Node(image, mask, parent)
{

}

FilterMask::~FilterMask()
{

}

QString FilterMask::type() const
{
    return "filtermask";
}

void FilterMask::setFilter(Filter &filter)
{
    KisFilterMask *mask = dynamic_cast<KisFilterMask*>(this->node().data());
    KIS_SAFE_ASSERT_RECOVER_RETURN(mask);

    mask->setFilter(filter.filterConfig()->cloneWithResourcesSnapshot());
}

Filter * FilterMask::filter()
{
    Filter* filter = new Filter();
    const KisFilterMask *mask = qobject_cast<const KisFilterMask*>(this->node());
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(mask, 0);

    filter->setName(mask->filter()->name());
    filter->setConfiguration(new InfoObject(mask->filter()));
    return filter;
}

