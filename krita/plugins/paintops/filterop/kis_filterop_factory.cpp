/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#include "kis_filterop_factory.h"

#include <kis_painter.h>
#include <kis_paintop_settings.h>
#include <kis_image.h>

#include "kis_filterop_settings_widget.h"
#include "kis_filterop_settings.h"
#include "kis_filterop.h"

#include <KoInputDevice.h>

KisFilterOpFactory::KisFilterOpFactory()
{
}

KisFilterOpFactory::~KisFilterOpFactory()
{
}


KisPaintOp * KisFilterOpFactory::createOp(const KisPaintOpSettingsSP settings,
                                         KisPainter * painter,
                                         KisImageWSP image)
{
    KisFilterOpSettings *filteropSettings = const_cast<KisFilterOpSettings*>( dynamic_cast<const KisFilterOpSettings *>(settings.data()) );
    Q_ASSERT(settings != 0 && filteropSettings != 0);
    filteropSettings->setImage( image );

    KisPaintOp * op = new KisFilterOp(filteropSettings, painter);
    Q_CHECK_PTR(op);
    return op;
}

KisPaintOpSettingsSP KisFilterOpFactory::settings(const KoInputDevice& inputDevice, KisImageWSP image)
{
    Q_UNUSED(inputDevice);
    return settings(image);
}

KisPaintOpSettingsSP KisFilterOpFactory::settings(KisImageWSP image)
{
    KisFilterOpSettings* settings = new KisFilterOpSettings();
    settings->setImage(image);
    return settings;
}

KisPaintOpSettingsWidget* KisFilterOpFactory::createSettingsWidget(QWidget* parent)
{
    return new KisFilterOpSettingsWidget(parent);
}
