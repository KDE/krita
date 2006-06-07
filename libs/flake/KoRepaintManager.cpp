/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoRepaintManager.h"
#include "KoCanvasBase.h"
#include "KoShape.h"
#include "KoSelection.h"
#include "KoTool.h"

#include <QRectF>
#include <QDebug>

KoRepaintManager::KoRepaintManager(KoCanvasBase *canvas, KoSelection *selection)
: m_canvas (canvas)
, m_selection(selection)
, m_active (true)
, m_locked(false)
, m_otherManagers()
, m_refCount(0)
{
}

void KoRepaintManager::addChainedManager(KoRepaintManager *manager) {
    if(manager == this)
        return;
    if(! m_otherManagers.contains(manager)) {
        m_otherManagers.append(manager);
        manager->addUser();
    }
}

void KoRepaintManager::dismantle() {
    m_canvas = 0;
    m_active = false;
}

void KoRepaintManager::repaint(QRectF &rect, const KoShape *shape, bool selectionHandles) {
    if(!m_active || m_locked) // lock out cyclic links
        return;
    m_canvas->updateCanvas(rect);
    if(selectionHandles && m_selection->isSelected(shape)) {
        if(m_canvas->tool())
            m_canvas->tool()->repaintDecorations();
    }
    m_locked = true;
    foreach(KoRepaintManager *manager, m_otherManagers) {
        if(! manager->isActive()) {
            manager->removeUser();
            m_otherManagers.removeAll(manager);
            if(manager->refCount() == 0)
                delete manager;
        }
        else
            manager->repaint(rect, shape, selectionHandles);
    }
    m_locked = false;
}
