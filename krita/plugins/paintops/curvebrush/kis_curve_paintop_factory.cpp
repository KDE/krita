/*
 *  Copyright (c) 2008 Lukáš Tvrdý (lukast.dev@gmail.com)
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
#include "kis_curve_paintop_factory.h"

#include <kis_painter.h>
#include <kis_paintop_settings.h>
#include <kis_image.h>

#include <KoInputDevice.h>

#include "kis_curve_paintop_settings_widget.h"
#include "kis_curve_paintop_settings.h"
#include "kis_curve_paintop.h"

KisCurvePaintOpFactory::KisCurvePaintOpFactory()
{
}

KisPaintOp * KisCurvePaintOpFactory::createOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisImageWSP image = 0)
{
    const KisCurvePaintOpSettings *curveSettings = dynamic_cast<const KisCurvePaintOpSettings *>(settings.data());
    Q_ASSERT(settings == 0 || curveSettings != 0);
    KisPaintOp * op = new KisCurvePaintOp(curveSettings, painter, image);
    Q_CHECK_PTR(op);
    return op;
}

KisPaintOpSettingsSP KisCurvePaintOpFactory::settings(const KoInputDevice& inputDevice, KisImageWSP image)
{
    Q_UNUSED(inputDevice);
    Q_UNUSED(image);
    return new KisCurvePaintOpSettings();
}

KisPaintOpSettingsSP KisCurvePaintOpFactory::settings(KisImageWSP image)
{
    Q_UNUSED(image);
    return new KisCurvePaintOpSettings();
}

KisPaintOpSettingsWidget* KisCurvePaintOpFactory::createSettingsWidget(QWidget* parent)
{
    return new KisCurvePaintOpSettingsWidget(parent);
}
