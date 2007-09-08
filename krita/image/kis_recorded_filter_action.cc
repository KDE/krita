/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_recorded_filter_action.h"

#include <QDomElement>
#include <QString>


#include "kis_filter.h"
#include "kis_filter_configuration.h"
#include "kis_filter_registry.h"
#include "kis_layer.h"
#include "kis_selection.h"

struct KisRecordedFilterAction::Private {
    KisLayerSP layer;
    const KisFilter* filter;
    QString config;
    QRect rect;
};

KisRecordedFilterAction::KisRecordedFilterAction(QString name, KisLayerSP layer, const KisFilter* filter, KisFilterConfiguration* fc) : KisRecordedAction(name, "FilterAction"), d(new Private)
{
    d->layer = layer;
    d->filter = filter;
    if(fc)
    {
        d->config = fc->toXML();
    }
}

KisRecordedFilterAction::~KisRecordedFilterAction()
{
}

void KisRecordedFilterAction::play()
{
    KisFilterConfiguration * kfc = d->filter->defaultConfiguration(0);
    if(kfc)
    {
        kfc->fromXML(d->config);
    }
    KisPaintDeviceSP dev = d->layer->paintDevice();

    QRect r1 = dev->extent();
    QRect r2 = d->layer->image()->bounds();

    // Filters should work only on the visible part of an image.
    QRect rect = r1.intersect(r2);

    if (dev->hasSelection()) {
        QRect r3 = dev->selection()->selectedExactRect();
        rect = rect.intersect(r3);
    }

    const_cast<KisFilter*>(d->filter)->process( d->layer->paintDevice(), rect, kfc);
}

void KisRecordedFilterAction::toXML(QDomDocument& doc, QDomElement& elt)
{
    KisRecordedAction::toXML(doc,elt);
    elt.setAttribute("layer", KisRecordedAction::layerToIndexPath(d->layer));
    elt.setAttribute("filter", d->filter->id());
    // Save configuration
    KisFilterConfiguration * kfc = d->filter->defaultConfiguration(d->layer->paintDevice());
    if(kfc)
    {
        QDomElement filterConfigElt = doc.createElement( "Params");
        kfc->toXML(doc, filterConfigElt);
        elt.appendChild(filterConfigElt);
    }
}

KisRecordedFilterActionFactory::KisRecordedFilterActionFactory() :
        KisRecordedActionFactory("FilterAction")
{
}

KisRecordedFilterActionFactory::~KisRecordedFilterActionFactory()
{
    
}

KisRecordedAction* KisRecordedFilterActionFactory::fromXML(KisImageSP img, const QDomElement& elt)
{
    QString name = elt.attribute("name");
    KisLayerSP layer = KisRecordedActionFactory::indexPathToLayer(img, elt.attribute("layer"));
    const KisFilterSP filter = KisFilterRegistry::instance()->get(elt.attribute("filter"));
    if(filter)
    {
        KisFilterConfiguration* config = filter->defaultConfiguration(layer->paintDevice());
        QDomElement paramsElt = elt.firstChildElement("Params");
        if(config and not paramsElt.isNull())
        {
            config->fromXML(paramsElt);
        }
        KisRecordedFilterAction* rfa = new KisRecordedFilterAction(name, layer, filter, config);
        delete config;
        return rfa;
    } else {
        return 0;
    }
}
