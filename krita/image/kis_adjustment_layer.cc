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

#include <kdebug.h>
#include <kicon.h>
#include <QImage>

#include <klocale.h>

#include "kis_debug_areas.h"
#include "kis_group_layer.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_painter.h"
#include "kis_undo_adapter.h"
#include "kis_selection.h"
#include "kis_fill_painter.h"
#include "kis_filter_configuration.h"
#include "kis_filter_registry.h"
#include "kis_filter.h"

KisAdjustmentLayer::KisAdjustmentLayer(KisImageSP img, const QString &name, KisFilterConfiguration * kfc, KisSelectionSP selection)
    : KisLayer (img.data(), name, OPACITY_OPAQUE)
{
    m_filterConfig = kfc;
    setSelection( selection );

    m_cachedPaintDev = new KisPaintDevice( img->colorSpace(), name.toLatin1());
    m_showSelection = true;
    Q_ASSERT(m_cachedPaintDev);
}

KisAdjustmentLayer::KisAdjustmentLayer(const KisAdjustmentLayer& rhs)
    : KisLayer(rhs), KisIndirectPaintingSupport(rhs)
{
    m_filterConfig = new KisFilterConfiguration(*rhs.m_filterConfig);
    if (rhs.m_selection) {
        m_selection = new KisSelection( *rhs.m_selection.data() );
//        m_selection->setParentLayer(this);
        m_selection->setInterestedInDirtyness(true);
        if (!m_selection->hasSelection())
            m_selection->setSelection(m_selection);
    }
    m_cachedPaintDev = new KisPaintDevice( *rhs.m_cachedPaintDev.data() );
    m_showSelection = false;
}


KisAdjustmentLayer::~KisAdjustmentLayer()
{
    delete m_filterConfig;
}

void KisAdjustmentLayer::updateProjection(const QRect& r)
{

}

KisPaintDeviceSP KisAdjustmentLayer::projection() const
{
    return m_cachedPaintDev;
}

KisPaintDeviceSP KisAdjustmentLayer::paintDevice() const
{
    return m_selection;
}


QIcon KisAdjustmentLayer::icon() const
{
    return KIcon("tool_filter");
}

KoDocumentSectionModel::PropertyList KisAdjustmentLayer::properties() const
{
    KoDocumentSectionModel::PropertyList l = KisLayer::properties();
    l << KoDocumentSectionModel::Property(i18n("Filter"), KisFilterRegistry::instance()->value(filter()->name())->name());
    return l;
}

KisLayerSP KisAdjustmentLayer::clone() const
{
    return KisLayerSP(new KisAdjustmentLayer(*this));
}


void KisAdjustmentLayer::resetCache()
{
    m_cachedPaintDev = new KisPaintDevice(image()->colorSpace(), name().toLatin1());
}

KisFilterConfiguration * KisAdjustmentLayer::filter() const
{
    Q_ASSERT(m_filterConfig);
    return m_filterConfig;
}


void KisAdjustmentLayer::setFilter(KisFilterConfiguration * filterConfig)
{
    Q_ASSERT(filterConfig);
    m_filterConfig = filterConfig;
    notifyPropertyChanged();
}


KisSelectionSP KisAdjustmentLayer::selection() const
{
    return m_selection;
}

void KisAdjustmentLayer::setSelection(KisSelectionSP selection)
{
    m_selection = new KisSelection();
    KisFillPainter gc(KisPaintDeviceSP(m_selection.data()));
    KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();

    if (selection) {
        gc.bitBlt(0, 0, cs->compositeOp(COMPOSITE_COPY), KisPaintDeviceSP(selection.data()),
                  0, 0, image()->bounds().width(), image()->bounds().height());
    } else {
        gc.fillRect(image()->bounds(), KoColor(Qt::white, cs), MAX_SELECTED);
    }

    gc.end();

//    m_selection->setParentLayer(this);
    m_selection->setInterestedInDirtyness(true);

    if (!m_selection->hasSelection())
        m_selection->setSelection(m_selection);
}


qint32 KisAdjustmentLayer::x() const
{
    if (m_selection)
        return m_selection->getX();
    else
        return 0;
}

void KisAdjustmentLayer::setX(qint32 x)
{
    if (m_selection) {
        m_selection->setX(x);
        resetCache();
    }

}

qint32 KisAdjustmentLayer::y() const
{
    if (m_selection)
        return m_selection->getY();
    else
        return 0;
}

void KisAdjustmentLayer::setY(qint32 y)
{
    if (m_selection) {
        m_selection->setY(y);
        resetCache();
    }
}

QRect KisAdjustmentLayer::extent() const
{
    if (m_selection)
        return m_selection->selectedRect();
    else if (image())
        return image()->bounds();
    else
        return QRect();
}

QRect KisAdjustmentLayer::exactBounds() const
{
    if (m_selection)
        return m_selection->selectedExactRect();
    else if (image())
        return image()->bounds();
    else
        return QRect();
}

bool KisAdjustmentLayer::accept(KisLayerVisitor & v)
{
    return v.visit( this );
}

QImage KisAdjustmentLayer::createThumbnail(qint32 w, qint32 h)
{
    if (!selection())
        return QImage();

    int srcw, srch;
    if( image() )
    {
        srcw = image()->width();
        srch = image()->height();
    }
    else
    {
        const QRect e = extent();
        srcw = e.width();
        srch = e.height();
    }

    if (w > srcw)
    {
        w = srcw;
        h = qint32(double(srcw) / w * h);
    }
    if (h > srch)
    {
        h = srch;
        w = qint32(double(srch) / h * w);
    }

    if (srcw > srch)
        h = qint32(double(srch) / srcw * w);
    else if (srch > srcw)
        w = qint32(double(srcw) / srch * h);

    QColor c;
    quint8 opacity;
    QImage img(w, h, QImage::Format_RGB32);

    for (qint32 y=0; y < h; ++y) {
        qint32 iY = (y * srch ) / h;
        for (qint32 x=0; x < w; ++x) {
            qint32 iX = (x * srcw ) / w;
            m_selection->pixel(iX, iY, &c, &opacity);
            img.setPixel(x, y, qRgb(opacity, opacity, opacity));
        }
    }

    return img;
}

#include "kis_adjustment_layer.moc"
