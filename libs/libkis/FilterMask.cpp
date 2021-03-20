/*
 *  SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
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

