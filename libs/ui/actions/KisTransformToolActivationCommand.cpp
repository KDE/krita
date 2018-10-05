/*
 *  Copyright (c) 2018 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "KisTransformToolActivationCommand.h"

#include <QApplication>

#include <KoToolManager.h>
#include <KoToolBase.h>
#include "canvas/kis_canvas2.h"

KisTransformToolActivationCommand::KisTransformToolActivationCommand(KisViewManager* view, KUndo2Command * parent)
    : KUndo2Command(kundo2_i18n("Activate transform tool"), parent),  m_firstRedo(true), m_view(view)
{
    connect(this, SIGNAL(requestTransformTool()), m_view, SLOT(slotActivateTransformTool()));
}

KisTransformToolActivationCommand::~KisTransformToolActivationCommand()
{
}

void KisTransformToolActivationCommand::redo()
{
    if(m_firstRedo)
    {
        m_firstRedo = false;
        emit requestTransformTool();
    }
}

void KisTransformToolActivationCommand::undo()
{

}

#include "KisTransformToolActivationCommand.moc"
