/* This file is part of the KDE project
   Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kis_widgets_plugin.h"
#include "kis_cmb_imagetype.h"
#include "kis_cmb_composite.h"
#include "integerwidget.h"
#include "kis_filter_configuration_widget.h"
#include "kis_previewwidget.h"

KisWidgetInfo::KisWidgetInfo(QString nincludeFile, QString ntoolTip, QString nwhatsThis, bool nisContainer) :
	includeFile (nincludeFile),
	toolTip (ntoolTip),
	whatsThis (nwhatsThis),
	isContainer (nisContainer)
{
};

KisWidgetsPlugin::KisWidgetsPlugin()
{
	m_widgetsMap.insert(widgetInfoMap::value_type("IntegerWidget", 
		KisWidgetInfo("integerwidget.h", "a combobox displaying the colorspaces", "", false)));
	m_widgetsMap.insert(widgetInfoMap::value_type("KisCmbImageType", 
		KisWidgetInfo("kis_cmb_imagetype.h", "a combobox displaying the colorspaces", "", false)));
	m_widgetsMap.insert(widgetInfoMap::value_type("KisCmbComposite", 
		KisWidgetInfo("kis_cmb_composite.h", "a combobox displaying the composite operations",
		"", false)));
	m_widgetsMap.insert(widgetInfoMap::value_type("KisFilterConfigurationWidget", 
		KisWidgetInfo("kis_filter_configuration_widget.h", "a widget for configuring a filter", "", true)));
	m_widgetsMap.insert(widgetInfoMap::value_type("KisPreviewWidget", 
		KisWidgetInfo("kis_previewwidget.h", "a widget which display a preview of an action", "", true)));
	new KInstance("kiswidgets");
}

KisWidgetsPlugin::~KisWidgetsPlugin()
{
}

QStringList KisWidgetsPlugin::keys() const
{
	QStringList list;
	widgetInfoMap_cit it = m_widgetsMap.begin();
	widgetInfoMap_cit endit = m_widgetsMap.end();
	while( it != endit )
	{
		list.append( it->first );
		++it;
	}
	return list;
}

QWidget* KisWidgetsPlugin::create(const QString& key, QWidget* parent, const char* name)
{
	if(key == "KisCmbImageType")
	{
		return new KisCmbImageType(parent, name);
	}
	if(key == "KisCmbComposite")
	{
		return new KisCmbComposite(parent, name);
	}
	if(key == "IntegerWidget")
	{
		return new IntegerWidget(0, 100, parent, name);
	}
	if(key == "KisFilterConfigurationWidget")
	{
		return new KisFilterConfigurationWidget(0, parent, name);
	}
	if(key == "KisPreviewWidget")
	{
		return new KisPreviewWidget(parent, name);
	}
	return 0;
}

QIconSet KisWidgetsPlugin::iconSet(const QString& ) const
{
	return QIconSet();
}
bool KisWidgetsPlugin::isContainer(const QString& key) const
{
	return m_widgetsMap.find(key)->second.isContainer;
}
QString KisWidgetsPlugin::group(const QString& ) const
{
	return "krita";
}
QString KisWidgetsPlugin::includeFile(const QString& key) const
{
	return m_widgetsMap.find(key)->second.includeFile;
}
QString KisWidgetsPlugin::tooltip(const QString& key) const
{
	return m_widgetsMap.find(key)->second.toolTip;
}
QString KisWidgetsPlugin::whatsThis(const QString& key) const
{
	return m_widgetsMap.find(key)->second.whatsThis;
}

Q_EXPORT_PLUGIN(KisWidgetsPlugin)
