/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
