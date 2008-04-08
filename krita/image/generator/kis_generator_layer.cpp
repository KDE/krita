/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org> 
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

#include "kis_generator_layer.h"

#include <QImage>

#include <kis_debug.h>
#include <kicon.h>

#include <klocale.h>

#include <KoCompositeOp.h>

#include "kis_debug.h"
#include "kis_group_layer.h"
#include "kis_image.h"
#include "kis_layer.h"

#include "kis_painter.h"
#include "kis_undo_adapter.h"
#include "kis_selection.h"
#include "kis_fill_painter.h"
#include "filter/kis_filter_configuration.h"
#include "generator/kis_generator_registry.h"
#include "generator/kis_generator.h"
#include "kis_pixel_selection.h"
#include "kis_datamanager.h"
#include "kis_node_visitor.h"
#include "kis_processing_information.h"

class KisGeneratorLayer::Private
{
public:
    bool showSelection;
    KisFilterConfiguration * filterConfig;
    KisSelectionSP selection;
    KisPaintDeviceSP paintDevice;
    KisPaintDeviceSP projection;
};

KisGeneratorLayer::KisGeneratorLayer(KisImageSP img, const QString &name, KisFilterConfiguration * kfc, KisSelectionSP selection)
    : KisLayer (img.data(), name, OPACITY_OPAQUE)
    , m_d( new Private() )
{
    Q_ASSERT(kfc);

    m_d->filterConfig = kfc;
    setSelection( selection );

    m_d->showSelection = true;

    update();
}

KisGeneratorLayer::KisGeneratorLayer(const KisGeneratorLayer& rhs)
    : KisLayer(rhs)
    , KisIndirectPaintingSupport(rhs)
    , m_d( new Private() )
{
    m_d->filterConfig = new KisFilterConfiguration(*rhs.m_d->filterConfig);
    if (rhs.m_d->selection) {
        m_d->selection = new KisSelection( *rhs.m_d->selection.data() );
        m_d->selection->setInterestedInDirtyness(true);
    }
    m_d->paintDevice = new KisPaintDevice( *rhs.m_d->paintDevice.data() );
    m_d->showSelection = false;
}


KisGeneratorLayer::~KisGeneratorLayer()
{
    delete m_d->filterConfig;
    delete m_d;
}

bool KisGeneratorLayer::allowAsChild( KisNodeSP node) const
{
    if ( node->inherits( "KisMask" ) )
       return true;
    else
       return false;
}


void KisGeneratorLayer::updateProjection(const QRect& rc)
{
    if ( !rc.isValid() ) return ;
    if ( !hasEffectMasks() ) return;
    if ( !m_d->paintDevice ) return;

    if ( !m_d->projection ) {
        m_d->projection = new KisPaintDevice( *m_d->paintDevice );
    }
    else {
        KisPainter gc( m_d->projection );
        gc.setCompositeOp( colorSpace()->compositeOp( COMPOSITE_COPY ) );
        gc.bitBlt( rc.topLeft(), m_d->paintDevice, rc);
    }

    applyEffectMasks( m_d->projection, rc );

}

KisPaintDeviceSP KisGeneratorLayer::projection() const
{
    if (m_d->projection)
        return m_d->projection;
    else
        return m_d->paintDevice;
}

KisPaintDeviceSP KisGeneratorLayer::paintDevice() const
{
    if (!m_d->selection) {
        m_d->selection = new KisSelection();
        KisPixelSelectionSP sel = m_d->selection->getOrCreatePixelSelection();
        sel->select(image()->bounds());
    }
    return m_d->selection;
}


QIcon KisGeneratorLayer::icon() const
{
    return KIcon("tool_filter");
}

KoDocumentSectionModel::PropertyList KisGeneratorLayer::sectionModelProperties() const
{
    KoDocumentSectionModel::PropertyList l = KisLayer::sectionModelProperties();
    l << KoDocumentSectionModel::Property(i18n("Generator"),
                                          KisGeneratorRegistry::instance()->value(generator()->name())->name());
    return l;
}

void KisGeneratorLayer::resetCache()
{
    m_d->paintDevice = new KisPaintDevice(image()->colorSpace(), name().toLatin1());
}

KisFilterConfiguration * KisGeneratorLayer::generator() const
{
    Q_ASSERT(m_d->filterConfig);
    return m_d->filterConfig;
}


void KisGeneratorLayer::setGenerator(KisFilterConfiguration * filterConfig)
{
    Q_ASSERT(filterConfig);
    m_d->filterConfig = filterConfig;
    update();
}


KisSelectionSP KisGeneratorLayer::selection() const
{
    return m_d->selection;
}

void KisGeneratorLayer::setSelection(KisSelectionSP selection)
{
    if (selection) {
        m_d->selection = new KisSelection(*selection.data());
        m_d->selection->updateProjection();
        m_d->selection->setInterestedInDirtyness(true);
    }
}


qint32 KisGeneratorLayer::x() const
{
    if (m_d->selection)
        return m_d->selection->x();
    else
        return 0;
}

void KisGeneratorLayer::setX(qint32 x)
{
    if (m_d->selection) {
        m_d->selection->setX(x);
        resetCache();
    }

}

qint32 KisGeneratorLayer::y() const
{
    if (m_d->selection)
        return m_d->selection->y();
    else
        return 0;
}

void KisGeneratorLayer::setY(qint32 y)
{
    if (m_d->selection) {
        m_d->selection->setY(y);
        resetCache();
    }
}

QRect KisGeneratorLayer::extent() const
{
    if (m_d->selection)
        return m_d->selection->selectedRect().intersected(image()->bounds());
    else if (image())
        return image()->bounds();
    else
        return QRect();
}

QRect KisGeneratorLayer::exactBounds() const
{
    if (m_d->selection)
        return m_d->selection->selectedExactRect().intersected(image()->bounds());
    else if (image())
        return image()->bounds();
    else
        return QRect();
}

bool KisGeneratorLayer::accept(KisNodeVisitor & v)
{
    return v.visit( this );
}

QImage KisGeneratorLayer::createThumbnail(qint32 w, qint32 h)
{
    if (m_d->paintDevice)
        return m_d->paintDevice->createThumbnail(w, h);
    else
        return QImage();
}

bool KisGeneratorLayer::showSelection() const { return m_d->showSelection; }

void KisGeneratorLayer::setSelection(bool b) { m_d->showSelection = b; }

void KisGeneratorLayer::update()
{

    KisGeneratorSP f = KisGeneratorRegistry::instance()->value( m_d->filterConfig->name() );
    if (!f) return;

    if (f->colorSpace())
        m_d->paintDevice = new KisPaintDevice( f->colorSpace(), name().toLatin1());
    else
        m_d->paintDevice = new KisPaintDevice( image()->colorSpace(), name().toLatin1());

    QRect tmpRc = exactBounds();
    
    KisProcessingInformation dstCfg(m_d->paintDevice, tmpRc.topLeft(), m_d->selection);
    m_d->filterConfig->setChannelFlags(channelFlags());
    f->generate(dstCfg, tmpRc.size(), m_d->filterConfig);
}


#include "kis_generator_layer.moc"
