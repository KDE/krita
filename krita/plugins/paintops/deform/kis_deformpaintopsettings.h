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

#ifndef KIS_DEFORMPAINTOP_SETTINGS_H_
#define KIS_DEFORMPAINTOP_SETTINGS_H_

#include <kis_config_widget.h>

#include <kis_paintop.h>

#include "ui_wdgdeformoptions.h"
#include "widgets/kis_popup_button.h"

class KisDeformPaintOpSettings : public QObject, public KisPaintOpSettings
{
    Q_OBJECT

public:
    KisDeformPaintOpSettings(QWidget * paren);
    virtual ~KisDeformPaintOpSettings() {}

    virtual KisPaintOpSettingsSP clone() const;

    KisConfigWidget * widget() const {
        return m_optionsWidget;
    }

    using KisPropertiesConfiguration::fromXML;
    using KisPropertiesConfiguration::toXML;

    virtual void fromXML(const QDomElement&);
    virtual void toXML(QDomDocument&, QDomElement&) const;

    int radius() const;
    double deformAmount() const;
    int deformAction() const;
    bool bilinear() const;

private:
    Ui::WdgDeformOptions* m_options;
    KisConfigWidget * m_optionsWidget;
};
#endif
