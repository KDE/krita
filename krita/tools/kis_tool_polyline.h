/*
 *  polylinetool.h - part of Krayon
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

#ifndef __polylinetool_h__
#define __polylinetool_h__

#include <qpoint.h>

#include "kis_tool.h"
#include "kis_tool_line.h"

class KisDoc;
class KisCanvas;

class PolyLineTool : public LineTool {
	typedef LineTool super;

public:
	PolyLineTool(KisDoc *doc, KisCanvas *canvas);
	virtual ~PolyLineTool();

	QString settingsName() const;

	virtual void setupAction(QObject *collection);

	virtual void mousePress(QMouseEvent *event);
	virtual void mouseRelease(QMouseEvent *event);
};

#endif //__polylinetool_h__
