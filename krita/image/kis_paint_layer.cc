/*
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
 *  Copyright (c) 2006 Bart Coppens <kde@bartcoppens.be>
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

#include "kis_paint_layer.h"

#include <kis_debug.h>
#include <QIcon>
#include <QImage>
#include <QUndoCommand>
#include <QList>

#include <klocale.h>

#include "KoColorSpaceRegistry.h"
#include <KoColorSpace.h>
#include <KoColorProfile.h>
#include <KoCompositeOp.h>
#include <KoProperties.h>

#include "kis_image.h"
#include "kis_selection.h"
#include "kis_painter.h"
#include "kis_undo_adapter.h"
#include "kis_iterators_pixel.h"
#include "kis_paint_device.h"
#include "kis_datamanager.h"
#include "kis_effect_mask.h"
#include "kis_transparency_mask.h"
#include "kis_mask.h"
#include "kis_node_visitor.h"

class KisPaintLayer::Private
{
public:
    KisPaintDeviceSP paintDevice;
    KisPaintDeviceSP projection;
    KisPaintDeviceSP driedPaintDevice;
};

KisPaintLayer::KisPaintLayer(KisImageSP img, const QString& name, quint8 opacity, KisPaintDeviceSP dev)
    : KisLayer(img, name, opacity)
    , m_d(new Private())
{
    Q_ASSERT(img);
    Q_ASSERT(dev);
    m_d->paintDevice = dev;

}


KisPaintLayer::KisPaintLayer(KisImageSP img, const QString& name, quint8 opacity)
    : KisLayer(img, name, opacity)
    , m_d(new Private())
{
    Q_ASSERT(img);
    m_d->paintDevice = new KisPaintDevice(this, img->colorSpace());
}

KisPaintLayer::KisPaintLayer(KisImageSP img, const QString& name, quint8 opacity, const KoColorSpace * colorSpace)
    : KisLayer(img, name, opacity)
    , m_d(new Private())
{
//     Q_ASSERT(img);
    if (colorSpace == 0)
        colorSpace = img->colorSpace();
    Q_ASSERT(colorSpace);
    m_d->paintDevice = new KisPaintDevice(this, colorSpace);
}

KisPaintLayer::KisPaintLayer(const KisPaintLayer& rhs)
    : KisLayer(rhs)
    , KisIndirectPaintingSupport(rhs)
    , m_d(new Private)
{
    m_d->paintDevice = new KisPaintDevice(*rhs.m_d->paintDevice.data());
    m_d->paintDevice->setParentNode(this);
}

KisPaintLayer::~KisPaintLayer()
{
    delete m_d;
}

bool KisPaintLayer::allowAsChild(KisNodeSP node) const
{
    if (node->inherits("KisMask"))
        return true;
    else
        return false;
}

KisPaintDeviceSP KisPaintLayer::projection() const
{
    if (!hasEffectMasks() && !hasTemporaryTarget()) {
        return m_d->paintDevice;
    } else {
        return m_d->projection;
    }
}

void KisPaintLayer::updateProjection(const QRect & rc)
{
    if (!rc.isValid()) return ;
    if (!hasEffectMasks() && !hasTemporaryTarget()) return;
    if (!m_d->paintDevice) return;
    if (!visible()) return;

    dbgImage << name() << ": updateProjection " << rc;

    if (!m_d->projection || !( *m_d->projection->colorSpace() == *m_d->paintDevice->colorSpace() ) ) {
        m_d->projection = new KisPaintDevice(*m_d->paintDevice);
    } else {
        m_d->projection->setX(m_d->paintDevice->x());
        m_d->projection->setY(m_d->paintDevice->y());

        KisPainter gc(m_d->projection);
        gc.setCompositeOp(colorSpace()->compositeOp(COMPOSITE_COPY));
        gc.bitBlt(rc.topLeft(), m_d->paintDevice, rc);
    }

    KisPaintDeviceSP source = m_d->paintDevice;

    KoProperties props;
    props.setProperty("visible", true);
    QList<KisNodeSP> masks = childNodes(QStringList("KisEffectMask"), props);

    if( temporaryTarget() ) {
        if ( masks.size() == 0 && !previewMask()) {
            KisPainter gc(m_d->projection);
            gc.setCompositeOp(temporaryCompositeOp());
            gc.setOpacity(temporaryOpacity());
            gc.bitBlt( rc.topLeft(), temporaryTarget(), rc);
        }
        else {
            source = new KisPaintDevice( m_d->paintDevice->colorSpace() );
            QRect currentNeededRc = rc;
            for( int i = masks.size() - 1; i >= 0 ; --i )
            {
                const KisEffectMask * effectMask = dynamic_cast<const KisEffectMask*>(masks.at(i).data());
                if (effectMask) {
                    currentNeededRc |= effectMask->neededRect( currentNeededRc );
                }
            }

            KisPainter gc(source);
            gc.setCompositeOp(COMPOSITE_COPY);
            gc.setOpacity(temporaryOpacity());
            gc.bitBlt( currentNeededRc.left(), currentNeededRc.top(),
                       m_d->paintDevice, 
                       currentNeededRc.left(), currentNeededRc.top(), 
                       currentNeededRc.width(), currentNeededRc.height());
                    
            gc.setCompositeOp(temporaryCompositeOp());
            gc.bitBlt( currentNeededRc.left(), currentNeededRc.top(), 
                       temporaryTarget(), 
                       currentNeededRc.left(), currentNeededRc.top(), 
                       currentNeededRc.width(), currentNeededRc.height());
        }

    }
    if (masks.size() > 0 || previewMask()) {
        applyEffectMasks(source, m_d->projection, rc);
    }
}


QIcon KisPaintLayer::icon() const
{
    return QIcon();
}

KoDocumentSectionModel::PropertyList KisPaintLayer::sectionModelProperties() const
{
    KoDocumentSectionModel::PropertyList l = KisLayer::sectionModelProperties();
    l << KoDocumentSectionModel::Property(i18n("ColorSpace"), m_d->paintDevice->colorSpace()->name());
    if (const KoColorProfile *profile = m_d->paintDevice->colorSpace()->profile())
        l << KoDocumentSectionModel::Property(i18n("Profile"), profile->name());
    return l;
}

const KoColorSpace * KisPaintLayer::colorSpace() const
{
    return m_d->paintDevice->colorSpace();
}

QImage KisPaintLayer::createThumbnail(qint32 w, qint32 h)
{
    if (m_d->paintDevice)
        return m_d->paintDevice->createThumbnail(w, h);
    else
        return QImage();
}

bool KisPaintLayer::accept(KisNodeVisitor &v)
{
    return v.visit(this);
}

KisPaintDeviceSP KisPaintLayer::paintDevice() const
{
    return m_d->paintDevice;
}

KisPaintDeviceSP KisPaintLayer::original() const
{
    return m_d->paintDevice;
}


KisPaintDeviceSP KisPaintLayer::driedPaintDevice()
{
    return m_d->driedPaintDevice;
}

void KisPaintLayer::removeDriedPaintDevice()
{
    m_d->driedPaintDevice = 0;
}

qint32 KisPaintLayer::x() const
{
    if (m_d->paintDevice)
        return m_d->paintDevice->x();
    else
        return 0;
}

void KisPaintLayer::setX(qint32 x)
{
    if (m_d->paintDevice)
        m_d->paintDevice->setX(x);
}

qint32 KisPaintLayer::y() const
{
    if (m_d->paintDevice)
        return m_d->paintDevice->y();
    else
        return 0;
}
void KisPaintLayer::setY(qint32 y)
{
    if (m_d->paintDevice)
        m_d->paintDevice->setY(y);
}

QRect KisPaintLayer::extent() const
{
    if (m_d->paintDevice)
    {
        QRect r = m_d->paintDevice->extent();
        if( temporaryTarget() )
        {
            r |= temporaryTarget()->extent();
        }
        return r;
    }
    else
        return QRect();
}

QRect KisPaintLayer::exactBounds() const
{
    if (m_d->paintDevice)
    {
        QRect r = m_d->paintDevice->exactBounds();
        if( temporaryTarget() )
        {
            r |= temporaryTarget()->exactBounds();
        }
        return r;
    }
    else
        return QRect();
}


#include "kis_paint_layer.moc"
