/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_change_frame_action.h"

#include <klocalizedstring.h>
#include "kis_action.h"
#include "kis_input_manager.h"
#include "kis_canvas2.h"
#include "KisViewManager.h"
#include "kis_action_manager.h"
#include "kis_node.h"


struct KisChangeFrameAction::Private
{
};

KisChangeFrameAction::KisChangeFrameAction()
    : KisAbstractInputAction("Switch Time"),
      m_d(new Private)
{
    setName(i18n("Switch Time"));
    setDescription(i18n("The <i>Switch Time</i> action changes the current time of the animation."));

    QHash< QString, int > shortcuts;
    shortcuts.insert(i18n("Next Frame"), NextFrameShortcut);
    shortcuts.insert(i18n("Previous Frame"), PreviousFrameShortcut);
    setShortcutIndexes(shortcuts);
}

KisChangeFrameAction::~KisChangeFrameAction()
{
}

bool KisChangeFrameAction::isAvailable() const
{
    KisNodeSP node = inputManager()->canvas()->viewManager()->activeNode();

    return node ? node->isAnimated() : false;
}

void KisChangeFrameAction::begin(int shortcut, QEvent *event)
{
    KisAbstractInputAction::begin(shortcut, event);

    switch(shortcut) {
    case NextFrameShortcut: {
        KisAction *action = inputManager()->canvas()->viewManager()->actionManager()->actionByName("next_frame");
        if (action) {
            action->trigger();
        }
        break;
    }
    case PreviousFrameShortcut: {
        KisAction *action = inputManager()->canvas()->viewManager()->actionManager()->actionByName("previous_frame");
        if (action) {
            action->trigger();
        }
        break;
    }
    }
}
