/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_paintop_lod_limitations.h"

struct KisPaintopLodLimitationsStaticRegistrar {
    KisPaintopLodLimitationsStaticRegistrar() {
        qRegisterMetaType<KisPaintopLodLimitations>("KisPaintopLodLimitations");
    }
};
static KisPaintopLodLimitationsStaticRegistrar __registrar;
