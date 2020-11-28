/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_MULTIWAY_CUT_H
#define __KIS_MULTIWAY_CUT_H

#include <QScopedPointer>

#include "kis_types.h"
#include "kritaimage_export.h"

class KoColor;

class KRITAIMAGE_EXPORT KisMultiwayCut
{
public:
    KisMultiwayCut(KisPaintDeviceSP src,
                   KisPaintDeviceSP dst,
                   const QRect &boundingRect);
    ~KisMultiwayCut();

    void addKeyStroke(KisPaintDeviceSP dev, const KoColor &color);

    void run();

    KisPaintDeviceSP srcDevice() const;
    KisPaintDeviceSP dstDevice() const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_MULTIWAY_CUT_H */
