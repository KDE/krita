/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2004 Clarence Dang <dang@kde.org>
 *  SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_DUPLICATEOP_SETTINGS_WIDGET_H_
#define KIS_DUPLICATEOP_SETTINGS_WIDGET_H_

#include <kis_brush_based_paintop_options_widget.h>
#include <kis_image.h>

class KisDuplicateOpOption;
class KisPaintopLodLimitations;

class KisDuplicateOpSettingsWidget : public KisBrushBasedPaintopOptionWidget
{

    Q_OBJECT

public:

    KisDuplicateOpSettingsWidget(QWidget* parent = 0);

    ~KisDuplicateOpSettingsWidget() override;

    KisPropertiesConfigurationSP configuration() const override;
    KisPaintopLodLimitations lodLimitations() const override;

    bool supportScratchBox() override {
        return false;
    }

public:
    KisDuplicateOpOption* m_duplicateOption;

};



#endif // KIS_DUPLICATEOP_SETTINGS_WIDGET_H_
