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

#include <QObject>
#include <QRect>

class KRITAIMAGE_EXPORT KisDefaultBounds : public QObject
{

    Q_OBJECT

public:
    KisDefaultBounds();


    KisDefaultBounds(KisPaintDeviceSP parentPaintDevice);
    KisDefaultBounds(KisImageWSP image);
    KisDefaultBounds(KisImageWSP image, KisPaintDeviceSP parentPaintDevice);

    ~KisDefaultBounds();

    static const QRect infiniteRect;

    virtual QRect bounds() const;

private:

    Q_DISABLE_COPY(KisDefaultBounds);

    struct Private;
    Private * const m_d;
};


#endif // KIS_DEFAULT_BOUNDS_H
