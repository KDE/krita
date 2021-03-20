/*
 *  SPDX-FileCopyrightText: 2019 Tusooa Zhu <tusooa@vista.aero>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QPointer>

#include <kundo2command.h>
#include <kis_painting_assistant.h>

class KisCanvas2;

class EditAssistantsCommand : public KUndo2Command
{
    using AssistantSPList = QList<KisPaintingAssistantSP>;
public:
    enum Type {
        ADD = -1,
        REMOVE = 1,
        EDIT = 0
    };
    EditAssistantsCommand(QPointer<KisCanvas2> canvas, AssistantSPList origAssistants, AssistantSPList newAssistants, KUndo2Command *parent = 0);
    EditAssistantsCommand(QPointer<KisCanvas2> canvas, AssistantSPList origAssistants, AssistantSPList newAssistants, Type type, int index, KUndo2Command *parent = 0);

    void undo() override;
    void redo() override;

private:
    void replaceWith(AssistantSPList assistants, Type type = EDIT);
    QPointer<KisCanvas2> m_canvas;
    AssistantSPList m_origAssistants, m_newAssistants;
    int m_index;
    bool m_firstRedo;
    Type m_type;
};
