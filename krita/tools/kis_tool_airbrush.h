/*
 *  kis_tool_airbrush.h - part of KImageShop
 *
 *  Copyright (c) 2004 Boudewijn Rempt
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

#if !defined KIS_TOOL_AIRBRUSH_H
#define KIS_TOOL_AIRBRUSH_H

#include "kis_tool_freehand.h"

class QTimer;
class KisBrush;

class KisToolAirBrush : public KisToolFreeHand {

	Q_OBJECT
	typedef KisToolFreeHand super;
    
public:
	KisToolAirBrush();
	virtual ~KisToolAirBrush();
  
	virtual void setup(KActionCollection *collection);

protected slots:
	void timeoutPaint();  

protected:
	virtual void initPaint(KisEvent *e);
	virtual void endPaint();

private:
	QTimer * m_timer;

	KisBrush *m_dummyBrush; // The airbrush doesn't use a real
				// brush-shape, but still needs a way
				// to get the initial size info into
				// KisPainter.
};

#endif // KIS_TOOL_AIRBRUSH_H
