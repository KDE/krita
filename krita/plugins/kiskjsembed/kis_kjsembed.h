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

#ifndef _KIS_KJSEMBEDED_H_
#define _KIS_KJSEMBEDED_H_

#include <kparts/plugin.h>
#include <kjsembed/jsproxy_imp.h>

#include <vector>

namespace KJSEmbed {
	class KJSEmbedPart;
	class JSConsoleWidget;
};

class KisView;
class KFileDialog;
class KisKJSEmbedScript;
class QPopupMenu;

class KisKJSEmbed : public KParts::Plugin
{
	Q_OBJECT
public:
	KisKJSEmbed(QObject *parent, const char *name, const QStringList &);
	virtual ~KisKJSEmbed();
private:
	void initBindings();
private slots:
	void slotLoadScript();
private:
	KisView* m_view;
	KFileDialog* m_fileDialog;
	KJSEmbed::KJSEmbedPart* m_jsEmbedPart;
	KJSEmbed::JSConsoleWidget* m_jsConsoleWidget;
	std::vector<KisKJSEmbedScript*> m_vScripts;
	QPopupMenu* m_scriptMenu;
};

class KisPaintDeviceKJSImp : public KJSEmbed::JSProxyImp
{
public:
	KisPaintDeviceKJSImp( KJS::ExecState *exec, int id  );
	virtual ~KisPaintDeviceKJSImp();
	
	static void addBindings( KJS::ExecState *exec, KJS::Object &object );
};


#endif
