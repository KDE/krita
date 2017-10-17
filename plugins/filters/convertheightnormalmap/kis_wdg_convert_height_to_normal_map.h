/*
 * Copyright (c) 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
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
#ifndef KIS_WDG_CONVERT_HEIGHT_TO_NORMAL_MAP_H
#define KIS_WDG_CONVERT_HEIGHT_TO_NORMAL_MAP_H
#include <kis_config_widget.h>
#include <QWidget>
#include <KoColorSpace.h>
#include "ui_wdg_convert_height_to_normal_map.h"

class Ui_WidgetConvertHeightToNormalMap;

class KisWdgConvertHeightToNormalMap : public KisConfigWidget
{
    Q_OBJECT
public:
    KisWdgConvertHeightToNormalMap(QWidget *parent, const KoColorSpace *cs);
    ~KisWdgConvertHeightToNormalMap();

    KisPropertiesConfigurationSP configuration() const override;
    void setConfiguration(const KisPropertiesConfigurationSP config) override;

    enum swizzle {
        xPlus,
        xMin,
        yPlus,
        yMin,
        zPlus,
        zMin
    };

private:
    Ui_WidgetConvertHeightToNormalMap *ui;
    QStringList m_types;
    QStringList m_types_translatable;
    const KoColorSpace *m_cs;
private Q_SLOTS:
    void horizontalRadiusChanged(qreal r);
    void verticalRadiusChanged(qreal r);
    void aspectLockChanged(bool v);
};

#endif // KIS_WDG_CONVERT_HEIGHT_TO_NORMAL_MAP_H
