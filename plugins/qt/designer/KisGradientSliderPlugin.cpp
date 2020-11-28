/*
 *  SPDX-FileCopyrightText: 2018 Victor Wåhlström <victor.wahlstrom@initiali.se>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisGradientSliderPlugin.h"
#include <KisGradientSlider.h>


KisGradientSliderPlugin::KisGradientSliderPlugin(QObject *parent)
    : QObject(parent),
      m_initialized(false)
{
}

void KisGradientSliderPlugin::initialize(QDesignerFormEditorInterface*)
{
    if (m_initialized)
        return;

    m_initialized = true;
}

bool KisGradientSliderPlugin::isInitialized() const
{
    return m_initialized;
}

QWidget* KisGradientSliderPlugin::createWidget(QWidget *parent)
{
    return new KisGradientSlider(parent);
}

QString KisGradientSliderPlugin::name() const
{
    return "KisGradientSlider";
}

QString KisGradientSliderPlugin::group() const
{
    return "Krita";
}

QIcon KisGradientSliderPlugin::icon() const
{
    return QIcon();
}

QString KisGradientSliderPlugin::toolTip() const
{
    return tr("A gradient slider.");
}

QString KisGradientSliderPlugin::whatsThis() const
{
    return tr("A gradient slider.");
}

bool KisGradientSliderPlugin::isContainer() const
{
    return false;
}

QString KisGradientSliderPlugin::domXml() const
{
    return "<ui language=\"c++\">\n"
           " <widget class=\"KisGradientSlider\" name=\"gradientSlider\">\n"
           "  <property name=\"geometry\">\n"
           "   <rect>\n"
           "    <x>0</x>\n"
           "    <y>0</y>\n"
           "    <width>100</width>\n"
           "    <height>25</height>\n"
           "   </rect>\n"
           "  </property>\n"
           "  <property name=\"toolTip\" >\n"
           "   <string></string>\n"
           "  </property>\n"
           "  <property name=\"whatsThis\" >\n"
           "   <string></string>\n"
           "  </property>\n"
           " </widget>\n"
           "</ui>\n";
}

QString KisGradientSliderPlugin::includeFile() const
{
    return "<KisGradientSlider.h>";
}
