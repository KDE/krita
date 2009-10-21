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
{
}

KisChalkPaintOpFactory::~KisChalkPaintOpFactory()
{
}

KisPaintOp * KisChalkPaintOpFactory::createOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisImageWSP image = 0)
{
    const KisChalkPaintOpSettings *chalkSettings =
        dynamic_cast<const KisChalkPaintOpSettings *>(settings.data());
    Q_ASSERT(settings == 0 || chalkSettings != 0);

    KisPaintOp * op = new KisChalkPaintOp(chalkSettings, painter, image);
    Q_CHECK_PTR(op);
    return op;
}

KisPaintOpSettingsSP KisChalkPaintOpFactory::settings(const KoInputDevice& inputDevice, KisImageWSP image)
{
    Q_UNUSED(inputDevice);
    Q_UNUSED(image);
    return new KisChalkPaintOpSettings();
}

KisPaintOpSettingsSP KisChalkPaintOpFactory::settings(KisImageWSP image)
{
    Q_UNUSED(image);
    return new KisChalkPaintOpSettings();
}

KisPaintOpSettingsWidget* KisChalkPaintOpFactory::createSettingsWidget(QWidget* parent)
{
    return new KisChalkPaintOpSettingsWidget(parent);
}
