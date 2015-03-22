/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_paintop_factory.h"

#include <klocale.h>
#include <KoColorSpace.h>

KisPaintOpFactory::KisPaintOpFactory(const QStringList & whiteListedCompositeOps)
    : m_whiteListedCompositeOps(whiteListedCompositeOps), m_priority(100)
    , m_visibility(AUTO)
{
}

QStringList KisPaintOpFactory::whiteListedCompositeOps() const
{
    return m_whiteListedCompositeOps;
}

#ifdef HAVE_THREADED_TEXT_RENDERING_WORKAROUND
void KisPaintOpFactory::preinitializePaintOpIfNeeded(const KisPaintOpSettingsSP settings)
{
    Q_UNUSED(settings);
}
#endif /* HAVE_THREADED_TEXT_RENDERING_WORKAROUND */

QString KisPaintOpFactory::pixmap()
{
    return "";
}

QString KisPaintOpFactory::categoryStable()
{
    return i18nc("Category of brush engines", "Brush engines");
}

void KisPaintOpFactory::setPriority(int newPriority)
{
    m_priority = newPriority;
}


int KisPaintOpFactory::priority() const
{
    return m_priority;
}


#include "kis_paintop_factory.moc"

