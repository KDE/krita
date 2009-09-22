/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2008 Lukas Tvrdy <lukast.dev@gmail.com>
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

#ifndef KIS_SPRAY_PAINTOP_FACTORY_H_
#define KIS_SPRAY_PAINTOP_FACTORY_H_

#include <klocale.h>
#include <kis_paintop_factory.h>
#include <kis_types.h>

#include "kis_spray_paintop_settings.h"

class KisPainter;
class KisSprayPaintOpSettings;
class KisSprayPaintOpSettingsWidget;

class KisSprayPaintOpFactory : public KisPaintOpFactory
{

public:

    KisSprayPaintOpFactory();
    virtual ~KisSprayPaintOpFactory();

    virtual KisPaintOp * createOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisImageSP image);

    virtual QString id() const {
        return "spraybrush";
    }

    virtual QString name() const {
        return i18n("Spray brush");
    }

    virtual QString pixmap() {
        return "krita-spray.png";
    }

    virtual KisPaintOpSettingsSP settings(const KoInputDevice& inputDevice, KisImageSP image);
    virtual KisPaintOpSettingsSP settings(KisImageSP image);
    virtual KisPaintOpSettingsWidget* settingsWidget(QWidget* parent);

private:

    KisSprayPaintOpSettingsWidget* const m_widget;
};
#endif // KIS_SPRAY_PAINTOP_FACTORY_H_
