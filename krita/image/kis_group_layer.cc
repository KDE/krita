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

#include <KoColorSpace.h>

#include "kis_types.h"
#include "kis_layer.h"

#include "kis_node_visitor.h"
#include "kis_debug.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_merge_visitor.h"
#include "kis_fill_painter.h"
#include "kis_projection.h"

class KisGroupLayer::Private
{
public:
    Private()
        : projection( 0 )
        , cacheProjection( true )
        , x( 0 )
        , y( 0 )
        {
        }

    KisPaintDeviceSP projection; // The cached composition of all
                                 // layers in this group
    bool cacheProjection;
    qint32 x;
    qint32 y;
};

KisGroupLayer::KisGroupLayer(KisImageWSP img, const QString &name, quint8 opacity) :
    KisLayer(img, name, opacity),
    m_d( new Private() )
{
    m_d->projection = new KisPaintDevice(this, img->colorSpace(), name.toLatin1());
    updateSettings();
}

KisGroupLayer::KisGroupLayer(const KisGroupLayer &rhs) :
    KisLayer(rhs),
    m_d( new Private() )
{
    m_d->projection = new KisPaintDevice(*rhs.m_d->projection.data());
    updateSettings();
}

KisGroupLayer::~KisGroupLayer()
{
    delete m_d;
}

bool KisGroupLayer::allowAsChild( KisNodeSP node) const
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
    m_d->cacheProjection = cfg.readEntry( "useProjections", true );
    emit settingsUpdated();
}

void KisGroupLayer::resetProjection(KisPaintDeviceSP to)
{
    if (to)
        m_d->projection = new KisPaintDevice(*to); /// XXX ### look into Copy on Write here (CoW)
    else
        m_d->projection = new KisPaintDevice(this, image()->colorSpace(), name().toLatin1());
}

bool KisGroupLayer::paintLayerInducesProjectionOptimization(KisPaintLayerSP l) const {
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

    for (uint i = 0; i < childCount(); ++i)
    {
        groupExtent |= (at( i ))->extent();
    }

    return groupExtent;
}

QRect KisGroupLayer::exactBounds() const
{

    QRect groupExactBounds;

    for (uint i = 0; i < childCount(); ++i)
    {
        groupExactBounds |= (at( i ))->exactBounds();
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

    for (uint i = 0; i < childCount(); ++i)
    {
        KisNodeSP layer = at( i );
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

    for (uint i = 0; i < childCount(); ++i)
    {
        KisNodeSP layer = at( i );
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
    m_d->projection = updateStrategy()->updateGroupLayerProjection( rc, m_d->projection );

}

#include "kis_group_layer.moc"
