/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2021 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisCanvasOnlyAction.h"
#include <KisPart.h>
#include <kactioncollection.h>
#include <QAction>
#include <kis_debug.h>

KisCanvasOnlyAction::KisCanvasOnlyAction()
    : KisAbstractInputAction("Toggle Canvas Only")
{
    setName(i18n("Toggle Canvas Only"));
    setDescription(i18n("The Toggle Canvas Only action switches between full interface and canvas only mode"));  
}

KisCanvasOnlyAction::~KisCanvasOnlyAction()
{

}

int KisCanvasOnlyAction::priority() const
{
    return 6;
}

void KisCanvasOnlyAction::begin(int, QEvent */*event*/)
{
    KActionCollection *actionCollection = KisPart::instance()->currentMainwindow()->actionCollection();
    QAction *action = actionCollection->action("view_show_canvas_only");
    if (action)
    {
        action->trigger();
    }
}
