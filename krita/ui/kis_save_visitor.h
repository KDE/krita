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
#ifndef KIS_SAVE_VISITOR_H_
#define KIS_SAVE_VISITOR_H_

#include <qrect.h>
#include "kis_types.h"
#include "kis_layer_visitor.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"

class KisSaveVisitor : public KisLayerVisitor {
public:
    KisSaveVisitor(KisImageSP img, KoStore *store, Q_UINT32 &count) :
        KisLayerVisitor(),
        m_count(count)
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
    {
        //connect(*layer->paintDevice(), SIGNAL(ioProgress(Q_INT8)), m_img, SLOT(slotIOProgress(Q_INT8)));

        QString location = m_external ? QString::null : m_uri;
        location += m_img->name() + QString("/layers/layer%1").arg(m_count);

        // Layer data
        if (m_store->open(location)) {
            if (!layer->paintDevice()->write(m_store)) {
                layer->paintDevice()->disconnect();
                m_store->close();
                //IODone();
                return false;
            }

            m_store->close();
        }

        if (layer->paintDevice()->colorSpace()->getProfile()) {
            KisAnnotationSP annotation = layer->paintDevice()->colorSpace()->getProfile()->annotation();

            if (annotation) {
                // save layer profile
                location = m_external ? QString::null : m_uri;
                location += m_img->name() + QString("/layers/layer%1").arg(m_count) + ".icc";

                if (m_store->open(location)) {
                    m_store->write(annotation->annotation());
                    m_store->close();
                }
            }
        }

        if (layer->hasMask()) {
            KisPaintDeviceSP mask = layer->getMask();

            if (mask) {
                // save layer profile
                location = m_external ? QString::null : m_uri;
                location += m_img->name() + QString("/layers/layer%1").arg(m_count) + ".mask";

                if (m_store->open(location)) {
                    if (!mask->write(m_store)) {
                        mask->disconnect();
                        m_store->close();
                        return false;
                    }

                    m_store->close();
                }
            }
        }

        m_count++;
        return true;
    }

    virtual bool visit(KisGroupLayer *layer)
    {
        KisSaveVisitor visitor(m_img, m_store, m_count);

        if(m_external)
            visitor.setExternalUri(m_uri);

        KisLayerSP child = layer->firstChild();

        while(child)
        {
            child->accept(visitor);
            child = child->nextSibling();
        }
        return true;
    }

    virtual bool visit(KisPartLayer *)
    {
        return true;
    }

    virtual bool visit(KisAdjustmentLayer* layer)
    {

        if (layer->selection()) {
            QString location = m_external ? QString::null : m_uri;
            location += m_img->name() + QString("/layers/layer%1").arg(m_count) + ".selection";

            // Layer data
            if (m_store->open(location)) {
                if (!layer->selection()->write(m_store)) {
                    layer->selection()->disconnect();
                    m_store->close();
                    //IODone();
                    return false;
                }
                m_store->close();
            }
        }

        if (layer->filter()) {
            QString location = m_external ? QString::null : m_uri;
            location = m_external ? QString::null : m_uri;
            location += m_img->name() + QString("/layers/layer%1").arg(m_count) + ".filterconfig";

            if (m_store->open(location)) {
                QString s = layer->filter()->toString();
                m_store->write(s.utf8(), qstrlen(s.utf8()));
                m_store->close();
            }
        }
        m_count++;
        return true;
    }
    
private:
    KisImageSP m_img;
    KoStore *m_store;
    bool m_external;
    QString m_uri;
    Q_UINT32 &m_count;
};

#endif // KIS_SAVE_VISITOR_H_

