/*
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2009 Dmitry Kazakov <dimula73@gmail.com>
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

#include <KoColorSpace.h>

#include "kis_types.h"
#include "kis_node_visitor.h"
#include "kis_debug.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_projection_update_strategy.h"


class KisGroupLayer::Private
{
public:
    Private()
            : paintDevice(0)
            , x(0)
            , y(0) {
    }

    KisPaintDeviceSP paintDevice;
    qint32 x;
    qint32 y;
};

KisGroupLayer::KisGroupLayer(KisImageWSP img, const QString &name, quint8 opacity) :
        KisLayer(img, name, opacity),
        m_d(new Private())
{
    m_d->paintDevice = new KisPaintDevice(this, img->colorSpace());
}

KisGroupLayer::KisGroupLayer(const KisGroupLayer &rhs) :
        KisLayer(rhs),
        m_d(new Private())
{
    m_d->paintDevice = new KisPaintDevice(*rhs.m_d->paintDevice.data());
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
    return m_d->paintDevice->colorSpace();
}

void KisGroupLayer::setColorSpace(const KoColorSpace* colorSpace, KoColorConversionTransformation::Intent renderingIntent) const
{
    m_d->paintDevice->convertTo(colorSpace, renderingIntent);
}

QIcon KisGroupLayer::icon() const
{
    return KIcon("folder");
}

void KisGroupLayer::resetCache(const KoColorSpace *colorSpace)
{
    if(!colorSpace)
        colorSpace = image()->colorSpace();

    if(!m_d->paintDevice ||
       !(*m_d->paintDevice->colorSpace() == *colorSpace)) {

        m_d->paintDevice = new KisPaintDevice(colorSpace);
    }
    else {
        m_d->paintDevice->clear();
    }
}

KisPaintDeviceSP KisGroupLayer::tryObligeChild() const
{
    KisPaintDeviceSP retval;

    if(parent().isNull() && childCount() == 1) {
        const KisLayer *child = dynamic_cast<KisLayer*>(firstChild().data());

        if(child &&
           child->projection() &&
           child->visible() &&
           child->opacity() == OPACITY_OPAQUE &&
           *child->projection()->colorSpace() == *colorSpace()) {

            retval = child->projection();
        }
    }

    return retval;
}

KisPaintDeviceSP KisGroupLayer::paintDevice() const
{
    return 0;
}

KisPaintDeviceSP KisGroupLayer::original() const
{
    /**
     * We are too lazy! Let's our children work for us.
     * Try to use children's paintDevice if it's the only
     * one in stack and meets some conditions
     */

    KisPaintDeviceSP childOriginal = tryObligeChild();
    return childOriginal ? childOriginal : m_d->paintDevice;
}

QRect KisGroupLayer::repaintOriginal(KisPaintDeviceSP original,
                                     const QRect& rect)
{
    /**
     * FIXME: A bit of dirty hack
     */
    if(original == tryObligeChild())
        return rect;

    /**
     * FIXME: A temporary crunch for being able to work with
     * a top-down update strategy
     */
    original->clear(rect);
    (void) updateStrategy()->updateGroupLayerProjection(rect, original);

    return rect;


    /**
     * Everything should have been prepared by KisBottomUpUpdateStrategy,
     * so do nothing
     */
//    Q_UNUSED(original);
//    return rect;
}

bool KisGroupLayer::accept(KisNodeVisitor &v)
{
    return v.visit(this);
}


qint32 KisGroupLayer::x() const
{
    return m_d->x;
}

qint32 KisGroupLayer::y() const
{
    return m_d->y;
}

void KisGroupLayer::setX(qint32 x)
{
    qint32 numChildren = childCount();

    qint32 delta = x - m_d->x;
    for (qint32 i = 0; i < numChildren; ++i) {
        KisNodeSP layer = at(i);
        layer->setX(layer->x() + delta);
    }
    m_d->x = x;
}

void KisGroupLayer::setY(qint32 y)
{
    qint32 numChildren = childCount();

    qint32 delta = y - m_d->y;
    for (qint32 i = 0; i < numChildren; ++i) {
        KisNodeSP layer = at(i);
        layer->setY(layer->y() + delta);
    }
    m_d->y = y;
}

/* we can measure the same value with m_d->paintDevice->extent(), right? *
Then it's better to use KisLayer's implementation

QRect KisGroupLayer::extent() const
{
    QRect groupExtent;

    qint32 numChidren = childCount();

    for (qint32 i = 0; i < numChildren; ++i) {
        groupExtent |= (at(i))->extent();
    }

    return groupExtent;
}

QRect KisGroupLayer::exactBounds() const
{
<<<<<<< HEAD:krita/image/kis_group_layer.cc

    QRect groupExactBounds;

    for (uint i = 0; i < childCount(); ++i) {
        groupExactBounds |= (at(i))->exactBounds();
=======
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
>>>>>>> master:krita/image/kis_group_layer.cc
    }

    return groupExactBounds;
}
*/

#include "kis_group_layer.moc"
