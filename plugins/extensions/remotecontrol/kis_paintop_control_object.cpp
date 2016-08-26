/*
 *
 * Copyright (c) 2016 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_paintop_control_object.h"
#include <KisViewManager.h>
#include <kis_canvas_resource_provider.h>
#include <kis_paintop_preset.h>
#include <kis_paintop_settings.h>

KisPaintopControlObject::KisPaintopControlObject(KisViewManager* view, QObject *parent) : QObject(parent)
, m_view(view)
{
    setObjectName("PaintOp");
}

void KisPaintopControlObject::setBrushSize(double size)
{
    if(!m_view || !m_view->resourceProvider()) {
        return;
    }
    
    m_view->resourceProvider()->currentPreset()->settings()->setPaintOpSize(size);

}

double KisPaintopControlObject::brushSize()
{
    if(!m_view || !m_view->resourceProvider()) {
        return 0.0;
    }
 
    return m_view->resourceProvider()->currentPreset()->settings()->paintOpSize();
}

double KisPaintopControlObject::flow()
{
    if(!m_view || !m_view->resourceProvider()) {
        return 0.0;
    }

    return m_view->resourceProvider()->currentPreset()->settings()->paintOpFlow();
}

void KisPaintopControlObject::setFlow(double flow)
{
    if(!m_view || !m_view->resourceProvider()) {
        return;
    }

    m_view->resourceProvider()->currentPreset()->settings()->setPaintOpFlow(flow);
}

double KisPaintopControlObject::opacity()
{
    if(!m_view || !m_view->resourceProvider()) {
        return 0.0;
    }

    return m_view->resourceProvider()->currentPreset()->settings()->paintOpOpacity();
}

void KisPaintopControlObject::setOpacity(double opacity)
{
    if(!m_view || !m_view->resourceProvider()) {
        return;
    }

    m_view->resourceProvider()->currentPreset()->settings()->setPaintOpOpacity(opacity);
}



