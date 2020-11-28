/*
 *  SPDX-FileCopyrightText: 2018 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
