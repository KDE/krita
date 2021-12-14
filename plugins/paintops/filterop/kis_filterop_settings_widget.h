/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2004 Clarence Dang <dang@kde.org>
 *  SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_FILTEROP_SETTINGS_WIDGET_H_
#define KIS_FILTEROP_SETTINGS_WIDGET_H_

#include <kis_brush_based_paintop_options_widget.h>
#include <kis_image.h>

class KisFilterOption;

class KisFilterOpSettingsWidget : public KisBrushBasedPaintopOptionWidget
{

    Q_OBJECT

public:

    KisFilterOpSettingsWidget(QWidget* parent = 0);

    ~KisFilterOpSettingsWidget() override;

    KisPropertiesConfigurationSP configuration() const override;
};

#endif // KIS_FILTEROP_SETTINGS_WIDGET_H_
