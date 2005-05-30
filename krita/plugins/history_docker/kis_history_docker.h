/* This file is part of the KDE project

   Copyright (C) 2001, 2002, 2003 The Karbon Developers
   Copyright (C) 2005 Boudewijn Rempt

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

#ifndef __KIS_HISTORY_DOCKER_H__
#define __KIS_HISTORY_DOCKER_H__

#include <qlistview.h>
#include <qptrdict.h>

class QHButtonGroup;
class QPoint;
class QLabel;
class QPixmap;
class QCheckBox;


class VHistoryItem;
 
class VHistoryGroupItem : public QListViewItem
{
	public:
		VHistoryGroupItem( VHistoryItem* item, QListView* parent, QListViewItem* after );
		~VHistoryGroupItem();

		void paintCell( QPainter* p, const QColorGroup& cg, int column, int width, int align );

		virtual QString key( int, bool ) const { return m_key; }
		virtual int rtti() const { return 1001; }

	private:
		QString	m_key;
}; // VHistoryGroupItem
 
class VHistoryItem : public QListViewItem
{
	public:
		VHistoryItem( VCommand* command, QListView* parent, QListViewItem* after );
		VHistoryItem( VCommand* command, VHistoryGroupItem* parent, QListViewItem* after );
		~VHistoryItem();

		VCommand* command() { return m_command; }

		void paintCell( QPainter* p, const QColorGroup& cg, int column, int width, int align );

		virtual QString key( int, bool ) const { return m_key; }
		virtual int rtti() const { return 1002; }

	private:
		void init();

		QString     m_key;
		VCommand*   m_command;
}; // VHistoryItem

class VHistoryTab : public QWidget
{
	Q_OBJECT

	public:
		VHistoryTab( KarbonPart* part, QWidget* parent );
		~VHistoryTab();

		bool groupingEnabled();

	public slots:
		void historyCleared();
		void commandExecuted( VCommand* command );
		void slotCommandAdded( VCommand* command );
		void removeFirstCommand();
		void removeLastCommand();

		void commandClicked( int button, QListViewItem* item, const QPoint& point, int col );
		void groupingChanged( int );

	signals:
		void undoCommand( VCommand* command );
		void redoCommand( VCommand* command );
		void undoCommandsTo( VCommand* command );
		void redoCommandsTo( VCommand* command );

	private:
		QListView*      m_history;
		QListViewItem*  m_lastCommand;
		QCheckBox*      m_groupCommands;
		long            m_lastCommandIndex;

		KarbonPart*     m_part;
}; // VHistoryTab


#endif /* __KIS_HISTORY_DOCKER_H__ */
