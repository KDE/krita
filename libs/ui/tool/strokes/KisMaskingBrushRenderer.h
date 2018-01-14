/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KISMASKINGBRUSHRENDERER_H
#define KISMASKINGBRUSHRENDERER_H

#include "kis_types.h"

class KisMaskingBrushCompositeOpBase;


class KisMaskingBrushRenderer
{
public:
    KisMaskingBrushRenderer(KisPaintDeviceSP dstDevice, const QString &compositeOpId);
    ~KisMaskingBrushRenderer();

    KisPaintDeviceSP strokeDevice() const;
    KisPaintDeviceSP maskDevice() const;

    void updateProjection(const QRect &rc);


private:
    KisPaintDeviceSP m_strokeDevice;
    KisPaintDeviceSP m_maskDevice;
    KisPaintDeviceSP m_dstDevice;

    QScopedPointer<KisMaskingBrushCompositeOpBase> m_compositeOp;
};

#endif // KISMASKINGBRUSHRENDERER_H
