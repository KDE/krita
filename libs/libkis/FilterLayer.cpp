/*
 *  SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "FilterLayer.h"
#include <kis_adjustment_layer.h>
#include <kis_image.h>
#include <kis_filter_configuration.h>
#include <kis_filter_registry.h>
#include <InfoObject.h>
#include <kis_selection.h>

FilterLayer::FilterLayer(KisImageSP image, QString name, Filter &filter, Selection &selection, QObject *parent) :
    Node(image, new KisAdjustmentLayer(image, name, filter.filterConfig()->cloneWithResourcesSnapshot(), selection.selection()), parent)
{

}

FilterLayer::FilterLayer(KisAdjustmentLayerSP layer, QObject *parent):
    Node(layer->image(), layer, parent)
{

}

FilterLayer::~FilterLayer()
{

}

QString FilterLayer::type() const
{
    return "filterlayer";
}

void FilterLayer::setFilter(Filter &filter)
{
    if (!this->node()) return;
    KisAdjustmentLayer *layer = dynamic_cast<KisAdjustmentLayer*>(this->node().data());
    //getting the default configuration here avoids trouble with versioning.
    if (layer) {
        layer->setFilter(filter.filterConfig()->cloneWithResourcesSnapshot());
    }
}

Filter * FilterLayer::filter()
{
    Filter* filter = new Filter();
    const KisAdjustmentLayer *layer = qobject_cast<const KisAdjustmentLayer*>(this->node());
    filter->setName(layer->filter()->name());
    filter->setConfiguration(new InfoObject(layer->filter()));
    return filter;
}

