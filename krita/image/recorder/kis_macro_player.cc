/*
 *  Copyright (c) 2007,2011 Cyrille Berger <cberger@cberger.net>
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

#include "kis_macro_player.h"

#include <klocale.h>

#include <KoProgressUpdater.h>
#include <KoUpdater.h>

#include "kis_image.h"
#include "kis_debug.h"
#include "kis_macro.h"
#include "kis_node_query_path.h"
#include "kis_play_info.h"
#include "kis_recorded_action.h"
#include "kis_undo_adapter.h"

struct KisMacroPlayer::Private
{
    Private(const KisPlayInfo& _info) : info(_info), updater(0) {}
    bool paused;
    KisMacro* macro;
    KisPlayInfo info;
    KoUpdater* updater;
};

KisMacroPlayer::KisMacroPlayer(KisMacro* _macro, const KisPlayInfo& info, KoUpdater * updater, QObject* _parent ) : QThread(_parent), d(new Private(info))
{
    d->macro = _macro;
    d->updater = updater;
}

KisMacroPlayer::~KisMacroPlayer()
{
    delete d;
}

void KisMacroPlayer::pause()
{
    d->paused = true;
}

void KisMacroPlayer::resume()
{
    d->paused = false;
}

void KisMacroPlayer::run()
{
    d->paused = false;
    QList<KisRecordedAction*> actions = d->macro->actions();

    if (actions.size() < 1) {
        return;
    }

    dbgImage << "Start playing macro with " << actions.size() << " actions";
    if (d->info.undoAdapter()) {
        d->info.undoAdapter()->beginMacro(i18n("Play macro"));
    }

    KoProgressUpdater* progressUpdater = 0;
    if(d->updater) {
        progressUpdater = new KoProgressUpdater(d->updater);
        progressUpdater->start(actions.size());
    }

    for (QList<KisRecordedAction*>::iterator it = actions.begin(); it != actions.end(); ++it) {
        if (*it) {
            dbgImage << "Play action : " << (*it)->name();
            KoUpdater* updater = 0;
            if(progressUpdater) {
                updater = progressUpdater->startSubtask();
            }
            (*it)->play(d->info, updater);
        }
        if(progressUpdater && progressUpdater->interrupted()) {
            break;
        }
    }

    if (d->info.undoAdapter()) {
        d->info.undoAdapter()->endMacro();
        if(progressUpdater && progressUpdater->interrupted()) {
            d->info.undoAdapter()->undoLastCommand();
        }
    }

}

#include "kis_macro_player.moc"
