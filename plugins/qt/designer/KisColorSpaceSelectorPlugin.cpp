/*
 *  SPDX-FileCopyrightText: 2018 Victor Wåhlström <victor.wahlstrom@initiali.se>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
