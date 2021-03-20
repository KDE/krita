/*
 *  SPDX-FileCopyrightText: 2008, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_DEFORM_PAINTOP_SETTINGS_WIDGET_H_
#define KIS_DEFORM_PAINTOP_SETTINGS_WIDGET_H_

#include <kis_paintop_settings_widget.h>

class KisDeformOption;
class KisBrushSizeOption;

class KisDeformPaintOpSettingsWidget : public KisPaintOpSettingsWidget
{
    Q_OBJECT
public:
    KisDeformPaintOpSettingsWidget(QWidget* parent = 0);
    ~KisDeformPaintOpSettingsWidget() override;

    KisPropertiesConfigurationSP configuration() const override;

private:
    KisDeformOption * m_deformOption;
    KisBrushSizeOption * m_brushSizeOption;
};

#endif
