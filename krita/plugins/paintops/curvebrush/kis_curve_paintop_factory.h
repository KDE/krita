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
#ifndef KIS_CURVE_PAINTOP_FACTORY_H_
#define KIS_CURVE_PAINTOP_FACTORY_H_

#include <kis_paintop_factory.h>
#include <kis_types.h>
#include <klocale.h>
#include <QString>

class KisPaintOp;
class KisPainter;
class QWidget;
class KoInputDevice;

class KisCurvePaintOpFactory : public KisPaintOpFactory
{

public:
    virtual ~KisCurvePaintOpFactory() {}
    KisCurvePaintOpFactory();

    virtual KisPaintOp * createOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisImageSP image);

    virtual QString id() const {
        return "curvebrush";
    }
    virtual QString name() const {
        return i18n("Curve brush");
    }
    virtual QString pixmap() {
        return "krita-curve.png";
    }

    virtual KisPaintOpSettingsSP settings(const KoInputDevice& inputDevice, KisImageSP image);
    virtual KisPaintOpSettingsSP settings(KisImageSP image);
    virtual KisPaintOpSettingsWidget* createSettingsWidget(QWidget* parent);
};

#endif

