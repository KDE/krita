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

#include <kis_kra_save_visitor.h>

#include <colorprofiles/KoIccColorProfile.h>
#include <KoStore.h>

#include "kis_adjustment_layer.h"
#include "kis_annotation.h"
#include "kis_filter_configuration.h"
#include "kis_group_layer.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_paint_layer.h"
#include "kis_selection.h"
#include "kis_shape_layer.h"


KisKraSaveVisitior::KisKraSaveVisitior(KisImageSP img, KoStore *store, quint32 &count, QString name) :
    KisNodeVisitor(),
    m_count(count)
{
    m_external = false;
    m_img = img;
    m_store = store;
    m_name = name;
}

void KisKraSaveVisitior::setExternalUri(QString &uri)
{
    m_external = true;
    m_uri = uri;
}

bool KisKraSaveVisitior::visit( KisExternalLayer * layer )
    {
        bool result = false;
        if (KisShapeLayer * shapeLayer = dynamic_cast<KisShapeLayer*>(layer)) {
            
            QString location = m_external ? QString::null : m_uri;
            m_store->pushDirectory();
            m_store->enterDirectory( m_name + QString("/shapelayers/layer%1").arg(m_count) );
            result = shapeLayer->saveOdf(m_store);
            m_store->popDirectory();
        }
        return result;
    }

bool KisKraSaveVisitior::visit(KisPaintLayer *layer)
{
    //connect(*layer->paintDevice(), SIGNAL(ioProgress(qint8)), m_img, SLOT(slotIOProgress(qint8)));

    QString location = m_external ? QString::null : m_uri;
    location += m_name + QString("/layers/layer%1").arg(m_count);

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

    if (layer->paintDevice()->colorSpace()->profile()) {
        const KoColorProfile *profile = layer->paintDevice()->colorSpace()->profile();
        KisAnnotationSP annotation;
        if (profile)
        {
            const KoIccColorProfile* iccprofile = dynamic_cast<const KoIccColorProfile*>(profile);
            if (iccprofile and !iccprofile->rawData().isEmpty())
                annotation = new  KisAnnotation("icc", iccprofile->name(), iccprofile->rawData());
        }

        if (annotation) {
            // save layer profile
            location = m_external ? QString::null : m_uri;
            location += m_name + QString("/layers/layer%1").arg(m_count) + ".icc";

            if (m_store->open(location)) {
                m_store->write(annotation->annotation());
                m_store->close();
            }
        }
    }

//     if (layer->hasMask()) {
//         KisPaintDeviceSP mask = layer->getMask();

//         if (mask) {
//             // save layer profile
//             location = m_external ? QString::null : m_uri;
//             location += m_img->name() + QString("/layers/layer%1").arg(m_count) + ".mask";

//             if (m_store->open(location)) {
//                 if (!mask->write(m_store)) {
//                     mask->disconnect();
//                     m_store->close();
//                     return false;
//                 }

//                 m_store->close();
//             }
//         }
//    }

    m_count++;
    return true;
}

bool KisKraSaveVisitior::visit(KisGroupLayer *layer)
{
    return visitAllInverse( layer );
}

bool KisKraSaveVisitior::visit(KisAdjustmentLayer* layer)
{

    if (layer->selection()) {
        QString location = m_external ? QString::null : m_uri;
        location += m_name + QString("/layers/layer%1").arg(m_count) + ".selection";

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
        location += m_name + QString("/layers/layer%1").arg(m_count) + ".filterconfig";

        if (m_store->open(location)) {
            QString s = layer->filter()->toLegacyXML();
            m_store->write(s.toUtf8(), qstrlen(s.toUtf8()));
            m_store->close();
        }
    }
    m_count++;
    return true;
}

