/*
 *  kis_tool_eraser.h - part of Krita
 *
 *  Copyright (c) 1999 Matthias Elter
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_TOOL_ERASER_H_
#define KIS_TOOL_ERASER_H_

#include "kis_tool_freehand.h"

#include "kis_tool_factory.h"

class KisToolEraser : public KisToolFreehand {

	typedef KisToolFreehand super;
	Q_OBJECT

public:
	KisToolEraser();
	virtual ~KisToolEraser();
  
	virtual void setup(KActionCollection *collection);

protected:

	virtual void initPaint(KisEvent *e);
};


class KisToolEraserFactory : public KisToolFactory {
	typedef KisToolFactory super;
public:
	KisToolEraserFactory(KActionCollection * ac) : super(ac) {};
	virtual ~KisToolEraserFactory(){};
	
	virtual KisTool * createTool() { KisTool * t =  new KisToolEraser(); t -> setup(m_ac); return t; }
	virtual QString name() { return i18n("Eraser tool"); }
};


#endif // KIS_TOOL_ERASER_H_

