/*
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_group_layer.h"

#include <kglobal.h>
#include <kicon.h>
#include <kconfiggroup.h>
#include <QImage>
#include <QDateTime>

#include <ksharedconfig.h>

#include <KoColorSpace.h>

#include "kis_types.h"
#include "kis_layer.h"
#include "kis_node_visitor.h"
#include "kis_debug.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_fill_painter.h"
#include "kis_projection.h"
#include "kis_paint_layer.h"
#include "kis_projection_update_strategy.h"
#include <KoProperties.h>
#include "kis_effect_mask.h"

class KisGroupLayer::Private
{
public:
    Private()
            : projection(0)
            , cacheProjection(true)
            , x(0)
            , y(0) {
    }

    KisPaintDeviceSP projection; // The cached composition of all
    KisPaintDeviceSP projectionUnfiltered; // The cached composition of all before filtering
    // layers in this group
    bool cacheProjection;
    qint32 x;
    qint32 y;
};

KisGroupLayer::KisGroupLayer(KisImageWSP img, const QString &name, quint8 opacity) :
        KisLayer(img, name, opacity),
        m_d(new Private())
{
    m_d->projection = new KisPaintDevice(this, img->colorSpace());
    updateSettings();
}

KisGroupLayer::KisGroupLayer(const KisGroupLayer &rhs) :
        KisLayer(rhs),
        m_d(new Private())
{
    m_d->projection = new KisPaintDevice(*rhs.m_d->projection.data());
    updateSettings();
}

KisGroupLayer::~KisGroupLayer()
{
    delete m_d;
}

bool KisGroupLayer::allowAsChild(KisNodeSP node) const
{
    Q_UNUSED(node);
    return true;
}


const KoColorSpace * KisGroupLayer::colorSpace() const
{
    // Due to virtual void resetProjection(KisPaintDeviceSP to =
    // 0), the colorspace of the group layer can be different from the
    // colorspace of the image. (XXX: is that desirable? BSAR)
    return m_d->projection->colorSpace();
}

KoColorSpace * KisGroupLayer::colorSpace()
{
    // Due to virtual void resetProjection(KisPaintDeviceSP to =
    // 0), the colorspace of the group layer can be different from the
    // colorspace of the image. (XXX: is that desirable? BSAR)
    return m_d->projection->colorSpace();
}

QIcon KisGroupLayer::icon() const
{
    return KIcon("folder");
}

void KisGroupLayer::updateSettings()
{
    KConfigGroup cfg = KGlobal::config()->group("");
    m_d->cacheProjection = cfg.readEntry("useProjections", true);
    emit settingsUpdated();
}

void KisGroupLayer::resetProjection(KisPaintDeviceSP to)
{
    if (to)
        m_d->projection = new KisPaintDevice(*to); /// XXX ### look into Copy on Write here (CoW)
    else if( *m_d->projection->colorSpace() == *image()->colorSpace())
        m_d->projection->clear();
    else
        m_d->projection = new KisPaintDevice(this, image()->colorSpace());
}

bool KisGroupLayer::paintLayerInducesProjectionOptimization(KisPaintLayerSP l) const
{
    if (!l) return false;
    if (!l->paintDevice()) return false;
    if (!(*l->paintDevice()->colorSpace() == *image()->colorSpace())) return false;
    if (!l->visible()) return false;
    if (l->opacity() != OPACITY_OPAQUE) return false;
    if (l->temporaryTarget()) return false;

    return true;
}

KisPaintDeviceSP KisGroupLayer::projection() const
{
    // We don't have a parent, and we've got only one child: abuse the child's
    // paint device as the projection if the child is visible
    if (parent().isNull() && childCount() == 1) {
        KisPaintLayer * l = dynamic_cast<KisPaintLayer*>(firstChild().data());
        if (l && paintLayerInducesProjectionOptimization(l)) {
            return l->projection();
        }
    }
    return m_d->projection;
}

KisPaintDeviceSP KisGroupLayer::paintDevice() const
{
    return 0;
}


QRect KisGroupLayer::extent() const
{
    QRect groupExtent;

    for (uint i = 0; i < childCount(); ++i) {
        groupExtent |= (at(i))->extent();
    }

    return groupExtent;
}

QRect KisGroupLayer::exactBounds() const
{

    QRect groupExactBounds;

    for (uint i = 0; i < childCount(); ++i) {
        groupExactBounds |= (at(i))->exactBounds();
    }

    return groupExactBounds;
}

bool KisGroupLayer::accept(KisNodeVisitor &v)
{
    return v.visit(this);
}


qint32 KisGroupLayer::x() const
{
    return m_d->x;
}

void KisGroupLayer::setX(qint32 x)
{
    qint32 delta = x - m_d->x;

    for (uint i = 0; i < childCount(); ++i) {
        KisNodeSP layer = at(i);
        layer->setX(layer->x() + delta);
    }
    m_d->x = x;
}

qint32 KisGroupLayer::y() const
{
    return m_d->y;
}

void KisGroupLayer::setY(qint32 y)
{
    qint32 delta = y - m_d->y;

    for (uint i = 0; i < childCount(); ++i) {
        KisNodeSP layer = at(i);
        layer->setY(layer->y() + delta);
    }

    m_d->y = y;
}

QImage KisGroupLayer::createThumbnail(qint32 w, qint32 h)
{
    return m_d->projection->createThumbnail(w, h);
}

void KisGroupLayer::updateProjection(const QRect & rc)
{
    QRect currentNeededRc = rc;
    if( childCount() == 0 )
    {
        m_d->projection->clear();
    } else {
        KoProperties props;
        props.setProperty("visible", true);
        QList<KisNodeSP> masks = childNodes(QStringList("KisEffectMask"), props);
        
        KisPaintDeviceSP source;
        
        if( masks.isEmpty() ) {
            source = m_d->projection;
            m_d->projectionUnfiltered = 0; // No masks, make sure this projection memory is freed
        } else {
            for( int i = masks.size() - 1; i >= 0 ; --i )
            {
                const KisEffectMask * effectMask = dynamic_cast<const KisEffectMask*>(masks.at(i).data());
                if (effectMask) {
                    currentNeededRc |= effectMask->neededRect( currentNeededRc );
                }
            }
            if( !m_d->projectionUnfiltered || !(*m_d->projectionUnfiltered->colorSpace() == *m_d->projection->colorSpace() ) ) {
                m_d->projectionUnfiltered = new KisPaintDevice(m_d->projection->colorSpace());
            }
            source = m_d->projectionUnfiltered;
        }
        
        source->clear(currentNeededRc); // needed when layers in the group aren't fully opaque
        source = updateStrategy()->updateGroupLayerProjection(currentNeededRc, m_d->projection);
    
        if (masks.size() > 0 ) {
            applyEffectMasks(source, m_d->projection, rc);
        }
    }
}

#include "kis_group_layer.moc"
