/*
 *  polylinetool.h - part of Krayon
 *
 *  Copyright (c) 2000 John Califf <jcaliff@comuzone.net>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __polylinetool_h__
#define __polylinetool_h__

#include <qpoint.h>

#include "kis_tool.h"

class KisDoc;
class KisView;
class KisCanvas;

class PolyLineTool : public KisTool {
public:
	PolyLineTool(KisDoc *doc, KisCanvas *canvas);
	virtual ~PolyLineTool();

	virtual void optionsDialog();
	virtual QDomElement saveSettings(QDomDocument& doc) const;
	virtual bool loadSettings(QDomElement& elem);
	virtual void setupAction(QObject *collection);

	virtual void mousePress( QMouseEvent* event );
	virtual void mouseMove( QMouseEvent* event );
	virtual void mouseRelease( QMouseEvent* event );

	void start(QPoint p);
	void finish(QPoint p); 
    
protected:
	void drawLine( const QPoint&, const QPoint& );

private:

	int         m_lineThickness;

	QPoint      m_dragStart;
	QPoint      m_dragEnd;
	QPoint      mStart;
	QPoint      mFinish;

	bool        m_dragging;
	bool        m_done;
};

#endif //__polylinetool_h__
