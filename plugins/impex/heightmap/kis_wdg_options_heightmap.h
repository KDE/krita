/*
 *  Copyright (c) 2017 Victor Wåhlström <victor.wahlstrom@initiali.se>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_WDG_OPTIONS_HEIGHTMAP_H_
#define _KIS_WDG_OPTIONS_HEIGHTMAP_H_

#include <kis_config_widget.h>
#include "ui_kis_wdg_options_heightmap.h"

class KisWdgOptionsHeightmap : public KisConfigWidget, public Ui::WdgOptionsHeightMap
{
    Q_OBJECT

public:
    explicit KisWdgOptionsHeightmap(QWidget *parent);
    explicit KisWdgOptionsHeightmap(QWidget *parent, bool export_mode);

Q_SIGNALS:
    void statusUpdated(bool status);

protected:
    void showEvent(QShowEvent *event) override;
    void setConfiguration(const KisPropertiesConfigurationSP  cfg) override;
    KisPropertiesConfigurationSP configuration() const override;

private Q_SLOTS:
    void guessDimensions();
    void widthChanged(int i);
    void heightChanged(int i);

private:
    void updateStatus();
    bool m_exportMode;
};

#endif // _KIS_WDG_OPTIONS_HEIGHTMAP_H_
