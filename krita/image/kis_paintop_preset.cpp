/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "kis_paintop_preset.h"

#include <QFile>
#include <QImage>

#include <KoColorSpaceRegistry.h>

#include "kis_types.h"
#include "kis_paintop_settings.h"
#include "kis_paintop_registry.h"
#include "kis_painter.h"
#include "kis_paint_information.h"
#include "kis_paint_device.h"
#include "kis_image.h"

struct KisPaintOpPreset::Private {
    KisPaintOpSettingsSP settings;
    QImage img;
};


KisPaintOpPreset::KisPaintOpPreset()
 : KoResource(QString::null)
 , m_d(new Private)
{
}

KisPaintOpPreset::KisPaintOpPreset(const QString & fileName)
 : KoResource(fileName)
 , m_d(new Private)
{
}

KisPaintOpPreset::~KisPaintOpPreset()
{
    delete m_d;
}

KoID KisPaintOpPreset::paintOp()
{
    return KoID(m_d->settings->getString("paintop"), name());
}

KisPaintOpSettingsSP KisPaintOpPreset::settings()
{
    return m_d->settings;
}

bool KisPaintOpPreset::load()
{
    if (filename().isEmpty())
        return false;
        
    QFile f(filename());
    f.open(QIODevice::ReadOnly);
    m_d->settings->fromXML(QString(f.readAll()));
    f.close();
    return true;

    setName(m_d->settings->getString("name"));
}

bool KisPaintOpPreset::save()
{
    if (filename().isEmpty())
        return false;
        
    QFile f(filename());
    f.open( QIODevice::WriteOnly );
    f.write( m_d->settings->toXML().toUtf8() );
    f.close();
    return true;
}

QImage KisPaintOpPreset::img() const
{
    return m_d->img;
}

void KisPaintOpPreset::updateImg()
{
    Q_ASSERT(!m_d->settings->getString("paintop").isNull());
    
    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8(), "paintop_preset");
    KisPainter gc(dev);
    KisPaintOp * op = KisPaintOpRegistry::instance()->paintOp(m_d->settings->getString("paintop"),
                                                              m_d->settings, &gc);

}

