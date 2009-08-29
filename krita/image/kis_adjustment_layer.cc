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

#include "kis_adjustment_layer.h"

#include <kicon.h>
#include <QImage>

#include <klocale.h>

#include "kis_debug.h"
#include "kis_group_layer.h"
#include "kis_image.h"
#include "kis_layer.h"

#include "kis_painter.h"
#include "kis_undo_adapter.h"
#include "kis_selection.h"
#include "kis_fill_painter.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include "filter/kis_filter.h"
#include "kis_pixel_selection.h"
#include "kis_datamanager.h"
#include "kis_node_visitor.h"

class KisAdjustmentLayer::Private
{
public:
    bool showSelection;
    KisFilterConfiguration * filterConfig;
    KisSelectionSP selection;
    KisPaintDeviceSP cachedPaintDevice;
};

KisAdjustmentLayer::KisAdjustmentLayer(KisImageSP img, const QString &name, KisFilterConfiguration * kfc, KisSelectionSP selection)
        : KisLayer(img.data(), name, OPACITY_OPAQUE)
        , m_d(new Private())
{
    m_d->filterConfig = kfc;
    setSelection(selection);

    m_d->cachedPaintDevice = new KisPaintDevice(img->colorSpace());
    m_d->showSelection = true;
    Q_ASSERT(m_d->cachedPaintDevice);
}

KisAdjustmentLayer::KisAdjustmentLayer(const KisAdjustmentLayer& rhs)
        : KisLayer(rhs)
        , KisIndirectPaintingSupport(rhs)
        , KisNodeFilterInterface(rhs)
        , m_d(new Private())
{
    m_d->filterConfig = new KisFilterConfiguration(*rhs.m_d->filterConfig);
    if (rhs.m_d->selection) {
        m_d->selection = new KisSelection(*rhs.m_d->selection.data());
        m_d->selection->setInterestedInDirtyness(true);
    }
    m_d->cachedPaintDevice = new KisPaintDevice(*rhs.m_d->cachedPaintDevice.data());
    m_d->showSelection = false;
}


KisAdjustmentLayer::~KisAdjustmentLayer()
{
    delete m_d->filterConfig;
    delete m_d;
}

bool KisAdjustmentLayer::allowAsChild(KisNodeSP node) const
{
    if (node->inherits("KisMask"))
        return true;
    else
        return false;
}


void KisAdjustmentLayer::updateProjection(const QRect& r)
{
    Q_UNUSED(r);
    // XXX: apply the masks to the selection data!

}

KisPaintDeviceSP KisAdjustmentLayer::projection() const
{
    return m_d->cachedPaintDevice;
}

KisPaintDeviceSP KisAdjustmentLayer::paintDevice() const
{
    return m_d->selection;
}


QIcon KisAdjustmentLayer::icon() const
{
    return KIcon("tool_filter");
}

KoDocumentSectionModel::PropertyList KisAdjustmentLayer::sectionModelProperties() const
{
    KoDocumentSectionModel::PropertyList l = KisLayer::sectionModelProperties();
    if (filter())
        l << KoDocumentSectionModel::Property(i18n("Filter"), KisFilterRegistry::instance()->value(filter()->name())->name());
    return l;
}

void KisAdjustmentLayer::resetCache()
{
    if( *m_d->cachedPaintDevice->colorSpace() == *image()->colorSpace())
        m_d->cachedPaintDevice->clear();
    else
        m_d->cachedPaintDevice = new KisPaintDevice(image()->colorSpace());
}

KisFilterConfiguration * KisAdjustmentLayer::filter() const
{
    return m_d->filterConfig;
}


void KisAdjustmentLayer::setFilter(KisFilterConfiguration * filterConfig)
{
    m_d->filterConfig = filterConfig;
}

void KisAdjustmentLayer::setDirty()
{
    if(selection())
    {
        KisLayer::setDirty(selection()->selectedExactRect());
    } else {
        KisLayer::setDirty();
    }
}

KisSelectionSP KisAdjustmentLayer::selection() const
{
    return m_d->selection;
}

void KisAdjustmentLayer::setSelection(KisSelectionSP selection)
{
    if (selection) {
        m_d->selection = new KisSelection(*selection.data());
    } else {
        m_d->selection = new KisSelection();
        m_d->selection->getOrCreatePixelSelection()->select(image()->bounds());
    }
    m_d->selection->updateProjection();
    m_d->selection->setInterestedInDirtyness(true);
}


qint32 KisAdjustmentLayer::x() const
{
    if (m_d->selection)
        return m_d->selection->x();
    else
        return 0;
}

void KisAdjustmentLayer::setX(qint32 x)
{
    if (m_d->selection) {
        m_d->selection->setX(x);
        resetCache();
    }

}

qint32 KisAdjustmentLayer::y() const
{
    if (m_d->selection)
        return m_d->selection->y();
    else
        return 0;
}

void KisAdjustmentLayer::setY(qint32 y)
{
    if (m_d->selection) {
        m_d->selection->setY(y);
        resetCache();
    }
}

QRect KisAdjustmentLayer::extent() const
{
    if (m_d->selection)
        return m_d->selection->selectedRect();
    else if (image())
        return image()->bounds();
    else
        return QRect();
}

QRect KisAdjustmentLayer::exactBounds() const
{
    if (m_d->selection)
        return m_d->selection->selectedExactRect();
    else if (image())
        return image()->bounds();
    else
        return QRect();
}

bool KisAdjustmentLayer::accept(KisNodeVisitor & v)
{
    return v.visit(this);
}

QImage KisAdjustmentLayer::createThumbnail(qint32 w, qint32 h)
{
    if (!selection())
        return QImage();

    int srcw, srch;
    if (image()) {
        srcw = image()->width();
        srch = image()->height();
    } else {
        const QRect e = extent();
        srcw = e.width();
        srch = e.height();
    }

    if (w > srcw) {
        w = srcw;
        h = qint32(double(srcw) / w * h);
    }
    if (h > srch) {
        h = srch;
        w = qint32(double(srch) / h * w);
    }

    if (srcw > srch)
        h = qint32(double(srch) / srcw * w);
    else if (srch > srcw)
        w = qint32(double(srcw) / srch * h);

    QColor c;
    QImage img(w, h, QImage::Format_RGB32);

    for (qint32 y = 0; y < h; ++y) {
        qint32 iY = (y * srch) / h;
        for (qint32 x = 0; x < w; ++x) {
            qint32 iX = (x * srcw) / w;
            m_d->selection->pixel(iX, iY, &c);
            quint8 opacity = c.alpha();
            img.setPixel(x, y, qRgb(opacity, opacity, opacity));
        }
    }

    return img;
}

KisPaintDeviceSP KisAdjustmentLayer::cachedPaintDevice()
{
    return m_d->cachedPaintDevice;
}
bool KisAdjustmentLayer::showSelection() const
{
    return m_d->showSelection;
}
void KisAdjustmentLayer::setSelection(bool b)
{
    m_d->showSelection = b;
}

#include "kis_adjustment_layer.moc"
