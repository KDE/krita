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
    Private() : kconfig(0) {}
    const KisFilter* filter;
    QRect rect;
    KisFilterConfiguration* configuration() {
        if (!kconfig) {
            kconfig = filter->defaultConfiguration(0);
            if (kconfig) {
                kconfig->fromXML(configstr);
            }
        }
        return kconfig;
    }
    void setConfiguration(KisFilterConfiguration* conf) {
        delete kconfig;
        kconfig = conf;
        configstr = conf->toXML();
    }
    void setConfig(const QString& cfg) {
        delete kconfig;
        kconfig = 0;
        configstr = cfg;
    }
    const QString& config() {
        return configstr;
    }
private:
    QString configstr;
    KisFilterConfiguration* kconfig;
};

KisRecordedFilterAction::KisRecordedFilterAction(QString name, const KisNodeQueryPath& path, const KisFilter* filter, const KisFilterConfiguration* fc) : KisRecordedAction("FilterAction", name, path), d(new Private)
{
    Q_ASSERT(filter);
    d->filter = filter;
    if (fc) {
        d->setConfig(fc->toXML());
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

    KisFilterConfiguration * kfc = d->configuration();
    KisPaintDeviceSP dev = nodeQueryPath().queryNodes(info.image(), info.currentNode())[0]->paintDevice(); // TODO: not good should take the full list into consideration
    KisTransaction * cmd = 0;
    if (info.undoAdapter()) cmd = new KisTransaction(d->filter->name(), dev);

    QRect r1 = dev->extent();

    // Ugly hack to get at the image without bloating the node interface
    KisImageWSP image;
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
    KisFilterConfiguration * kfc = d->configuration();
    if (kfc) {
        QDomElement filterConfigElt = doc.createElement("Params");
        kfc->toXML(doc, filterConfigElt);
        elt.appendChild(filterConfigElt);
    }
}

KisRecordedAction* KisRecordedFilterAction::clone() const
{
    return new KisRecordedFilterAction(*this);
}

const KisFilter* KisRecordedFilterAction::filter() const
{
    return d->filter;
}

const KisFilterConfiguration* KisRecordedFilterAction::filterConfiguration() const
{
    return d->configuration();
}

void KisRecordedFilterAction::setFilterConfiguration(KisFilterConfiguration* config)
{
    d->setConfiguration(config);
}


KisRecordedFilterActionFactory::KisRecordedFilterActionFactory() :
        KisRecordedActionFactory("FilterAction")
{
}

KisRecordedFilterActionFactory::~KisRecordedFilterActionFactory()
{

}

KisRecordedAction* KisRecordedFilterActionFactory::fromXML(const QDomElement& elt)
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
