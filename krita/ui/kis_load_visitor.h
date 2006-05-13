/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
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
#ifndef KIS_LOAD_VISITOR_H_
#define KIS_LOAD_VISITOR_H_

#include <QRect>
#include "kis_types.h"
#include "kis_layer_visitor.h"
#include "kis_image.h"
#include "kis_selection.h"
#include "kis_layer.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_filter_configuration.h"

#include "kis_datamanager.h"

class KisLoadVisitor : public KisLayerVisitor {
public:
    KisLoadVisitor(KisImageSP img, KoStore *store, QMap<KisLayer *, QString> &layerFilenames) :
        KisLayerVisitor(),
        m_layerFilenames(layerFilenames)
    {
        m_external = false;
        m_img = img;
        m_store = store;
    }

public:
    void setExternalUri(QString &uri)
    {
        m_external = true;
        m_uri = uri;
    }

    virtual bool visit(KisPaintLayer *layer)
    {        //connect(*layer->paintDevice(), SIGNAL(ioProgress(qint8)), m_img, SLOT(slotIOProgress(qint8)));

        QString location = m_external ? QString::null : m_uri;
        location += m_img->name() + "/layers/" + m_layerFilenames[layer];

        // Layer data
        if (m_store->open(location)) {
            if (!layer->paintDevice()->read(m_store)) {
                layer->paintDevice()->disconnect();
                m_store->close();
                //IODone();
                return false;
            }

            m_store->close();
        }

        // icc profile
        location = m_external ? QString::null : m_uri;
        location += m_img->name() + "/layers/" + m_layerFilenames[layer] + ".icc";

        if (m_store->hasFile(location)) {
            QByteArray data;
            m_store->open(location);
            data = m_store->read(m_store->size());
            m_store->close();
            // Create a colorspace with the embedded profile
            KisColorSpace * cs = KisMetaRegistry::instance()->csRegistry()->getColorSpace(layer->paintDevice()->colorSpace()->id(),
                                                                                            new KisProfile(data));
            // replace the old colorspace
            layer->paintDevice()->setData(layer->paintDevice()->dataManager(), cs);
            QRect rc = layer->paintDevice()->extent();
            kDebug() << "After loading " << layer->name() << " extent is: " << rc.x() << ", " << rc.y() << ", " << rc.width() << ", " << rc.height() << endl;
            layer->setDirty(rc);
            kDebug(DBG_AREA_FILE) << "Opened icc information, size is " << data.size() << endl;
        }

        return true;

    }

    virtual bool visit(KisGroupLayer *layer)
    {
        KisLoadVisitor visitor(m_img, m_store, m_layerFilenames);

        if(m_external)
            visitor.setExternalUri(m_uri);

        KisLayerSP child = layer->firstChild();

        while(child)
        {
            child->accept(visitor);
            child = child->nextSibling();
        }

        layer->setDirty(m_img->bounds());
        return true;
    }

    virtual bool visit(KisPartLayer *)
    {
        return true;
    }
    
    virtual bool visit(KisAdjustmentLayer* layer)
    {
        //connect(*layer->paintDevice(), SIGNAL(ioProgress(qint8)), m_img, SLOT(slotIOProgress(qint8)));

        // The selection -- if present. If not, we simply cannot open the dratted thing.
        QString location = m_external ? QString::null : m_uri;
        location += m_img->name() + "/layers/" + m_layerFilenames[layer] + ".selection";
        if (m_store->hasFile(location)) {
            m_store->open(location);
            KisSelectionSP selection = KisSelectionSP(new KisSelection());
            if (!selection->read(m_store)) {
                selection->disconnect();
                m_store->close();
            }
            else {
                layer->setSelection( selection );
            }
            m_store->close();
        }

        // filter configuration
        location = m_external ? QString::null : m_uri;
        location += m_img->name() + "/layers/" + m_layerFilenames[layer] + ".filterconfig";

        if (m_store->hasFile(location) && layer->filter()) {
            QByteArray data;
            m_store->open(location);
            data = m_store->read(m_store->size());
            m_store->close();
            if (!data.isEmpty()) {
                KisFilterConfiguration * kfc = layer->filter();
                kfc->fromXML(QString(data));
            }
        }

        return true;

    }
    
private:
    KisImageSP m_img;
    KoStore *m_store;
    bool m_external;
    QString m_uri;
    QMap<KisLayer *, QString> m_layerFilenames;
};

#endif // KIS_LOAD_VISITOR_H_

