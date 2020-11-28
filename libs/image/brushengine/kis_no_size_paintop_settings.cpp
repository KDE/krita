/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_no_size_paintop_settings.h"

KisNoSizePaintOpSettings::KisNoSizePaintOpSettings(KisResourcesInterfaceSP resourcesInterface)
    : KisPaintOpSettings(resourcesInterface)
{
}

void KisNoSizePaintOpSettings::setPaintOpSize(qreal value)
{
    Q_UNUSED(value);
}

qreal KisNoSizePaintOpSettings::paintOpSize() const
{
    return 1.0;
}

