/*
 *  SPDX-FileCopyrightText: 2010 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_preference_set_registry.h"

#include <QGlobalStatic>

Q_GLOBAL_STATIC(KisPreferenceSetRegistry, s_instance)

KisPreferenceSetRegistry ::KisPreferenceSetRegistry ()
{
}

KisPreferenceSetRegistry ::~KisPreferenceSetRegistry ()
{
    qDeleteAll(values());
}

KisPreferenceSetRegistry * KisPreferenceSetRegistry ::instance()
{
    return s_instance;
}

