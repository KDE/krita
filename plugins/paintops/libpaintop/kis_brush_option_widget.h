/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
    void setHSLBrushTipEnabled(bool value);

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
