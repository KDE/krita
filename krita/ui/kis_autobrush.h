/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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
 
#ifndef _KIS_AUTOBRUSH_H_
#define _KIS_AUTOBRUSH_H_

#include "kis_wdg_autobrush.h"
#include "kis_brush.h"

class KisAutobrushResource : public KisBrush
{
	public:
		KisAutobrushResource(QImage& img) : KisBrush("")
		{
			setImage(img);
			setBrushType(MASK);
		};
	public:
		virtual bool loadAsync() { return false; };
};

typedef double (*NormeSquare)(double, double);

class KisAutobrush : public KisWdgAutobrush
{
	Q_OBJECT
public:
	KisAutobrush(QWidget *parent, const char* name, const QString& caption);
signals:
	void activatedResource(KisResource *r);
private:
	void createBrush(Q_INT32, Q_INT32, Q_INT32, Q_INT32, NormeSquare );
private slots:
	void paramChanged();
	void spinBoxWidthChanged(int );
	void spinBoxHeigthChanged(int );
	void spinBoxHorizontalChanged(int);
	void spinBoxVerticalChanged(int);
private:
	QImage* m_brsh;
};

#endif
