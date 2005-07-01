/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef _KIS_TOOLBOX_H_
#define _KIS_TOOLBOX_H_

#include <qtoolbutton.h>
#include <qptrvector.h>

#include <ktoolbar.h>
#include <kis_tool.h>

class QWidget;

class KMainWindow;
class KDualColorButton;

class KisView;



enum KisToolBoxTypes {
	TOOLBOX_PAINT = 0,
	TOOLBOX_SELECT = 1,
	TOOLBOX_OTHER = 2,
};

/**
 * KisToolBox is a kind of super-specialized toolbox that can order tools according to
 * type and priority.
 *
 * This is to a large extent a port of the Karbon vtoolbox -- with which it should be
 * merged one day.
 */
class KisToolBox : public KToolBar {

	Q_OBJECT

public:
	
	KisToolBox( KisView * view, KMainWindow *mainWin, const char* name = 0L );
	virtual ~KisToolBox();

	// Called by the toolcontroller for each tool. For every category,
	// there is a separate list, and the tool is categorized correctly.
	// The tool is not yet added to the widgets; call setupTools()
	// to do that.
	void registerTool( KisTool * );

	// Called when all tools have been added by the tool controller
	void setupTools();

signals:
	void activeToolChanged( KisTool * );

public slots:
	
	virtual void setOrientation ( Orientation o );
	void slotButtonPressed( int id );
	void slotPressButton( int id );

private:

	QToolButton *addButton( const char* iconName, QString tooltip, int id );
	
private:
	KisView * m_view;

	QButtonGroup * m_buttonGroup;

	KDualColorButton * m_colorButton;
	
	QBoxLayout * m_leftLayout;
	QBoxLayout * m_rightLayout;
	QBoxLayout * m_columnsLayouter;

	QWidget * m_left;
	QWidget * m_right;
		
	bool m_insertLeft;

	typedef QPtrVector<KisTool> ToolList;
	
	QPtrList<ToolList> m_tools;

};

#endif // _KIS_TOOLBOX_H_
