/*
 *  linetool.h - part of Krayon
 *
 *  Copyright (c) 2000 John Califf <jcaliff@comuzone.net>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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

#ifndef __linetool_h__
#define __linetool_h__

#include <qpoint.h>

class KisDoc;
class KisView;
class KisCanvas;

class LineTool : public KisTool {
	typedef KisTool super;

public:
	LineTool(KisDoc *doc, KisCanvas *canvas);
	virtual ~LineTool();

	virtual void setupAction(QObject *collection);
	
	virtual QString settingsName() const;
	virtual QDomElement saveSettings(QDomDocument& doc) const;
	virtual bool loadSettings(QDomElement& elem);

	virtual void mousePress(QMouseEvent *event);
	virtual void mouseMove(QMouseEvent *event);
	virtual void mouseRelease(QMouseEvent *event);

	virtual void optionsDialog();
	virtual void toolSelect();
  
protected:
	virtual void draw(const QPoint& start, const QPoint& end);
    
protected:
	int m_lineThickness;
	bool m_dragging;

	QPoint m_dragStart;
	QPoint m_dragEnd;
};

#endif //__linetool_h__

