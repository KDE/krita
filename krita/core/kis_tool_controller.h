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

#ifndef KIS_TOOL_CONTROLLER_H_
#define KIS_TOOL_CONTROLLER_H_

class KisTool;

class KisToolControllerInterface {
public:
	KisToolControllerInterface();
	virtual ~KisToolControllerInterface();

public:
	virtual void setCurrentTool(KisTool *tool) = 0;
	virtual KisTool *currentTool() const = 0;

private:
	KisToolControllerInterface(const KisToolControllerInterface&);
	KisToolControllerInterface& operator=(const KisToolControllerInterface&);
};

#endif // KIS_TOOL_CONTROLLER_H_

