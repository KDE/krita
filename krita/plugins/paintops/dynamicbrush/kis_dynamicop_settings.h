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

#ifndef KIS_DYNAMICOP_SETTINGS_H_
#define KIS_DYNAMICOP_SETTINGS_H_

#include <QObject>
#include <kis_paintop_settings.h>
#include <kis_types.h>

#include "kis_dynamicop_settings_widget.h"

class QWidget;
class QDomElement;

class KisBookmarkedConfigurationManager;
class KisBookmarkedConfigurationsModel;
class KisPainter;
class KisDynamicBrush;
class KisDynamicOpSettingsWidget;

class KisDynamicOpSettings : public QObject, public KisPaintOpSettings
{

    Q_OBJECT

public:

    using KisPaintOpSettings::fromXML;
    using KisPaintOpSettings::toXML;

    KisDynamicOpSettings(KisDynamicOpSettingsWidget* widget,
                         KisBookmarkedConfigurationsModel* shapeConfigurationManager,
                         KisBookmarkedConfigurationsModel* coloringConfigurationManager);
    virtual ~KisDynamicOpSettings();

    bool paintIncremental();

    virtual void fromXML(const QDomElement&);
    virtual void toXML(QDomDocument&, QDomElement&) const;

    KisPaintOpSettingsSP clone() const;

    /// @return a brush with the current shapes, coloring and program
    KisDynamicBrush* createBrush(KisPainter *painter) const;

    // XXX: Hack!
    void setOptionsWidget(KisPaintOpSettingsWidget* widget) {
        if (m_options != 0 && m_options->property("owned by settings").toBool()) {
            delete m_options;
        }
        if (!widget) {
            m_options = 0;
        } else {
            m_options = qobject_cast<KisDynamicOpSettingsWidget*>(widget);
            m_options->writeConfiguration(this);
        }
    }

private:

    KisDynamicOpSettingsWidget *m_options;
    KisBookmarkedConfigurationsModel* m_shapeBookmarksModel;
    KisBookmarkedConfigurationsModel* m_coloringBookmarksModel;
};

#endif
