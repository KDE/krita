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

void KRayonViewIface::copySelectionToNewLayer()
{
	m_view->copySelectionToNewLayer();
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


void KRayonViewIface::next_layer()
{
	m_view->next_layer();
}

void KRayonViewIface::previous_layer()
{
	m_view->previous_layer();
}

void KRayonViewIface::slotImportImage()
{
    m_view->slotImportImage();
}

void KRayonViewIface::export_image()
{
    m_view->export_image();
}

void KRayonViewIface::imgResizeToActiveLayer()
{
    m_view->imgResizeToActiveLayer();
}

void KRayonViewIface::add_new_image_tab()
{
    m_view->add_new_image_tab();
}

void KRayonViewIface::remove_current_image_tab()
{
    m_view->remove_current_image_tab();
}

// void KRayonViewIface::imageResize()
// {
//     m_view->imageResize();
// }

void KRayonViewIface::preferences()
{
    m_view->preferences();
}

void KRayonViewIface::rotateLayer180()
{
    m_view->rotateLayer180();
}

void KRayonViewIface::rotateLayerLeft90()
{
    m_view->rotateLayerLeft90();
}

void KRayonViewIface::rotateLayerRight90()
{
    m_view->rotateLayerRight90();
}

void KRayonViewIface::rotateLayerCustom()
{
    m_view->rotateLayerCustom();
}

void KRayonViewIface::mirrorLayerX()
{
    m_view->mirrorLayerX();
}

void KRayonViewIface::mirrorLayerY()
{
    m_view->mirrorLayerY();
}
