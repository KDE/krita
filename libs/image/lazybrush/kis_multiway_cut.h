/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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
