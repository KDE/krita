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

#include "KRayonViewIface.h"

#include "kis_view.h"

#include <dcopclient.h>

KRayonViewIface::KRayonViewIface( KisView *view_ )
	: KoViewIface( view_ )
{
	m_view = view_;
}

void KRayonViewIface::copy()
{
	m_view->copy();
}

void KRayonViewIface::cut()
{
	m_view->cut();
}

void KRayonViewIface::removeSelection()
{
	m_view->removeSelection();
}

void KRayonViewIface::paste()
{
	m_view->paste();
}

void KRayonViewIface::crop()
{
	m_view->crop();
}

void KRayonViewIface::selectAll()
{
	m_view->selectAll();
}

void KRayonViewIface::unSelectAll()
{
	m_view->unSelectAll();
}

void KRayonViewIface::dialog_gradient()
{
	m_view->dialog_gradient();
}

void KRayonViewIface::dialog_colors()
{
	m_view->dialog_colors();
}

void KRayonViewIface::dialog_crayons()
{
	m_view->dialog_crayons();
}

void KRayonViewIface::dialog_brushes()
{
	m_view->dialog_brushes();
}

void KRayonViewIface::dialog_patterns()
{
	m_view->dialog_patterns();
}

void KRayonViewIface::dialog_layers()
{
	m_view->dialog_layers();
}

void KRayonViewIface::dialog_channels()
{
	m_view->dialog_channels();
}

void KRayonViewIface::insert_layer()
{
	m_view->insert_layer();
}

void KRayonViewIface::remove_layer()
{
	m_view->remove_layer();
}

void KRayonViewIface::link_layer()
{
	m_view->link_layer();
}

void KRayonViewIface::hide_layer()
{
	m_view->hide_layer();
}

void KRayonViewIface::next_layer()
{
	m_view->next_layer();
}

void KRayonViewIface::previous_layer()
{
	m_view->previous_layer();
}

void KRayonViewIface::layer_properties() 
{
	m_view->layer_properties();
}
