/*
 *  colorpicker.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter <me@kde.org>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <kaction.h>

#include "kis_tool_colorpicker.h"
#include "kis_doc.h"
#include "kis_view.h"
#include "kis_cursor.h"

ColorPicker::ColorPicker(KisDoc *doc) : KisTool(doc)
{
    m_cursor = KisCursor::pickerCursor();
}

ColorPicker::~ColorPicker() {}

KoColor ColorPicker::pick(int x, int y)
{
#if 0
    KisImage * img = m_doc->current();
    KisLayer *lay = img->getCurrentLayer();
    
    if (!img) return KoColor::white();
    if (!lay) return KoColor::white();

    // FIXME: Implement this for non-RGB modes.
    if (!img->colorMode() == cm_RGB && !img->colorMode() == cm_RGBA)
	    return KoColor::white();

    int r = lay->pixel(0, x, y);
    int g = lay->pixel(1, x, y);
    int b = lay->pixel(2, x, y);
        
    return KoColor(r, g,  b, cs_RGB);
#endif
    return KoColor(0, 0, 0, cs_RGB);
}

void ColorPicker::mousePress(QMouseEvent *e)
{
    KisImage * img = m_doc->current();
    if (!img) return;

    if (e->button() != QMouseEvent::LeftButton
    && e->button() != QMouseEvent::RightButton)
        return;

    if( !img->getCurrentLayer()->visible() )
        return;

    QPoint pos = e->pos();
    pos = zoomed(pos);
          
    if( !img->getCurrentLayer()->imageExtents().contains(pos))
        return;
  
    if (e->button() == QMouseEvent::LeftButton)
        m_view->slotSetFGColor(pick(pos.x(), pos.y()));
        
    else if (e->button() == QMouseEvent::RightButton)
        m_view->slotSetBGColor(pick(pos.x(), pos.y()));
}

void ColorPicker::setupAction(QObject *collection)
{
	KToggleAction *toggle = new KToggleAction(i18n("&Color picker"), "colorpicker", 0, this, SLOT(toolSelect()), collection, "tool_colorpicker");

        toggle -> setExclusiveGroup("tools");
}

