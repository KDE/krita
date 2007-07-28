/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_mask.h"

#include <kdebug.h>
#include <klocale.h>

#include "KoColorSpaceRegistry.h"

#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_layer.h"

class KisMask::KisMaskPrivate
{

public:
    KisPaintDeviceSP parent;
    KisLayerSP parentLayer;
    bool active;
};


KisMask::KisMask(KisPaintDeviceSP dev,  const QString & name)
    : KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8(),
                     name)
    , m_d( new KisMaskPrivate() )
{
    Q_ASSERT(dev);
    m_d->parent = dev;
    m_d->active = true;
}

KisMask::KisMask(const QString & name)
    : KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8(), name)
    , m_d( new KisMaskPrivate() )
{
    m_d->parent = 0;
    m_d->active = false;

}

KisMask::KisMask(const KisMask& rhs)
    : KisPaintDevice(rhs)
    , m_d( new KisMaskPrivate() )
{
    m_d->parent = rhs.m_d->parent;
    m_d->parentLayer = rhs.m_d->parentLayer;
    m_d->active = rhs.m_d->active;
}

KisMask::~KisMask()
{
}

KisPaintDeviceSP KisMask::parentPaintDevice() const
{
    return m_d->parent;
}

void KisMask::setName( const QString & name )
{
    setObjectName( name );
}

void KisMask::setParentLayer( KisLayerSP parentLayer )
{
    m_d->parentLayer = parentLayer;
}

KisLayerSP KisMask::parentLayer() const
{
    return m_d->parentLayer;
}

bool KisMask::active()
{
    return m_d->active;
}

void KisMask::setActive( bool active )
{
    m_d->active = active;
}
