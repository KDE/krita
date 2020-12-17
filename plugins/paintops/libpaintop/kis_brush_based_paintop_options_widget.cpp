/*
 *  SPDX-FileCopyrightText: 2010 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_brush_based_paintop_options_widget.h"

#include "kis_brush_option_widget.h"
#include <klocalizedstring.h>

KisBrushBasedPaintopOptionWidget::KisBrushBasedPaintopOptionWidget(QWidget* parent)
    : KisPaintOpSettingsWidget(parent)
{
    m_brushOption = new KisBrushOptionWidget();
    addPaintOpOption(m_brushOption, i18n("Brush Tip"));
}

KisBrushBasedPaintopOptionWidget::~KisBrushBasedPaintopOptionWidget()
{
}

void KisBrushBasedPaintopOptionWidget::setPrecisionEnabled(bool value)
{
    m_brushOption->setPrecisionEnabled(value);
}

void KisBrushBasedPaintopOptionWidget::setHSLBrushTipEnabled(bool value)
{
    m_brushOption->setHSLBrushTipEnabled(value);
}

KisBrushSP KisBrushBasedPaintopOptionWidget::brush()
{
    return m_brushOption->brush();
}

bool KisBrushBasedPaintopOptionWidget::presetIsValid()
{
    return m_brushOption->presetIsValid();
}

KisBrushOptionWidget *KisBrushBasedPaintopOptionWidget::brushOptionWidget() const
{
    return m_brushOption;
}
