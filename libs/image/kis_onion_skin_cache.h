/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_ONION_SKIN_CACHE_H
#define __KIS_ONION_SKIN_CACHE_H

#include <QScopedPointer>
#include "kis_types.h"


class KisOnionSkinCache
{
public:
    KisOnionSkinCache();
    ~KisOnionSkinCache();

    KisPaintDeviceSP projection(KisPaintDeviceSP source);
    void reset();

    KisPaintDeviceSP lodCapableDevice() const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_ONION_SKIN_CACHE_H */
