/* This file is part of the KDE project
   Copyright (C) 2002 Laurent Montel <lmontel@mandrakesoft.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef KRAYON_VIEW_IFACE_H
#define KRAYON_VIEW_IFACE_H

#include <KoViewIface.h>

#include <qstring.h>

class KisView;

/**
 * This is the definition of the interface Krita presents to
 * dcop.
 */
class KRayonViewIface : public KoViewIface
{
	K_DCOP
		public:
	KRayonViewIface( KisView *view_ );
 k_dcop:
	void copy();
	void cut();
	void removeSelection();
	void paste();
	void copySelectionToNewLayer();
	void selectAll();
	void unSelectAll();

	// dialog action slots
	void dialog_gradient();

	// layer action slots
	void next_layer();
	void previous_layer();

	void slotImportImage();
	void export_image();
	void imgResizeToActiveLayer();
	void add_new_image_tab();
	void remove_current_image_tab();
	//void imageResize(); // XXX: made into a plugin, don't know yet how to export a plugin with DCOP.

	// tool action slots
	void tool_properties();

	// settings action slots
	void preferences();

	void rotateLayer180();
	void rotateLayerLeft90();
	void rotateLayerRight90();
	void rotateLayerCustom();
	void mirrorLayerX();
	void mirrorLayerY();

 private:
	KisView *m_view;
};

#endif
