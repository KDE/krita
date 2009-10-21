/* This file is part of the KDE project
 * Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#include "kis_actions_editor_dialog.h"
#include "kis_actions_editor.h"

#include <klocale.h>

KisActionsEditorDialog::KisActionsEditorDialog(QWidget* parent) : KDialog(parent),
        m_actionsEditor(new KisActionsEditor(this))
{
    setMainWidget(m_actionsEditor);
    setButtons(Cancel | Ok);
    setButtonText(Ok, i18n("Save macro"));
    setButtonText(Cancel, i18n("Discard changes"));
}

KisActionsEditorDialog::~KisActionsEditorDialog()
{
}

KisActionsEditor* KisActionsEditorDialog::actionsEditor()
{
    return m_actionsEditor;
}
