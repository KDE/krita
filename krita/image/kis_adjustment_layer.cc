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

#include "kis_image.h"
#include "kis_selection.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include "filter/kis_filter.h"
#include "kis_node_visitor.h"
#include "kis_processing_visitor.h"


struct KisAdjustmentLayer::Private
{
public:
    KisFilterConfiguration *filterConfig;
};

KisAdjustmentLayer::KisAdjustmentLayer(KisImageWSP image,
                                       const QString &name,
                                       KisFilterConfiguration * kfc,
                                       KisSelectionSP selection)
        : KisSelectionBasedLayer(image.data(), name, selection),
        m_d(new Private())
{
    if(kfc)
        m_d->filterConfig = KisFilterRegistry::instance()->cloneConfiguration(kfc);
}

KisAdjustmentLayer::KisAdjustmentLayer(const KisAdjustmentLayer& rhs)
        : KisSelectionBasedLayer(rhs),
        m_d(new Private())
{
    m_d->filterConfig = KisFilterRegistry::instance()->cloneConfiguration(rhs.m_d->filterConfig);
}


KisAdjustmentLayer::~KisAdjustmentLayer()
{
    delete m_d->filterConfig;
    delete m_d;
}

KisFilterConfiguration * KisAdjustmentLayer::filter() const
{
    return m_d->filterConfig;
}


void KisAdjustmentLayer::setFilter(KisFilterConfiguration * filterConfig)
{
    delete m_d->filterConfig;
    m_d->filterConfig = KisFilterRegistry::instance()->cloneConfiguration(filterConfig);
    m_d->filterConfig->setChannelFlags(channelFlags());
}

QRect KisAdjustmentLayer::changeRect(const QRect &rect, PositionToFilthy pos) const
{
    QRect filteredRect = rect;

    if (m_d->filterConfig) {
        KisFilterSP filter = KisFilterRegistry::instance()->value(m_d->filterConfig->name());
        filteredRect = filter->changedRect(rect, m_d->filterConfig);
    }

    /**
     * We can't paint outside a selection, that is why we call
     * KisSelectionBasedLayer::changeRect to crop actual change
     * area in the end
     */
    filteredRect = KisSelectionBasedLayer::changeRect(filteredRect, pos);

    return rect | filteredRect;
}

QRect KisAdjustmentLayer::needRect(const QRect& rect, PositionToFilthy pos) const
{
    Q_UNUSED(pos);
    if (!m_d->filterConfig) return rect;
    KisFilterSP filter = KisFilterRegistry::instance()->value(m_d->filterConfig->name());

    /**
     * If we need some additional pixels even outside of a selection
     * for accurate layer filtering, we'll get them!
     * And no KisSelectionBasedLayer::needRect will prevent us
     * from doing this! ;)
     * That's why simply we do not call
     * KisSelectionBasedLayer::needRect here :)
     */
    return filter->neededRect(rect, m_d->filterConfig);
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

KoDocumentSectionModel::PropertyList KisAdjustmentLayer::sectionModelProperties() const
{
    KoDocumentSectionModel::PropertyList l = KisLayer::sectionModelProperties();
    if (filter())
        l << KoDocumentSectionModel::Property(i18n("Filter"), KisFilterRegistry::instance()->value(filter()->name())->name());
    return l;
}

void KisAdjustmentLayer::setChannelFlags(const QBitArray & channelFlags)
{
    if (m_d->filterConfig) {
        m_d->filterConfig->setChannelFlags(channelFlags);
    }
    KisLayer::setChannelFlags(channelFlags);
}

#include "kis_adjustment_layer.moc"
