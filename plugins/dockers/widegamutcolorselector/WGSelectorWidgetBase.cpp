/*
 * SPDX-FileCopyrightText: 2021 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "WGSelectorWidgetBase.h"

#include <kis_display_color_converter.h>

const KisDisplayColorConverter *WGSelectorDisplayConfig::displayConverter() const
{
     return m_displayConverter ? m_displayConverter
                               : KisDisplayColorConverter::dumbConverterInstance();
}

void WGSelectorDisplayConfig::setDisplayConverter(const KisDisplayColorConverter *converter)
{
    if (converter != m_displayConverter) {
        if (m_displayConverter) {
            m_displayConverter->disconnect(this);
        }
        if (converter) {
            connect(converter, &KisDisplayColorConverter::displayConfigurationChanged,
                    this, &WGSelectorDisplayConfig::sigDisplayConfigurationChanged, Qt::UniqueConnection);
        }
        m_displayConverter = converter;

        Q_EMIT sigDisplayConfigurationChanged();
    }
}

void WGSelectorDisplayConfig::setPreviewInPaintingCS(bool enabled)
{
    if (enabled != m_previewInPaintingCS) {
        m_previewInPaintingCS = enabled;
        Q_EMIT sigDisplayConfigurationChanged();
    }
}

WGSelectorWidgetBase::WGSelectorWidgetBase(WGSelectorDisplayConfigSP displayConfig, QWidget *parent, WGSelectorWidgetBase::UiMode uiMode)
    : QWidget(parent)
    , m_displayConfig(displayConfig)
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

WGSelectorDisplayConfigSP WGSelectorWidgetBase::displayConfiguration() const
{
    return m_displayConfig;
}

const KisDisplayColorConverter *WGSelectorWidgetBase::displayConverter() const
{
    return m_displayConfig ? m_displayConfig->displayConverter() : KisDisplayColorConverter::dumbConverterInstance();
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
