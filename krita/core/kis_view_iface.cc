/* 
 * This file is part of the KDE project
 *
 * Copyright (C) 2002 Laurent Montel <lmontel@mandrakesoft.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "kis_view_iface.h"

#include "kis_view.h"

#include <dcopclient.h>

KisViewIface::KisViewIface( KisView *view_ )
    : KoViewIface( view_ )
{
    m_view = view_;
}

void KisViewIface::copy()
{
//     m_view->copy();
}

void KisViewIface::cut()
{
//     m_view->cut();
}

void KisViewIface::removeSelection()
{
//     m_view->removeSelection();
}

void KisViewIface::paste()
{
//     m_view->paste();
}

void KisViewIface::copySelectionToNewLayer()
{
//     m_view->copySelectionToNewLayer();
}

void KisViewIface::selectAll()
{
//     m_view->selectAll();
}

void KisViewIface::unSelectAll()
{
//     m_view->unSelectAll();
}



void KisViewIface::slotImportImage()
{
}


void KisViewIface::rotateLayer180()
{
    m_view->rotateLayer180();
}

void KisViewIface::rotateLayerLeft90()
{
    m_view->rotateLayerLeft90();
}

void KisViewIface::rotateLayerRight90()
{
    m_view->rotateLayerRight90();
}

void KisViewIface::mirrorLayerX()
{
    m_view->mirrorLayerX();
}

void KisViewIface::mirrorLayerY()
{
    m_view->mirrorLayerY();
}
