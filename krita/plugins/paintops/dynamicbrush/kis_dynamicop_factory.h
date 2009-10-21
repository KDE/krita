/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_DYNAMICOP_FACTORY_H_
#define KIS_DYNAMICOP_FACTORY_H_

#include <kis_paintop_settings.h>
#include "kis_paintop.h"
#include "kis_paintop_factory.h"
#include <QString>
#include <klocale.h>

class QWidget;
class QPointF;
class KisPainter;

class KisDynamicBrush;
class KisDynamicOpSettingsWidget;
class KisBookmarkedConfigurationManager;
class KisBookmarkedConfigurationsModel;
class KisDynamicOpSettings;

class KisDynamicOpFactory : public KisPaintOpFactory
{
public:

    KisDynamicOpFactory(KisBookmarkedConfigurationsModel* shapeBookmarksManager,
                        KisBookmarkedConfigurationsModel* coloringBookmarksManager);

    virtual ~KisDynamicOpFactory();

    virtual KisPaintOp * createOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisImageWSP image);

    virtual QString id() const {
        return "dynamicbrush";
    }

    virtual QString name() const {
        return i18n("Dynamic Brush");
    }

    virtual QString pixmap() {
        return "krita-dynamic.png";
    }

    virtual KisPaintOpSettingsSP settings(const KoInputDevice& inputDevice, KisImageWSP image);
    virtual KisPaintOpSettingsSP settings(KisImageWSP image);
    virtual KisPaintOpSettingsWidget* createSettingsWidget(QWidget* parent);

    KisBookmarkedConfigurationsModel* shapeBookmarksManager() {
        return m_shapeBookmarksManager;
    }

    KisBookmarkedConfigurationsModel* coloringBookmarksManager() {
        return m_coloringBookmarksManager;
    }

private:

    KisBookmarkedConfigurationsModel* m_shapeBookmarksManager;
    KisBookmarkedConfigurationsModel* m_coloringBookmarksManager;
};


#endif // KIS_DYNAMICOP_H_
