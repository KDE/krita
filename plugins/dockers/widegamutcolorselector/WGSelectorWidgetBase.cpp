/*
 * SPDX-FileCopyrightText: 2021 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "WGSelectorWidgetBase.h"

#include <kis_display_color_converter.h>

WGSelectorWidgetBase::WGSelectorWidgetBase(QWidget *parent, WGSelectorWidgetBase::UiMode uiMode)
    : QWidget(parent)
    , m_uiMode(uiMode)
{

}

WGSelectorWidgetBase::UiMode WGSelectorWidgetBase::uiMode() const
{
    return m_uiMode;
}

void WGSelectorWidgetBase::setUiMode(WGSelectorWidgetBase::UiMode mode)
{
    m_uiMode = mode;
}

void WGSelectorWidgetBase::setDisplayConverter(const KisDisplayColorConverter *converter)
{
    m_converter = converter;
}

const KisDisplayColorConverter *WGSelectorWidgetBase::displayConverter() const
{
    return m_converter ? m_converter : KisDisplayColorConverter::dumbConverterInstance();
}

QPoint WGSelectorWidgetBase::popupOffset() const
{
    return QPoint(width()/2, height()/2);
}

void WGSelectorWidgetBase::setModel(KisVisualColorModelSP model)
{
    Q_UNUSED(model);
}

void WGSelectorWidgetBase::updateSettings()
{

}
