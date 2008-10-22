/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_paintop_settings.h"

#include <QWidget>

#include <KoPointerEvent.h>

#include "kis_node.h"
#include "kis_paintop_settings_widget.h"

struct KisPaintOpSettings::Private {
    KisNodeSP node;
    KisPaintOpSettingsWidget *settingsWidget;
};

KisPaintOpSettings::KisPaintOpSettings(KisPaintOpSettingsWidget *settingsWidget)
    : d(new Private)
{
    d->settingsWidget = settingsWidget;
}

KisPaintOpSettings::~KisPaintOpSettings()
{
    delete d;
}

KisPaintOpSettingsWidget* KisPaintOpSettings::widget() const
{
    return d->settingsWidget;
}

void KisPaintOpSettings::mousePressEvent(KoPointerEvent *e)
{
    e->ignore();
}

void KisPaintOpSettings::activate()
{
}

void KisPaintOpSettings::setNode(KisNodeSP node)
{
    d->node = node;
}

KisNodeSP KisPaintOpSettings::node() const
{
    return d->node;
}

