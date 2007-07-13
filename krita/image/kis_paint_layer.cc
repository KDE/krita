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

#include <kdebug.h>
#include <QIcon>
#include <QImage>
#include <QUndoCommand>
#include <QList>

#include <klocale.h>

#include <KoColorSpace.h>
#include <KoColorProfile.h>

#include "kis_debug_areas.h"
#include "kis_image.h"
#include "kis_paint_layer.h"
#include "kis_selection.h"
#include "kis_painter.h"
#include "kis_undo_adapter.h"
#include "kis_iterators_pixel.h"
#include "kis_paint_device.h"
#include "kis_meta_registry.h"
#include "KoColorSpaceRegistry.h"
#include "kis_datamanager.h"
#include "kis_undo_adapter.h"
#include "kis_effect_mask.h"
#include "kis_transparency_mask.h"
#include "kis_painterly_overlay.h"
#include "kis_mask.h"

class KisPaintLayer::Private
{
public:
    KisPaintDeviceSP paintDevice;
    KisPaintDeviceSP projection;
    KisPaintDeviceSP driedPaintDevice;
    KisPainterlyOverlaySP painterlyOverlay;
};

KisPaintLayer::KisPaintLayer(KisImageSP img, const QString& name, quint8 opacity, KisPaintDeviceSP dev)
    : super(img, name, opacity)
    , m_d( new Private() )
{
    Q_ASSERT(img);
    Q_ASSERT(dev);
    m_d->paintDevice = dev;
    m_d->projection = 0;

    init();
}


KisPaintLayer::KisPaintLayer(KisImageSP img, const QString& name, quint8 opacity)
    : super(img, name, opacity)
    , m_d( new Private() )
{
    Q_ASSERT(img);
    m_d->paintDevice = new KisPaintDevice(this, img->colorSpace(), name);
    m_d->projection = 0;
    init();
}

KisPaintLayer::KisPaintLayer(KisImageSP img, const QString& name, quint8 opacity, KoColorSpace * colorSpace)
    : super(img, name, opacity)
    , m_d( new Private() )
{
    Q_ASSERT(img);
    Q_ASSERT(colorSpace);
    m_d->paintDevice = new KisPaintDevice(this, colorSpace, name);
    m_d->projection = 0;
    init();
}

KisPaintLayer::KisPaintLayer(const KisPaintLayer& rhs) :
    KisLayer(rhs), m_d (new Private), KisIndirectPaintingSupport(rhs)
{
    m_d->paintDevice = new KisPaintDevice( *rhs.m_d->paintDevice.data() );
    m_d->projection = 0;


    init();
}

KisPaintLayer::~KisPaintLayer()
{
    delete m_d;
}


void KisPaintLayer::init()
{
    m_d->paintDevice->startBackgroundFilters();
    connect( m_d->paintDevice.data(), SIGNAL( colorSpaceChanged( KoColorSpace* ) ), this, SLOT( slotColorSpaceChanged() ) );
    connect( m_d->paintDevice.data(), SIGNAL( profileChanged( KoColorProfile* ) ), this, SLOT( slotColorSpaceChanged() ) );
    connect( m_d->paintDevice.data(), SIGNAL( dirtied() ), this, SLOT( setDirty() ) );
    connect( m_d->paintDevice.data(), SIGNAL( dirtied( const QRect & ) ), this, SLOT( setDirty( const QRect &) ) );
    connect( m_d->paintDevice.data(), SIGNAL( dirtied( const QRegion & ) ), this, SLOT( setDirty( const QRegion & ) ) );
}


KisPaintDeviceSP KisPaintLayer::projection() const
{
    if ( !hasEffectMasks() )
        return m_d->paintDevice;
    else {
        Q_ASSERT( m_d->projection );
        return m_d->projection;
    }
}

void KisPaintLayer::updateProjection(const QRect & rc)
{

    Q_ASSERT( rc.isValid() );
    Q_ASSERT( isDirty( rc ) );
    Q_ASSERT( hasEffectMasks() );
    Q_ASSERT( m_d->paintDevice );

    if ( !rc.isValid() ) return ;
    if ( !isDirty( rc ) ) return;
    if ( !hasEffectMasks() ) return;
    if ( !m_d->paintDevice ) return;

    if ( !m_d->projection ) {
        m_d->projection = new KisPaintDevice( *m_d->paintDevice );
    }
    else {
        // Clean up the area before we re-apply the masks.
        KisPainter gc( m_d->projection );
        gc.setCompositeOp( colorSpace()->compositeOp( COMPOSITE_COPY ) );
        gc.bitBlt( rc.topLeft(), m_d->paintDevice, rc );
    }

    applyEffectMasks( m_d->projection, rc );
}


QIcon KisPaintLayer::icon() const
{
    return QIcon();
}

KoDocumentSectionModel::PropertyList KisPaintLayer::properties() const
{
    KoDocumentSectionModel::PropertyList l = super::properties();
    l << KoDocumentSectionModel::Property(i18n("Colorspace"), m_d->paintDevice->colorSpace()->name());
    if( KoColorProfile *profile = m_d->paintDevice->colorSpace()->profile() )
        l << KoDocumentSectionModel::Property(i18n("Profile"), profile->name());
    return l;
}

KisLayerSP KisPaintLayer::clone() const
{
    return KisLayerSP(new KisPaintLayer(*this));
}


KoColorSpace * KisPaintLayer::colorSpace()
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

/// Returns the paintDevice that accompanies this layer
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

KisPainterlyOverlaySP KisPaintLayer::painterlyOverlay()
{
    return m_d->painterlyOverlay;
}

void KisPaintLayer::createPainterlyOverlay()
{
    m_d->painterlyOverlay = new KisPainterlyOverlay();
}

void KisPaintLayer::removePainterlyOverlay()
{
    m_d->painterlyOverlay = 0;
}


qint32 KisPaintLayer::x() const
{
    if (m_d->paintDevice)
        return m_d->paintDevice->getX();
    else
        return 0;
}

void KisPaintLayer::setX(qint32 x)
{
    if (m_d->paintDevice)
        m_d->paintDevice->setX(x);
}

qint32 KisPaintLayer::y() const {
    if (m_d->paintDevice)
        return m_d->paintDevice->getY();
    else
        return 0;
}
void KisPaintLayer::setY(qint32 y) {
    if (m_d->paintDevice)
        m_d->paintDevice->setY(y);
}

QRect KisPaintLayer::extent() const {
    if (m_d->paintDevice)
        return m_d->paintDevice->extent();
    else
        return QRect();
}

QRect KisPaintLayer::exactBounds() const {
    if (m_d->paintDevice)
        return m_d->paintDevice->exactBounds();
    else
        return QRect();
}

void KisPaintLayer::slotColorSpaceChanged()
{
    notifyPropertyChanged();
}


void KisPaintLayer::setDirty() {
    QRect rc = extent();
    super::setDirty(rc);
}

void KisPaintLayer::setDirty(const QRect & rect) {
    super::setDirty(rect);
}

void KisPaintLayer::setDirty(const QRegion & region) {
    super::setDirty(region);
}



#include "kis_paint_layer.moc"
