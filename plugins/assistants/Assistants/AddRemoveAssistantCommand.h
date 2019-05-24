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

#ifndef ADD_REMOVE_ASSISTANT_COMMAND_H_
#define ADD_REMOVE_ASSISTANT_COMMAND_H_

#include <kundo2command.h>
#include <kis_painting_assistant.h>
#include <kis_types.h>

#include <QPointer>

class KisCanvas2;

class AddRemoveAssistantCommand : public KUndo2Command
{
public:
    enum Type {
        ADD,
        REMOVE
    };

    AddRemoveAssistantCommand(Type type, QPointer<KisCanvas2> canvas, KisPaintingAssistantSP assistant, KUndo2Command *parent = 0);
    ~AddRemoveAssistantCommand() override;

    void undo() override;
    void redo() override;

private:
    void addAssistant();
    void removeAssistant();

    Type m_type;
    QPointer<KisCanvas2> m_canvas;
    KisPaintingAssistantSP m_assistant;
};

#endif
