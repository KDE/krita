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

#include "KisColorSpaceSelectorPlugin.h"
#include <widgets/kis_color_space_selector.h>


KisColorSpaceSelectorPlugin::KisColorSpaceSelectorPlugin(QObject *parent)
    : QObject(parent),
      m_initialized(false)
{
}

void KisColorSpaceSelectorPlugin::initialize(QDesignerFormEditorInterface*)
{
    if (m_initialized)
        return;

    m_initialized = true;
}

bool KisColorSpaceSelectorPlugin::isInitialized() const
{
    return m_initialized;
}

QWidget* KisColorSpaceSelectorPlugin::createWidget(QWidget *parent)
{
    return new KisColorSpaceSelector(parent);
}

QString KisColorSpaceSelectorPlugin::name() const
{
    return "KisColorSpaceSelector";
}

QString KisColorSpaceSelectorPlugin::group() const
{
    return "Krita";
}

QIcon KisColorSpaceSelectorPlugin::icon() const
{
    return QIcon();
}

QString KisColorSpaceSelectorPlugin::toolTip() const
{
    return tr("Krita widget for selecting color spaces.");
}

QString KisColorSpaceSelectorPlugin::whatsThis() const
{
    return tr("Krita widget for selecting color spaces.");
}

bool KisColorSpaceSelectorPlugin::isContainer() const
{
    return false;
}

QString KisColorSpaceSelectorPlugin::domXml() const
{
    return "<ui language=\"c++\">\n"
           " <widget class=\"KisColorSpaceSelector\" name=\"colorSpaceSelector\">\n"
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

QString KisColorSpaceSelectorPlugin::includeFile() const
{
    return "<widgets/kis_color_space_selector.h>";
}
