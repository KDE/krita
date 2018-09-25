/*
 *  Copyright (c) 2018 Victor Wåhlström <victor.wahlstrom@initiali.se>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
