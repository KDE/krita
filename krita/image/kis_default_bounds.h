/*
 *  Copyright (c) 2010 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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
#ifndef KIS_DEFAULT_BOUNDS_H
#define KIS_DEFAULT_BOUNDS_H

#include "kis_image.h"
#include "kis_types.h"

#include "QRect"

class KRITAIMAGE_EXPORT KisDefaultBounds
{
public:
    KisDefaultBounds();
    KisDefaultBounds(KisImageWSP image);
    KisDefaultBounds(KisPaintDeviceSP parentPaintDevice, KisImageWSP image = 0);
    KisDefaultBounds(const KisDefaultBounds &rhs);
    ~KisDefaultBounds();
    bool operator==(const KisDefaultBounds &rhs);
    KisDefaultBounds operator=(const KisDefaultBounds &rhs);
    static const QRect infiniteRect;

    QRect bounds() const;

private:
    struct Private;
    Private * const m_d;
};



#endif // KIS_DEFAULT_BOUNDS_H
