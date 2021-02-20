/*
 *  SPDX-FileCopyrightText: 2015 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TANGENTNORMAL_PAINTOP_SETTINGS_WIDGET_H_
#define KIS_TANGENTNORMAL_PAINTOP_SETTINGS_WIDGET_H_

#include <kis_brush_based_paintop_options_widget.h>


class KisTangentNormalPaintOpSettingsWidget : public KisBrushBasedPaintopOptionWidget
{
    Q_OBJECT

public:
    KisTangentNormalPaintOpSettingsWidget(QWidget* parent = 0);
    ~KisTangentNormalPaintOpSettingsWidget() override;

    KisPropertiesConfigurationSP configuration() const override;
};



#endif // KIS_TANGENTNORMAL_PAINTOP_SETTINGS_WIDGET_H_
