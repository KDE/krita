/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <KoIcon.h>
#include <kis_icon.h>
#include <KoCompositeOpRegistry.h>

#include "kis_filter_mask.h"
#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include "kis_selection.h"
#include "kis_node.h"
#include "kis_node_visitor.h"
#include "kis_processing_visitor.h"
#include "kis_busy_progress_indicator.h"
#include "kis_painter.h"

struct KisFilterMask::Private
{
    bool needsTransparentPixels;
};

KisFilterMask::KisFilterMask(KisImageWSP image, const QString &name)
    : KisEffectMask(image, name)
    , KisNodeFilterInterface(0)
    , m_d(new Private())
{
    setCompositeOpId(COMPOSITE_COPY);
}

KisFilterMask::KisFilterMask(const KisFilterMask& rhs)
    : KisEffectMask(rhs)
    , KisNodeFilterInterface(rhs)
    , m_d(new Private())
{
    *m_d = *rhs.m_d;
}

KisFilterMask::~KisFilterMask() = default;

QIcon KisFilterMask::icon() const
{
    return KisIconUtils::loadIcon("filterMask");
}

void KisFilterMask::setFilter(KisFilterConfigurationSP filterConfig, bool checkCompareConfig)
{
    KisNodeFilterInterface::setFilter(filterConfig, checkCompareConfig);

    KisFilterSP filter = KisFilterRegistry::instance()->value(filterConfig->name());

    m_d->needsTransparentPixels = filter->needsTransparentPixels(filterConfig, colorSpace());
}

void KisFilterMask::notifyColorSpaceChanged()
{
    KisFilterConfigurationSP filterConfig = filter();
    KisFilterSP filter = KisFilterRegistry::instance()->value(filterConfig->name());

    m_d->needsTransparentPixels = filter->needsTransparentPixels(filterConfig, colorSpace());
}

QRect KisFilterMask::decorateRect(KisPaintDeviceSP &src,
                                  KisPaintDeviceSP &dst,
                                  const QRect & rc,
                                  PositionToFilthy maskPos,
                                  KisRenderPassFlags flags) const
{
    Q_UNUSED(maskPos);
    Q_UNUSED(flags);

    KisFilterConfigurationSP filterConfig = filter();

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(nodeProgressProxy(), rc);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(
        src != dst &&
            "KisFilterMask::decorateRect: "
            "src must be != dst, because we can't create transactions "
            "during merge, as it breaks reentrancy",
        rc);

    if (!filterConfig) {
        return QRect();
    }

    KisFilterSP filter =
        KisFilterRegistry::instance()->value(filterConfig->name());

    if (!filter) {
        warnKrita << "Could not retrieve filter \"" << filterConfig->name() << "\"";
        return QRect();
    }

    KIS_ASSERT_RECOVER_NOOP(this->busyProgressIndicator());
    this->busyProgressIndicator()->update();

    filter->process(src, dst, 0, rc, filterConfig.data(), 0);

    QRect r = filter->changedRect(rc, filterConfig.data(), dst->defaultBounds()->currentLevelOfDetail());
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

QRect KisFilterMask::extent() const
{
    KisNodeSP parentNode = parent();

    if (!parentNode) {
        return {};
    }

    QRect rect = KisEffectMask::extent();

    if (!m_d->needsTransparentPixels) {
        rect &= parentNode->extent();
    }

    return rect;
}

QRect KisFilterMask::exactBounds() const
{
    KisNodeSP parentNode = parent();

    if (!parentNode) {
        return {};
    }

    QRect rect = KisEffectMask::exactBounds();

    if (!m_d->needsTransparentPixels) {
        rect &= parentNode->exactBounds();
    }

    return rect;
}

QRect KisFilterMask::changeRect(const QRect &rect, PositionToFilthy pos) const
{
    /**
     * FIXME: This check of the emptiness should be done
     * on the higher/lower level
     */
    if (rect.isEmpty()) return rect;

    QRect filteredRect = rect;

    KisFilterConfigurationSP filterConfig = filter();
    if (filterConfig) {
        KisNodeSP parent = this->parent();
        const int lod = parent && parent->projection() ?
            parent->projection()->defaultBounds()->currentLevelOfDetail() : 0;

        KisFilterSP filter = KisFilterRegistry::instance()->value(filterConfig->name());
        filteredRect = filter->changedRect(rect, filterConfig.data(), lod);
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

    KisFilterConfigurationSP filterConfig = filter();
    if (!filterConfig) return rect;

    KisNodeSP parent = this->parent();
    const int lod = parent && parent->projection() ?
        parent->projection()->defaultBounds()->currentLevelOfDetail() : 0;

    KisFilterSP filter = KisFilterRegistry::instance()->value(filterConfig->name());

    /**
     * If we need some additional pixels even outside of a selection
     * for accurate layer filtering, we'll get them!
     * And no KisMask::needRect will prevent us from doing this! ;)
     * That's why simply we do not call KisMask::needRect here :)
     */
    return filter->neededRect(rect, filterConfig.data(), lod);
}

