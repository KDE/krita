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

#include <KoIcon.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoCompositeOp.h>

#include "kis_layer.h"
#include "kis_filter_mask.h"
#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include "kis_selection.h"
#include "kis_processing_information.h"
#include "kis_node.h"
#include "kis_node_visitor.h"
#include "kis_processing_visitor.h"
#include "kis_node_progress_proxy.h"
#include "kis_transaction.h"
#include "kis_painter.h"

#include <KoUpdater.h>
#include <QMutex>

struct KisFilterMask::Private
{
public:

    KisFilterConfiguration * filterConfig;
    QMutex filterConfigMutex;
};

KisFilterMask::KisFilterMask()
        : KisEffectMask()
        , m_d(new Private())
{
    m_d->filterConfig = 0;
    setCompositeOp(COMPOSITE_COPY);
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
    m_d->filterConfig = KisFilterRegistry::instance()->cloneConfiguration(rhs.m_d->filterConfig);
}

KisFilterConfiguration * KisFilterMask::filter() const
{
    return m_d->filterConfig;
}

QIcon KisFilterMask::icon() const
{
    return koIcon("view-filter");
}

void KisFilterMask::setFilter(KisFilterConfiguration * filterConfig)
{
    QMutexLocker l(&m_d->filterConfigMutex);
    Q_ASSERT(filterConfig);
    delete m_d->filterConfig;
    m_d->filterConfig = KisFilterRegistry::instance()->cloneConfiguration(filterConfig);
    if (parent() && parent()->inherits("KisLayer")) {
        m_d->filterConfig->setChannelFlags(qobject_cast<KisLayer*>(parent().data())->channelFlags());
    }
}

QRect KisFilterMask::decorateRect(KisPaintDeviceSP &src,
                                  KisPaintDeviceSP &dst,
                                  const QRect & rc) const
{
    Q_ASSERT(nodeProgressProxy());
    Q_ASSERT_X(src != dst, "KisFilterMask::decorateRect",
               "src must be != dst, because we cant create transactions "
               "during merge, as it breaks reentrancy");

    if (!m_d->filterConfig) {
        warnKrita << "No filter configuration present";
        return QRect();
    }
    KisFilterConfiguration* config;
    {
        QMutexLocker l(&m_d->filterConfigMutex);
        config = KisFilterRegistry::instance()->cloneConfiguration(m_d->filterConfig);
    }

    KisFilterSP filter =
        KisFilterRegistry::instance()->value(config->name());

    if (!filter) {
        warnKrita << "Could not retrieve filter \"" << config->name() << "\"";
        return QRect();
    }

    KoProgressUpdater updater(nodeProgressProxy());
    updater.start(100, filter->name());

    QPointer<KoUpdater> updaterPtr = updater.startSubtask();

    filter->process(src, dst, 0, rc, config, updaterPtr);

    updaterPtr->setProgress(100);

    QRect r = filter->changedRect(rc, config);
    delete config;
    return r;
}

bool KisFilterMask::accept(KisNodeVisitor &v)
{
    return v.visit(this);
}

void KisFilterMask::accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter)
{
    return visitor.visit(this, undoAdapter);
}

/**
 * FIXME: try to cache filter pointer inside a Private block
 */
QRect KisFilterMask::changeRect(const QRect &rect, PositionToFilthy pos) const
{
    /**
     * FIXME: This check of the emptiness should be done
     * on the higher/lower level
     */
    if(rect.isEmpty()) return rect;

    QRect filteredRect = rect;

    if (m_d->filterConfig) {
        KisFilterSP filter = KisFilterRegistry::instance()->value(m_d->filterConfig->name());
        filteredRect = filter->changedRect(rect, m_d->filterConfig);
    }

    /**
     * We can't paint outside a selection, that is why we call
     * KisMask::changeRect to crop actual change area in the end
     */
    filteredRect = KisMask::changeRect(filteredRect, pos);
    /**
     * FIXME: Think over this solution
     * Union of rects means that changeRect returns NOT the rect
     * changed by this very layer, but an accumulated rect changed
     * by all underlying layers. Just take into account and change
     * documentation accordingly
     */
    return rect | filteredRect;
}

QRect KisFilterMask::needRect(const QRect& rect, PositionToFilthy pos) const
{
    Q_UNUSED(pos);

    /**
     * FIXME: This check of the emptiness should be done
     * on the higher/lower level
     */
    if(rect.isEmpty()) return rect;
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
