/*
 *  SPDX-FileCopyrightText: 2010 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_brush_based_paintop_options_widget.h"

#include "kis_brush_option_widget.h"
#include <klocalizedstring.h>

KisBrushBasedPaintopOptionWidget::KisBrushBasedPaintopOptionWidget(KisBrushOptionWidgetFlags flags, QWidget* parent)
    : KisPaintOpSettingsWidget(parent)
{
    m_brushOption = new KisBrushOptionWidget(flags);
    addPaintOpOption(m_brushOption);
}

KisBrushBasedPaintopOptionWidget::~KisBrushBasedPaintopOptionWidget()
{
}

KisBrushSP KisBrushBasedPaintopOptionWidget::brush()
{
    return m_brushOption->brush();
}

lager::reader<qreal> KisBrushBasedPaintopOptionWidget::effectiveBrushSize() const
{
    return m_brushOption->effectiveBrushSize();
}

KisBrushOptionWidget *KisBrushBasedPaintopOptionWidget::brushOptionWidget() const
{
    return m_brushOption;
}
