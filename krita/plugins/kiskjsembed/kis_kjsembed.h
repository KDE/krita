/*
 * This file is part of Krita
 *
 * Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
class KoTabbedToolDock;

namespace Krita {
	namespace Plugins {
		namespace KisKJSEmbed {
			class Script;
			namespace Bindings {
				class FunctionsFactory;
				class ObjectsFactory;
			};
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
				void viewConsoleDocker(bool v);
			private:
				KisView* m_view;
				KFileDialog* m_fileDialog;
				KJSEmbed::KJSEmbedPart* m_jsEmbedPart;
				KJSEmbed::JSConsoleWidget* m_jsConsoleWidget;
				KoTabbedToolDock* m_consoleDocker;
				Bindings::FunctionsFactory* m_functionsFactory;
				Bindings::ObjectsFactory* m_objectsFactory;
				std::vector<Script*> m_vScripts;
				QPopupMenu* m_scriptMenu;
			};
		};
	};
};


#endif
