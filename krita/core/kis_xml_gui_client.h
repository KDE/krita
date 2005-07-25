/*
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
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

#ifndef KIS_XML_GUI_CLIENT_H_
#define KIS_XML_GUI_CLIENT_H_

#include <kxmlguiclient.h>

/**
 * Class so that plugins can merge more easily their XML GUI dynamically. For example,
 * when the current layer changes to a color model with texture support, this could
 * be used to have a seperate rc file defining a texture menu to merge with the krita GUI.
 * 
 * This is provided because the setXMLFile function of KXMLGUIClient is protected, and hence
 * can't be accessed directly by a plugin.
 **/
class KisXMLGUIClient : public KXMLGUIClient
{
public:
	typedef KXMLGUIClient super;
	KisXMLGUIClient(KXMLGUIClient *parent, const QString& rcFile) : super(parent) {
		setRcFile(rcFile);
	}
	KisXMLGUIClient(const QString& rcFile) : super() {
		setRcFile(rcFile);
	}
protected:
	void setRcFile(const QString& rcFile) {
		QString notused;
		QStringList files = KGlobal::dirs() -> findAllResources( "data", rcFile );
		kdDebug(DBG_AREA_CORE) << files << endl;
		setXMLFile( findMostRecentXMLFile( files, notused ) );
	}
};

#endif // KIS_XML_GUI_CLIENT_H_
