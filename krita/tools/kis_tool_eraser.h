/*
 *  kis_tool_eraser.h - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter
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

#ifndef __erasertool_h__
#define __erasertool_h__

#include <qpoint.h>

#include "kis_tool.h"

class KisBrush;
class KisDoc;
class BrushTool;

class EraserTool : public BrushTool {
	typedef BrushTool super;

public:
	EraserTool(KisDoc *doc, KisBrush *brush);
	virtual ~EraserTool();
  
	virtual void setupAction(QObject *collection);
	virtual QDomElement saveSettings(QDomDocument& doc) const;
	virtual bool loadSettings(QDomElement& elem);

	virtual bool paint(const QPoint& pos);
	virtual void mousePress(QMouseEvent*); 

protected:
	QCursor defaultCursor() const;
};

#endif

