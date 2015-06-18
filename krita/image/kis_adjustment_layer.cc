/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 C. Boemann <cbo@boemann.dk>
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

#include <klocale.h>
#include "kis_debug.h"

#include <KoIcon.h>
#include <KoCompositeOpRegistry.h>

#include "kis_image.h"
#include "kis_selection.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include "filter/kis_filter.h"
#include "kis_node_visitor.h"
#include "kis_processing_visitor.h"


KisAdjustmentLayer::KisAdjustmentLayer(KisImageWSP image,
                                       const QString &name,
                                       KisFilterConfiguration *kfc,
                                       KisSelectionSP selection)
    : KisSelectionBasedLayer(image.data(), name, selection, kfc)
{
    // by default Adjustment Layers have a copy composition,
    // which is more natural for users
    // https://bugs.kde.org/show_bug.cgi?id=324505
    // https://bugs.kde.org/show_bug.cgi?id=294122
    // demand the opposite from each other...
    setCompositeOp(COMPOSITE_COPY);
    setUseSelectionInProjection(false);
}

KisAdjustmentLayer::KisAdjustmentLayer(const KisAdjustmentLayer& rhs)
        : KisSelectionBasedLayer(rhs)
{
}


KisAdjustmentLayer::~KisAdjustmentLayer()
{
}

void KisAdjustmentLayer::setFilter(KisFilterConfiguration *filterConfig)
{
    filterConfig->setChannelFlags(channelFlags());
    KisSelectionBasedLayer::setFilter(filterConfig);
}

QRect KisAdjustmentLayer::incomingChangeRect(const QRect &rect) const
{
    KisSafeFilterConfigurationSP filterConfig = filter();

    QRect filteredRect = rect;

    if (filterConfig) {
        KisFilterSP filter = KisFilterRegistry::instance()->value(filterConfig->name());
        filteredRect = filter->changedRect(rect, filterConfig.data());
    }

    /**
     * We can't paint outside a selection, that is why we call
     * KisSelectionBasedLayer::cropChangeRectBySelection to crop
     * actual change area in the end.
     */
    filteredRect = cropChangeRectBySelection(filteredRect);

    return filteredRect;
}

QRect KisAdjustmentLayer::needRect(const QRect& rect, PositionToFilthy pos) const
{
    Q_UNUSED(pos);

    KisSafeFilterConfigurationSP filterConfig = filter();
    if (!filterConfig) return rect;
    KisFilterSP filter = KisFilterRegistry::instance()->value(filterConfig->name());

    /**
     * If we need some additional pixels even outside of a selection
     * for accurate layer filtering, we'll get them!
     * And no KisSelectionBasedLayer::needRect will prevent us
     * from doing this! ;)
     * That's why simply we do not call
     * KisSelectionBasedLayer::needRect here :)
     */
    return filter->neededRect(rect, filterConfig.data());
}

bool KisAdjustmentLayer::accept(KisNodeVisitor & v)
{
    return v.visit(this);
}

void KisAdjustmentLayer::accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter)
{
    return visitor.visit(this, undoAdapter);
}

QIcon KisAdjustmentLayer::icon() const
{
    return koIcon("view-filter");
}

KisDocumentSectionModel::PropertyList KisAdjustmentLayer::sectionModelProperties() const
{
    KisSafeFilterConfigurationSP filterConfig = filter();
    KisDocumentSectionModel::PropertyList l = KisLayer::sectionModelProperties();
    if (filterConfig)
        l << KisDocumentSectionModel::Property(i18nc("property of a filter layer, noun", "Filter"), KisFilterRegistry::instance()->value(filterConfig->name())->name());
    return l;
}

void KisAdjustmentLayer::setChannelFlags(const QBitArray & channelFlags)
{
    KisSafeFilterConfigurationSP filterConfig = filter();

    if (filterConfig) {
        filterConfig->setChannelFlags(channelFlags);
    }
    KisLayer::setChannelFlags(channelFlags);
}

#include "kis_adjustment_layer.moc"
