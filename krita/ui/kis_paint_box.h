/*
 * This file is part of Krita
 *
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef _KIS_PAINT_BOX_H_
#define _KIS_PAINT_BOX_H_

#include "kis_basedocker.h"
#include "kis_types.h"

class QToolBox;
class QIconSet;
class QString;

/**
 * The paint box is a toolbox that contains all
 * the paintops, that is, all the artist's materials
 * in neatly categorized drawers, simulating a painter's
 * field box. Every tool can work with every paint op, in
 * theory.
 *
 * You can plugin intelligent widgets that can collect paintops
 * filters, or whatever thing looks like the stuff you find in a
 * painter's chest.
 *
 */
class KisPaintBox : public KisBaseDocker {

	Q_OBJECT
	typedef KisBaseDocker super;

public:

	KisPaintBox(QWidget * parent = 0, const char * name = 0);
	virtual ~KisPaintBox();

	/// Plug a new entry into the stack
	void plug( QWidget *w );
	void plug(QWidget *w, const QString & label);
	void plug(QWidget *w, const QString & label, const QIconSet & iconset);

	 /// Get an entry
	QWidget * getWidget(const QString & label);

	/// Remove an entry from the stack
        void unplug(QWidget *w);

	/// Show a particular entry from the stack
        void showPage(QWidget *w);

private:

	/// Ask all know paintops to add themselves to where they want to go
	void addPaintOps();

private:

	QToolBox * m_toolbox;


};

#endif // _KIS_PAINT_BOX_H_
