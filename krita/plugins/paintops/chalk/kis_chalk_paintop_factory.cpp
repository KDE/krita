/*
 *  Copyright (c) 2008 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_chalk_paintop_factory.h"
#include "kis_chalk_paintop.h"
#include "kis_chalk_paintop_settings.h"
#include "kis_chalk_paintop_settings_widget.h"

#include <KoInputDevice.h>

#include <kis_image.h>
#include <kis_painter.h>
#include <kis_types.h>


KisChalkPaintOpFactory::KisChalkPaintOpFactory()
    :  m_widget( new KisChalkPaintOpSettingsWidget )
{

}

KisChalkPaintOpFactory::~KisChalkPaintOpFactory()
{
}

KisPaintOp * KisChalkPaintOpFactory::createOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisImageSP image = 0)
{
    Q_ASSERT( settings->widget() );
    const KisChalkPaintOpSettings *chalkSettings =
        dynamic_cast<const KisChalkPaintOpSettings *>(settings.data());
    Q_ASSERT(settings == 0 || chalkSettings != 0);

    KisPaintOp * op = new KisChalkPaintOp(chalkSettings, painter, image);
    Q_CHECK_PTR(op);
    return op;
}

KisPaintOpSettingsSP KisChalkPaintOpFactory::settings(QWidget * parent, const KoInputDevice& inputDevice, KisImageSP image)
{
    Q_UNUSED( parent );
    Q_UNUSED(inputDevice);
    Q_UNUSED(image);
    return new KisChalkPaintOpSettings(m_widget);
}

KisPaintOpSettingsSP KisChalkPaintOpFactory::settings(KisImageSP image)
{
    Q_UNUSED(image);
    return new KisChalkPaintOpSettings(m_widget);
}
