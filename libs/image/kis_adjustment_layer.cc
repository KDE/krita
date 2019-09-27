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

#include <klocalizedstring.h>
#include "kis_debug.h"

#include <KoIcon.h>
#include <kis_icon.h>
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
                                       KisFilterConfigurationSP kfc,
                                       KisSelectionSP selection)
    : KisSelectionBasedLayer(image.data(), name, selection, kfc)
{
    // by default Adjustment Layers have a copy composition,
    // which is more natural for users
    // https://bugs.kde.org/show_bug.cgi?id=324505
    // https://bugs.kde.org/show_bug.cgi?id=294122
    // demand the opposite from each other...
    //
    // also see a comment in KisLayerUtils::mergeMultipleLayersImpl()

    setCompositeOpId(COMPOSITE_COPY);
    setUseSelectionInProjection(false);
}

KisAdjustmentLayer::KisAdjustmentLayer(const KisAdjustmentLayer& rhs)
        : KisSelectionBasedLayer(rhs)
{
}


KisAdjustmentLayer::~KisAdjustmentLayer()
{
}

void KisAdjustmentLayer::setFilter(KisFilterConfigurationSP filterConfig)
{
    filterConfig->setChannelFlags(channelFlags());
    KisSelectionBasedLayer::setFilter(filterConfig);
}

QRect KisAdjustmentLayer::incomingChangeRect(const QRect &rect) const
{
    KisFilterConfigurationSP filterConfig = filter();

    QRect filteredRect = rect;

    if (filterConfig) {
        KisFilterSP filter = KisFilterRegistry::instance()->value(filterConfig->name());
        filteredRect = filter->changedRect(rect, filterConfig.data(), projection()->defaultBounds()->currentLevelOfDetail());
    }

    /**
     * After the change in the blending using
     * setUseSelectionInProjection(false) we should *not* crop the
     * change rect of the layer, because we pass contents through.
     *
     * //filteredRect = cropChangeRectBySelection(filteredRect);
     */

    return filteredRect;
}

QRect KisAdjustmentLayer::needRect(const QRect& rect, PositionToFilthy pos) const
{
    Q_UNUSED(pos);

    KisFilterConfigurationSP filterConfig = filter();
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
    QRect needRect;
    needRect |= needRectForOriginal(needRect);
    needRect = filter->neededRect(rect, filterConfig.data(), projection()->defaultBounds()->currentLevelOfDetail());
    return needRect;
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
    return KisIconUtils::loadIcon("filterLayer");
}

KisBaseNode::PropertyList KisAdjustmentLayer::sectionModelProperties() const
{
    KisFilterConfigurationSP filterConfig = filter();
    KisBaseNode::PropertyList l = KisLayer::sectionModelProperties();
    if (filterConfig)
        l << KisBaseNode::Property(KoID("filter", i18nc("property of a filter layer, noun", "Filter")), KisFilterRegistry::instance()->value(filterConfig->name())->name());

    return l;
}

void KisAdjustmentLayer::setChannelFlags(const QBitArray & channelFlags)
{
    KisFilterConfigurationSP filterConfig = filter();

    if (filterConfig) {
        filterConfig->setChannelFlags(channelFlags);
    }
    KisLayer::setChannelFlags(channelFlags);
}

