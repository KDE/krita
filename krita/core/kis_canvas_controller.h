/*
 *  Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
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

#if !defined KIS_CANVAS_CONTROLLER_H_
#define KIS_CANVAS_CONTROLLER_H_

#include <qglobal.h>

class QRect;
class KisTool;

class KisCanvasControllerInterface {
public:
	KisCanvasControllerInterface();
	virtual ~KisCanvasControllerInterface();

public:
	virtual void activateTool(KisTool *tool) = 0;
	virtual KisTool *currentTool() const = 0;
	virtual void updateCanvas() = 0;
	virtual void updateCanvas(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h) = 0;
	virtual void updateCanvas(const QRect& rc) = 0;

private:
	KisCanvasControllerInterface(const KisCanvasControllerInterface&);
	KisCanvasControllerInterface& operator=(const KisCanvasControllerInterface&);
};

#endif // KIS_CANVAS_CONTROLLER_H_

