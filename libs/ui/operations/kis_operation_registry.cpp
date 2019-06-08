/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_operation_registry.h"

#include <QGlobalStatic>
#include "actions/kis_selection_action_factories.h"
#include "actions/KisPasteActionFactories.h"

Q_GLOBAL_STATIC(KisOperationRegistry, s_instance)

KisOperationRegistry* KisOperationRegistry::instance()
{
    return s_instance;
}


KisOperationRegistry::KisOperationRegistry()
{
    add(new KisSelectAllActionFactory);
    add(new KisDeselectActionFactory);
    add(new KisReselectActionFactory);
    add(new KisFillActionFactory);
    add(new KisClearActionFactory);
    add(new KisImageResizeToSelectionActionFactory);
    add(new KisCutCopyActionFactory);
    add(new KisCopyMergedActionFactory);
    add(new KisPasteActionFactory);
    add(new KisPasteNewActionFactory);
}

KisOperationRegistry::~KisOperationRegistry()
{
    Q_FOREACH (const QString &id, keys()) {
        delete get(id);
    }
}
