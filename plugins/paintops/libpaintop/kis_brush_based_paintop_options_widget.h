/*
 *  SPDX-FileCopyrightText: 2010 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_BRUSH_BASED_PAINTOP_OPTIONS_WIDGET_H
#define KIS_BRUSH_BASED_PAINTOP_OPTIONS_WIDGET_H

#include "kis_paintop_settings_widget.h"
#include "kis_types.h"
#include "kis_brush.h"
#include <kritapaintop_export.h>

class KisBrushOptionWidget;

class PAINTOP_EXPORT KisBrushBasedPaintopOptionWidget : public KisPaintOpSettingsWidget
{
public:
    KisBrushBasedPaintopOptionWidget(QWidget* parent = 0);
    ~KisBrushBasedPaintopOptionWidget() override;

    void setPrecisionEnabled(bool value);
    void setHSLBrushTipEnabled(bool value);

    KisBrushSP brush();

    bool presetIsValid() override;

protected:
    KisBrushOptionWidget *brushOptionWidget() const;

private:
    KisBrushOptionWidget *m_brushOption;
};

#endif // KIS_BRUSH_BASED_PAINTOP_OPTIONS_WIDGET_H
