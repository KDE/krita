/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 Geoffry Song <goffrie@gmail.com>
 *  Copyright (c) 2017 Scott Petrovic <scottpetrovic@gmail.com>
 *  Copyright (c) 2019 Tusooa Zhu <tusooa@vista.aero>
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

#include "AddRemoveAssistantCommand.h"

#include <kis_canvas2.h>
#include <kis_abstract_perspective_grid.h>
#include <KisViewManager.h>
#include <kis_canvas_resource_provider.h>
#include <kis_painting_assistants_decoration.h>

AddRemoveAssistantCommand::AddRemoveAssistantCommand(Type type, QPointer<KisCanvas2> canvas, KisPaintingAssistantSP assistant, KUndo2Command *parent)
    : KUndo2Command((type == ADD ? kundo2_i18n("Add Assistant") : kundo2_i18n("Remove Assistant")), parent)
    , m_type(type)
    , m_canvas(canvas.data())
    , m_assistant(assistant)
{
}

AddRemoveAssistantCommand::~AddRemoveAssistantCommand()
{
}

void AddRemoveAssistantCommand::undo()
{
    if (m_type == ADD) {
        removeAssistant();
    } else {
        addAssistant();
    }
}

void AddRemoveAssistantCommand::redo()
{
    if (m_type == ADD) {
        addAssistant();
    } else {
        removeAssistant();
    }
}

void AddRemoveAssistantCommand::addAssistant()
{
    m_canvas->paintingAssistantsDecoration()->addAssistant(m_assistant);

    KisAbstractPerspectiveGrid* grid = dynamic_cast<KisAbstractPerspectiveGrid*>(m_assistant.data());
    if (grid) {
        m_canvas->viewManager()->canvasResourceProvider()->addPerspectiveGrid(grid);
    }
}

void AddRemoveAssistantCommand::removeAssistant()
{
    KisAbstractPerspectiveGrid* grid = dynamic_cast<KisAbstractPerspectiveGrid*>(m_assistant.data());
    if (grid) {
        m_canvas->viewManager()->canvasResourceProvider()->removePerspectiveGrid(grid);
    }
    m_canvas->paintingAssistantsDecoration()->removeAssistant(m_assistant);
}
