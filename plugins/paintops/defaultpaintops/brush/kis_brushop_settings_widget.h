/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2004 Clarence Dang <dang@kde.org>
 *  SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_BRUSHOP_SETTINGS_WIDGET_H_
#define KIS_BRUSHOP_SETTINGS_WIDGET_H_

#include <kis_brush_based_paintop_options_widget.h>


class KisBrushOpSettingsWidget : public KisBrushBasedPaintopOptionWidget
{

    Q_OBJECT

public:

    KisBrushOpSettingsWidget(QWidget* parent, KisResourcesInterfaceSP resourcesInterface, KoCanvasResourcesInterfaceSP canvasResourcesInterface);

    ~KisBrushOpSettingsWidget() override;

    KisPropertiesConfigurationSP configuration() const override;

};


#endif // KIS_BRUSHOP_SETTINGS_WIDGET_H_
