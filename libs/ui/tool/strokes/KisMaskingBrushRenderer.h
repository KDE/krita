/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
