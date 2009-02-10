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

#include "kis_complexop_factory.h"

#include <KoInputDevice.h>
#include <kis_painter.h>
#include <kis_paintop_settings.h>
#include <kis_image.h>

#include "kis_complexop_settings_widget.h"
#include "kis_complexop_settings.h"
#include "kis_complexop.h"

KisComplexOpFactory::KisComplexOpFactory()
    : m_widget( new KisComplexOpSettingsWidget )
{
}


KisComplexOpFactory::~KisComplexOpFactory()
{
    // XXX? Delete our widget?
}


KisPaintOp * KisComplexOpFactory::createOp(const KisPaintOpSettingsSP settings,
                                         KisPainter * painter,
                                         KisImageSP image)
{
    Q_UNUSED(image);
    Q_ASSERT( settings->widget() );

    const KisComplexOpSettings *complexopSettings = dynamic_cast<const KisComplexOpSettings *>(settings.data());
    Q_ASSERT(settings != 0 && complexopSettings != 0);

    KisPaintOp * op = new KisComplexOp(complexopSettings, painter);
    Q_CHECK_PTR(op);
    return op;
}

KisPaintOpSettingsSP KisComplexOpFactory::settings(QWidget * parent, const KoInputDevice& inputDevice, KisImageSP image)
{
    // XXX: store widgets per inputDevice?
    Q_UNUSED( parent );
    Q_UNUSED(inputDevice);
    m_widget->setImage(image);
    return new KisComplexOpSettings(m_widget);
}

KisPaintOpSettingsSP KisComplexOpFactory::settings(KisImageSP image)
{
    m_widget->setImage(image);
    return new KisComplexOpSettings(m_widget);
}
