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
#include "kis_filter.h"
#include "kis_filter_configuration.h"

class KisFilterMask::Private {
public:

    KisFilterConfiguration * filterConfig;
};

KisFilterMask::KisFilterMask()
    : KisEffectMask()
{
    m_d = new Private();
    m_d->filterConfig = 0;
}

KisFilterMask::~KisFilterMask()
{
    delete m_d->filterConfig;
    delete m_d;
}

KisFilterMask::KisFilterMask( const KisFilterMask& rhs )
    : KisMask( rhs )
{
}

KisFilterConfiguration * KisFilterMask::filter() const
{
    return m_d->filterConfig;
}


void KisFilterMask::setFilter(KisFilterConfiguration * filterConfig)
{
    Q_ASSERT(filterConfig);
    m_d->filterConfig = filterConfig;
}

void KisFilterMask::apply( KisPaintDeviceSP projection, const QRect & rc )
{
    Q_ASSERT( filterConfig );

    KisFilterSP filter = KisFilterRegistry::instance()->value( cfg->name() );
    if (!filter) {
        kWarning() << "Could not retrieve filter with name " << cfg->name() << endl;
        return;
    }

    KisSelectionSP oldSelection = projection->setSelection( new KisSelection( 0, this ) );

    filter->process( projection, rc, cfg );

    projection->setSelection( oldSelection );

}
