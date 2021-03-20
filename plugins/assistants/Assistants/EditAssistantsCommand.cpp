/*
 *  SPDX-FileCopyrightText: 2019 Tusooa Zhu <tusooa@vista.aero>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    Q_UNUSED(m_index);
    KIS_ASSERT_RECOVER_RETURN(type != EDIT);
}

void EditAssistantsCommand::replaceWith(AssistantSPList newAssistants, Type type)
{
    AssistantSPList oldAssistants = m_canvas->paintingAssistantsDecoration()->assistants();

    if (type == ADD) {
        KIS_ASSERT_RECOVER_RETURN(newAssistants.size() > oldAssistants.size());
    } else if (type == REMOVE) {
        KIS_ASSERT_RECOVER_RETURN(newAssistants.size() < oldAssistants.size());
    }

    Q_FOREACH (KisPaintingAssistantSP assistant, oldAssistants) {
        KisAbstractPerspectiveGrid* grid = dynamic_cast<KisAbstractPerspectiveGrid*>(assistant.data());
        if (grid) {
            m_canvas->viewManager()->canvasResourceProvider()->removePerspectiveGrid(grid);
        }
    }

    m_canvas->paintingAssistantsDecoration()->setAssistants(newAssistants);

    Q_FOREACH (KisPaintingAssistantSP assistant, newAssistants) {
        assistant->uncache();
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
