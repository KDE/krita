/* This file is part of the KDE project
   Copyright (C) 2009 Jeremias Epperlein <jeeree@web.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "FormulaCommand.h"
#include "FormulaCommandUpdate.h"
#include "FormulaCursor.h"
#include <klocalizedstring.h> 
#include "FormulaData.h"


FormulaCommandUpdate::FormulaCommandUpdate (KoFormulaShape* shape, FormulaCommand* command )
                    : KUndo2Command ()
{
    m_shape=shape;
    m_command=command;
    setText( m_command->text() );
}

void FormulaCommandUpdate::redo()
{
    m_shape->update();
    m_command->redo();
    m_shape->updateLayout();
    m_shape->update();
    m_shape->formulaData()->notifyDataChange(m_command,false);
}

void FormulaCommandUpdate::undo()
{
    m_shape->update();
    m_command->undo();
    m_shape->updateLayout();
    m_shape->update();
    m_shape->formulaData()->notifyDataChange(m_command,true);
}

