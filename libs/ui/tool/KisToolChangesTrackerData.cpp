/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisToolChangesTrackerData.h"

struct KisToolChangesTrackerDataSPRegistrar {
    KisToolChangesTrackerDataSPRegistrar() {
        qRegisterMetaType<KisToolChangesTrackerDataSP>("KisToolChangesTrackerDataSP");
    }
};
static KisToolChangesTrackerDataSPRegistrar __registrar;


KisToolChangesTrackerData::~KisToolChangesTrackerData()
{
}

KisToolChangesTrackerData *KisToolChangesTrackerData::clone() const
{
    return new KisToolChangesTrackerData(*this);
}
