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

#ifndef _KIS_WIDGETS_PLUGIN_H_
#define _KIS_WIDGETS_PLUGIN_H_

#include <map>
#include <qwidgetplugin.h>

struct KisWidgetInfo
{
	KisWidgetInfo(QString nincludeFile, QString ntoolTip, QString nwhatsThis, bool isContainer);
	QString includeFile;
	QString toolTip;
	QString whatsThis;
	bool isContainer;
};

class KisWidgetsPlugin : public QWidgetPlugin
{
	typedef std::map<QString, KisWidgetInfo> widgetInfoMap;
	typedef widgetInfoMap::const_iterator widgetInfoMap_cit;

public:
  KisWidgetsPlugin();
  virtual ~KisWidgetsPlugin();

  virtual QStringList keys() const;
  virtual QWidget* create(const QString& key, QWidget* parent = 0, const char* name = 0);
  virtual QIconSet iconSet(const QString& key) const;
  virtual bool isContainer(const QString& key) const;
  virtual QString group(const QString& key) const;
  virtual QString includeFile(const QString& key) const;
  virtual QString tooltip(const QString& key) const;
  virtual QString whatsThis(const QString& key) const;

private:
  widgetInfoMap m_widgetsMap;
};

#endif
