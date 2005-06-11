/*
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

#ifndef _KO_TOOLBOX_PALETTE_
#define _KO_TOOLBOX_PALETTE_

#include <qdockwindow.h>
#include <qtoolbox.h>

#include <koPaletteManager.h>

/**
 * A palette based on a toolbox widget. This does not support drag and drop
 * configuration of palette widgets
 */
class KoToolBoxPalette : public KoPalette {

Q_OBJECT

public:

	KoToolBoxPalette(KoView * parent, const char * name);
	~KoToolBoxPalette();

public:

	virtual void plug(QWidget * widget, const QString & name);
	virtual void unplug(const QWidget * widget);
        void showPage(QWidget *w);
        
private:

	QToolBox * m_page;
};

#endif //_KO_TOOLBOX_PALETTE_
