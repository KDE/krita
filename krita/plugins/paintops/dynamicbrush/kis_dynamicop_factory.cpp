/*
 *  Copyright (c) 2006-2007 Cyrille Berger <cberger@cberger.net>
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

#include "kis_dynamicop_factory.h"

#include <QWidget>

#include <KoInputDevice.h>

#include <kis_painter.h>
#include <kis_image.h>
#include <kis_types.h>
#include <kis_bookmarked_configuration_manager.h>

#include "kis_dynamicop_settings.h"
#include "kis_dynamicop.h"
#include "kis_dynamicop_settings_widget.h"

KisDynamicOpFactory::KisDynamicOpFactory(KisBookmarkedConfigurationsModel* shapeBookmarksManager,
        KisBookmarkedConfigurationsModel* coloringBookmarksManager)
        : m_shapeBookmarksManager(shapeBookmarksManager)
        , m_coloringBookmarksManager(coloringBookmarksManager)
{
}

KisDynamicOpFactory::~KisDynamicOpFactory()
{
}

KisPaintOp * KisDynamicOpFactory::createOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisImageWSP image)
{
    Q_UNUSED(image);
    const KisDynamicOpSettings *dosettings = dynamic_cast<const KisDynamicOpSettings *>(settings.data());
    Q_ASSERT(dosettings);

    KisPaintOp* op = new KisDynamicOp(dosettings, painter);
    Q_CHECK_PTR(op);
    return op;
}

KisPaintOpSettingsSP KisDynamicOpFactory::settings(const KoInputDevice& inputDevice, KisImageWSP image)
{
    Q_UNUSED(image);
    Q_UNUSED(inputDevice);
    return new KisDynamicOpSettings(m_shapeBookmarksManager, m_coloringBookmarksManager);
}

KisPaintOpSettingsSP KisDynamicOpFactory::settings(KisImageWSP image)
{
    Q_UNUSED(image);
    return new KisDynamicOpSettings(m_shapeBookmarksManager, m_coloringBookmarksManager);
}

KisPaintOpSettingsWidget* KisDynamicOpSettings::createSettingsWidget(QWidget* parent)
{
    return new KisDynamicOpSettingsWidget(parent);
}
