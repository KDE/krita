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

class KisMask::KisMaskPrivate
{

public:
    KisPaintDeviceSP parent;
    EnumKisMaskRenderMoment renderWhen;
    bool active;
};


KisMask::KisMask(KisPaintDeviceSP dev,  const QString & name)
    : KisPaintDevice(dev->parentLayer(),
            KoColorSpaceRegistry::instance()->alpha8(),
            name)
    , m_d( new KisMaskPrivate() )
{
    Q_ASSERT(dev);
    m_d->parent = dev;
    m_d->active = true;
    m_d->renderWhen = RenderOnTop;
}

KisMask::KisMask(const QString & name)
    : KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8(), name)
{
    m_d = new KisMaskPrivate();
    m_d->parent = 0;
    m_d->active = false;
    m_d->renderWhen = RenderOnTop;

}

KisMask::KisMask(const KisMask& rhs)
    : KisPaintDevice(rhs)
{
    // XXX: as soon as there are pointers in m_d, fix this.
    m_d = rhs.m_d;
}

KisMask::~KisMask()
{
}

KisPaintDeviceSP KisMask::parent()
{
    return m_d->parent;
}

bool KisMask::active()
{
    return m_d->active;
}

EnumKisMaskRenderMoment KisMask::renderWhen()
{
    return m_d->renderWhen;
}
