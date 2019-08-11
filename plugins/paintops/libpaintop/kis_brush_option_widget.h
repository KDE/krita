/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KIS_BRUSH_OPTION_H
#define KIS_BRUSH_OPTION_H

#include "kis_paintop_option.h"
#include "kis_brush_option.h"
#include <kritapaintop_export.h>
#include "kis_brush.h"

class KisBrushSelectionWidget;

/**
 * The brush option allows the user to select a particular brush
 * footprint for suitable paintops
 */
class PAINTOP_EXPORT KisBrushOptionWidget : public KisPaintOpOption
{
    Q_OBJECT
public:

    KisBrushOptionWidget();

    /**
     * @return the currently selected brush
     */
    KisBrushSP brush() const;

    void setAutoBrush(bool on);
    void setPredefinedBrushes(bool on);
    void setCustomBrush(bool on);
    void setTextBrush(bool on);

    void setImage(KisImageWSP image) override;

    void setPrecisionEnabled(bool value);

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

    void lodLimitations(KisPaintopLodLimitations *l) const override;

    bool presetIsValid();

    void hideOptions(const QStringList &options);

private Q_SLOTS:
    void brushChanged();

private:

    KisBrushSelectionWidget * m_brushSelectionWidget;
    KisBrushOptionProperties m_brushOption;

};

#endif
