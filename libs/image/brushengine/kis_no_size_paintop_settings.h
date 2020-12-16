/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISNOSIZEPAINTOPSETTINGS_H
#define KISNOSIZEPAINTOPSETTINGS_H

#include "kis_paintop_settings.h"
#include "kritaimage_export.h"


class KRITAIMAGE_EXPORT KisNoSizePaintOpSettings : public KisPaintOpSettings
{
public:
    KisNoSizePaintOpSettings(KisResourcesInterfaceSP resourcesInterface);

    void setPaintOpSize(qreal value) override;
    qreal paintOpSize() const override;
};

#endif // KISNOSIZEPAINTOPSETTINGS_H
