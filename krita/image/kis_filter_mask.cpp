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

void KisFilterMask::setDirty()
{
    if(selection())
    {
        KisEffectMask::setDirty(selection()->selectedExactRect());
    } else {
        KisEffectMask::setDirty();
    }
}

void KisFilterMask::apply(KisPaintDeviceSP projection, const QRect & rc) const
{
    dbgImage << "Applying filter mask on projection  " << projection << " with rect " << rc
    << " and filter config " << m_d->filterConfig;

    if (!m_d->filterConfig) return;

    selection()->updateProjection(rc);

    KisTransaction transac("", projection, 0 );
    KisConstProcessingInformation src(projection,  rc.topLeft(), selection());
    KisProcessingInformation dst(projection, rc.topLeft(), selection());

    KisFilterSP filter = KisFilterRegistry::instance()->value(m_d->filterConfig->name());
    if (!filter) {
        warnKrita << "Could not retrieve filter with name " <<  m_d->filterConfig->name();
        return;
    }

    Q_ASSERT( nodeProgressProxy() );

    KoProgressUpdater updater( nodeProgressProxy() );
    updater.start( 100, filter->name() );
    QPointer<KoUpdater> up = updater.startSubtask();

    filter->process(src, dst, rc.size(), m_d->filterConfig,  up);
    nodeProgressProxy()->setValue( nodeProgressProxy()->maximum() );

}

bool KisFilterMask::accept(KisNodeVisitor &v)
{
    return v.visit(this);
}

QRect KisFilterMask::adjustedDirtyRect( const QRect& _rect ) const
{
    if( !m_d->filterConfig) return _rect;
    KisFilterSP filter = KisFilterRegistry::instance()->value(m_d->filterConfig->name());
    return filter->changedRect( _rect, m_d->filterConfig );
}

QRect KisFilterMask::neededRect( const QRect& _rect ) const
{
    if( !m_d->filterConfig) return _rect;
    KisFilterSP filter = KisFilterRegistry::instance()->value(m_d->filterConfig->name());
    return filter->neededRect( _rect, m_d->filterConfig );
}

#include "kis_filter_mask.moc"
