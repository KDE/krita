/*
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

#ifndef KIS_DYNA_PAINTOP_SETTINGS_H_
#define KIS_DYNA_PAINTOP_SETTINGS_H_

#include <QList>
#include <kis_paintop_settings.h>
#include <kis_types.h>

#include "kis_dyna_paintop_settings_widget.h"

class QWidget;
class QDomElement;
class QDomDocument;


class KisDynaPaintOpSettings : public KisPaintOpSettings
{

public:


    KisDynaPaintOpSettings();
    virtual ~KisDynaPaintOpSettings() {}

    bool paintIncremental();

    using KisPropertiesConfiguration::fromXML;
    using KisPropertiesConfiguration::toXML;

    virtual void fromXML(const QDomElement&);
    virtual void toXML(QDomDocument&, QDomElement&) const;

    KisPaintOpSettingsSP clone() const;


    qreal initWidth() const;
    qreal mass() const;
    qreal drag() const;
    bool useFixedAngle() const;
    qreal xAngle() const;
    qreal yAngle() const;
    qreal widthRange() const;

    int action() const;
    int circleRadius() const;
    bool enableLine() const;
    bool twoCircles() const;

    int lineCount() const;
    qreal lineSpacing() const;

    // XXX: Hack!
    void setOptionsWidget(KisPaintOpSettingsWidget* widget) {
        if (m_options != 0 && m_options->property("owned by settings").toBool()) {
            delete m_options;
        }
        if (!widget) {
            m_options = 0;
        } else {
            m_options = qobject_cast<KisDynaPaintOpSettingsWidget*>(widget);
            m_options->writeConfiguration(this);
        }
    }

private:

    KisDynaPaintOpSettingsWidget* m_options;
};

#endif
