/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
 *
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

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoCompositeOp.h>

#include "kis_filter_mask.h"
#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include "kis_selection.h"
#include "kis_processing_information.h"
#include "kis_node.h"
#include "kis_node_visitor.h"
#include "kis_node_progress_proxy.h"
#include "kis_transaction.h"
#include "kis_painter.h"

#include <KoUpdater.h>

class KRITAIMAGE_EXPORT KisFilterMask::Private
{
public:

    KisFilterConfiguration * filterConfig;
};

KisFilterMask::KisFilterMask()
        : KisEffectMask()
        , m_d(new Private())
{
    m_d->filterConfig = 0;
}

KisFilterMask::~KisFilterMask()
{
    delete m_d->filterConfig;
    delete m_d;
}

bool KisFilterMask::allowAsChild(KisNodeSP node) const
{
    Q_UNUSED(node);
    return false;
}

KisFilterMask::KisFilterMask(const KisFilterMask& rhs)
        : KisEffectMask(rhs)
        , KisNodeFilterInterface(rhs)
        , m_d(new Private())
{
    m_d->filterConfig = rhs.m_d->filterConfig;
}

KisFilterConfiguration * KisFilterMask::filter() const
{
    return m_d->filterConfig;
}

QIcon KisFilterMask::icon() const
{
    return KIcon("view-filter");
}

void KisFilterMask::setFilter(KisFilterConfiguration * filterConfig)
{
    Q_ASSERT(filterConfig);
    m_d->filterConfig = filterConfig;
}

QRect KisFilterMask::decorateRect(KisPaintDeviceSP &src,
                                  KisPaintDeviceSP &dst,
                                  const QRect & rc) const
{
    Q_ASSERT(nodeProgressProxy());

    if (!m_d->filterConfig) {
        warnKrita << "No filter configuration present";
        return QRect();
    }

    KisFilterSP filter =
        KisFilterRegistry::instance()->value(m_d->filterConfig->name());

    if (!filter) {
        warnKrita << "Could not retrieve filter \"" << m_d->filterConfig->name() << "\"";
        return QRect();
    }

    KisConstProcessingInformation srcInfo(src,  rc.topLeft(), selection());
    KisProcessingInformation dstInfo(dst, rc.topLeft(), 0);

    KoProgressUpdater updater(nodeProgressProxy());
    updater.start(100, filter->name());

    QPointer<KoUpdater> updaterPtr = updater.startSubtask();

    KisTransaction transaction("", dst);
    filter->process(srcInfo, dstInfo, rc.size(),
                    m_d->filterConfig, updaterPtr);

    updaterPtr->setProgress(100);

    return filter->changedRect(rc, m_d->filterConfig);
}

bool KisFilterMask::accept(KisNodeVisitor &v)
{
    return v.visit(this);
}

/**
 * FIXME: try to cache filter pointer inside a Private block
 */
QRect KisFilterMask::changeRect(const QRect& rect) const
{
    QRect filteredRect = rect;

    if (m_d->filterConfig) {
        KisFilterSP filter = KisFilterRegistry::instance()->value(m_d->filterConfig->name());
        filteredRect = filter->changedRect(rect, m_d->filterConfig);
    }

    /**
     * We can't paint outside a selection, that is why we call
     * KisMask::changeRect to crop actual change area in the end
     */
    filteredRect = KisMask::changeRect(filteredRect);

    /**
     * FIXME: Think over this solution
     * Union of rects means that changeRect returns NOT the rect
     * changed by this very layer, but an accumulated rect changed
     * by all underlying layers. Just take into account and change
     * documentation accordingly
     */
    return rect | filteredRect;
}

QRect KisFilterMask::needRect(const QRect& rect) const
{
    if (!m_d->filterConfig) return rect;
    KisFilterSP filter = KisFilterRegistry::instance()->value(m_d->filterConfig->name());

    /**
     * If we need some additional pixels even outside of a selection
     * for accurate layer filtering, we'll get them!
     * And no KisMask::needRect will prevent us from doing this! ;)
     * That's why simply we do not call KisMask::needRect here :)
     */
    return filter->neededRect(rect, m_d->filterConfig);
}

#include "kis_filter_mask.moc"
