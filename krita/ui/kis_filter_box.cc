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


#include <qlistbox.h>
#include <qwidget.h>
#include <qstringlist.h>
#include <qstring.h>

#include <kdebug.h>

#include "kis_filter_registry.h"
#include "kis_filter.h"
#include "kis_canvas_subject.h"

#include "kis_filter_box.h"

KisFilterBox::KisFilterBox(KisCanvasSubject * subject, QWidget * parent, const char * name)
	: super( parent,  name ),
	  m_subject( subject )
{}


KisFilterBox::~KisFilterBox()
{
}

void KisFilterBox::init()
{
	// Fill the box with all the filter that can be used to paint with
	clear();
	QStringList filters = m_subject -> filterList();
	for ( QStringList::Iterator it = filters.begin(); it != filters.end(); ++ it ) {
		KisFilterSP f = m_subject -> filterGet( *it );
		if ( f -> supportsPainting() ) {
			insertItem( f -> name() );
		}
	}
 	connect(this, SIGNAL(currentChanged( QListBoxItem * )), this, SLOT( slotFilterSelected( QListBoxItem * ) ));
}


void KisFilterBox::slotFilterSelected(QListBoxItem * item) {
	// Get the current tool
	// Set the paintop to filter
	// Set the config widget to the config widget of the current filter
}
#include "kis_filter_box.moc"
