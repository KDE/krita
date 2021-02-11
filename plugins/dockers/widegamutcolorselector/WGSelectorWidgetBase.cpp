/*
 * SPDX-FileCopyrightText: 2021 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "WGSelectorWidgetBase.h"

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

void WGSelectorWidgetBase::setModel(KisVisualColorModelSP model)
{

}

void WGSelectorWidgetBase::updateSettings()
{

}
