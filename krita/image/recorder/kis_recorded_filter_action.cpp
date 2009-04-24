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

#include "recorder/kis_recorded_filter_action.h"
#include <QDomElement>
#include <QString>


#include "kis_image.h"
#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include "kis_layer.h"
#include "kis_node.h"
#include "kis_selection.h"
#include "kis_transaction.h"
#include "kis_undo_adapter.h"
#include "kis_selection_mask.h"
#include "kis_config_widget.h"
#include "kis_node_query_path.h"
#include "kis_play_info.h"

struct KisRecordedFilterAction::Private {
    const KisFilter* filter;
    QString config;
    QRect rect;
};

KisRecordedFilterAction::KisRecordedFilterAction(QString name, const KisNodeQueryPath& path, const KisFilter* filter, KisFilterConfiguration* fc) : KisRecordedAction("FilterAction", name, path), d(new Private)
{
    d->filter = filter;
    if (fc) {
        d->config = fc->toXML();
    }
}

KisRecordedFilterAction::KisRecordedFilterAction(const KisRecordedFilterAction& rhs) : KisRecordedAction(rhs), d(new Private(*rhs.d))
{
}

KisRecordedFilterAction::~KisRecordedFilterAction()
{
}

void KisRecordedFilterAction::play(KisNodeSP node, const KisPlayInfo& info) const
{

    KisFilterConfiguration * kfc = d->filter->defaultConfiguration(0);
    if (kfc) {
        kfc->fromXML(d->config);
    }
    KisPaintDeviceSP dev = nodeQueryPath().queryNodes(info.image(), info.currentNode())[0]->paintDevice(); // TODO: not good should take the full list into consideration
    KisTransaction * cmd = 0;
    if ( info.undoAdapter()) cmd = new KisTransaction(d->filter->name(), dev);

    QRect r1 = dev->extent();

    // Ugly hack to get at the image without bloating the node interface
    KisImageSP image;
    KisNodeSP parent = nodeQueryPath().queryNodes(info.image(), info.currentNode())[0];
    while (image == 0 && parent->parent()) {
        // XXX: ugly!
        KisLayerSP layer = dynamic_cast<KisLayer*>(parent.data());
        if (layer) {
            image = layer->image();
            r1 = r1.intersected(image->bounds());
            if (layer->selectionMask()) {
                r1 = r1.intersected(layer->selectionMask()->exactBounds());
            }
            if (image->globalSelection())
                r1 = r1.intersected(image->globalSelection()->selectedExactRect());
        }
        parent = parent->parent();
    }

    d->filter->process(dev, r1, kfc);
    nodeQueryPath().queryNodes(info.image(), info.currentNode())[0]->setDirty(r1);
    if (info.undoAdapter()) info.undoAdapter()->addCommand(cmd);
}

void KisRecordedFilterAction::toXML(QDomDocument& doc, QDomElement& elt) const
{
    KisRecordedAction::toXML(doc, elt);
    elt.setAttribute("filter", d->filter->id());
    // Save configuration
    KisFilterConfiguration * kfc = d->filter->defaultConfiguration(0);
    if (kfc) {
        kfc->fromXML(d->config);
        QDomElement filterConfigElt = doc.createElement("Params");
        kfc->toXML(doc, filterConfigElt);
        elt.appendChild(filterConfigElt);
    }
}

KisRecordedAction* KisRecordedFilterAction::clone() const
{
    return new KisRecordedFilterAction(*this);
}

QWidget* KisRecordedFilterAction::createEditor(QWidget* parent)
{
    // TODO make a proxy QObject to update the action
    KisConfigWidget* widget = d->filter->createConfigurationWidget(parent, 0, 0 );
    KisFilterConfiguration * kfc = d->filter->defaultConfiguration(0);
    if (kfc) {
        kfc->fromXML(d->config);
    }
    widget->setConfiguration(kfc);
    return widget;
}

KisRecordedFilterActionFactory::KisRecordedFilterActionFactory() :
        KisRecordedActionFactory("FilterAction")
{
}

KisRecordedFilterActionFactory::~KisRecordedFilterActionFactory()
{

}

KisRecordedAction* KisRecordedFilterActionFactory::fromXML( const QDomElement& elt)
{
    QString name = elt.attribute("name");
    KisNodeQueryPath pathnode = KisNodeQueryPath::fromString(elt.attribute("path"));
    const KisFilterSP filter = KisFilterRegistry::instance()->get(elt.attribute("filter"));
    if (filter) {
        KisFilterConfiguration* config = filter->defaultConfiguration(0);
        QDomElement paramsElt = elt.firstChildElement("Params");
        if (config && !paramsElt.isNull()) {
            config->fromXML(paramsElt);
        }
        KisRecordedFilterAction* rfa = new KisRecordedFilterAction(name, pathnode, filter, config);
        delete config;
        return rfa;
    } else {
        return 0;
    }
}
