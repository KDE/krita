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
#include "FilterLayer.h"
#include <kis_adjustment_layer.h>
#include <kis_image.h>
#include <kis_filter_configuration.h>
#include <kis_filter_registry.h>
#include <InfoObject.h>
#include <kis_selection.h>

FilterLayer::FilterLayer(KisImageSP image, QString name, Filter &filter, Selection &selection, QObject *parent) :
    Node(image, new KisAdjustmentLayer(image, name, filter.filterConfig(), selection.selection()), parent)
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
        layer->setFilter(filter.filterConfig());
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

