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
