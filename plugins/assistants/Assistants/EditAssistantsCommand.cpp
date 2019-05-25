/*
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

#include "EditAssistantsCommand.h"

#include <QListIterator>

#include <kis_canvas2.h>
#include <KisView.h>
#include <KisDocument.h>
#include <KisViewManager.h>
#include <kis_canvas_resource_provider.h>
#include <kis_painting_assistants_decoration.h>

EditAssistantsCommand::EditAssistantsCommand(QPointer<KisCanvas2> canvas, AssistantSPList origAssistants, AssistantSPList newAssistants, KUndo2Command *parent)
    : KUndo2Command(kundo2_i18n("Edit Assistants"), parent)
    , m_canvas(canvas)
    , m_origAssistants(origAssistants)
    , m_newAssistants(newAssistants)
    , m_index(-1)
    , m_firstRedo(true)
    , m_type(EDIT)
{
}

EditAssistantsCommand::EditAssistantsCommand(QPointer<KisCanvas2> canvas, AssistantSPList origAssistants, AssistantSPList newAssistants, Type type, int index, KUndo2Command *parent)
    : KUndo2Command((type == ADD ? kundo2_i18n("Add Assistant") : kundo2_i18n("Remove Assistant")), parent)
    , m_canvas(canvas)
    , m_origAssistants(origAssistants)
    , m_newAssistants(newAssistants)
    , m_index(index)
    , m_firstRedo(true)
    , m_type(type)
{
    KIS_ASSERT_RECOVER_RETURN(type != EDIT);
}

void EditAssistantsCommand::replaceWith(AssistantSPList assistants, Type type)
{
    AssistantSPList curAssistants = m_canvas->paintingAssistantsDecoration()->assistants();
    if (type == EDIT) {
        KIS_ASSERT_RECOVER_RETURN(curAssistants.size() == assistants.size());
    } else if (type == ADD) {
        KIS_ASSERT_RECOVER_RETURN(curAssistants.size() == assistants.size() - 1);
    } else { // type == REMOVE
        KIS_ASSERT_RECOVER_RETURN(curAssistants.size() == assistants.size() + 1);
    }


    // when undo/redoing,
    // keep locations, don't touch the current display configuration
    int i = 0;
    for (QListIterator<KisPaintingAssistantSP> cur(curAssistants), dest(assistants); cur.hasNext() && dest.hasNext(); ++i) {
        KisPaintingAssistantSP current = cur.next(), target = dest.next();
        if (i == m_index) {
            if (type == ADD) { // we will add an assistant to the canvas now
                target = dest.next(); // pass this one as it is not in `cur'
            } else {
                current = cur.next();
            }
        }
        KIS_ASSERT_RECOVER_RETURN(current->id() == target->id());
        target->setAssistantCustomColor(current->assistantCustomColor());
        target->setUseCustomColor(current->useCustomColor());
        target->setSnappingActive(current->isSnappingActive());
        target->uncache();
    }

    Q_FOREACH (KisPaintingAssistantSP assistant, curAssistants) {
        KisAbstractPerspectiveGrid* grid = dynamic_cast<KisAbstractPerspectiveGrid*>(assistant.data());
        if (grid) {
            m_canvas->viewManager()->canvasResourceProvider()->removePerspectiveGrid(grid);
        }
    }

    m_canvas->imageView()->document()->setAssistants(assistants);

    Q_FOREACH (KisPaintingAssistantSP assistant, assistants) {
        KisAbstractPerspectiveGrid* grid = dynamic_cast<KisAbstractPerspectiveGrid*>(assistant.data());
        if (grid) {
            m_canvas->viewManager()->canvasResourceProvider()->addPerspectiveGrid(grid);
        }
    }

    m_canvas->updateCanvas();
}

void EditAssistantsCommand::undo()
{
    replaceWith(m_origAssistants, Type(-m_type));
}

void EditAssistantsCommand::redo()
{
    // this is a post-execution command
    if (m_firstRedo) {
        m_firstRedo = false;
        return;
    }
    replaceWith(m_newAssistants, m_type);
}
