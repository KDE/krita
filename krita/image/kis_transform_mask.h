/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef _KIS_TRANSFORM_MASK_
#define _KIS_TRANSFORM_MASK_

#include <QScopedPointer>

#include "kis_types.h"
#include "kis_effect_mask.h"

/**
   Transform a layer according to a matrix transform
*/

class KRITAIMAGE_EXPORT KisTransformMask : public KisEffectMask
{
    Q_OBJECT

public:

    /**
     * Create an empty filter mask.
     */
    KisTransformMask();

    virtual ~KisTransformMask();

    QIcon icon() const;

    KisNodeSP clone() const {
        return KisNodeSP(new KisTransformMask(*this));
    }

    KisPaintDeviceSP paintDevice() const;

    bool accept(KisNodeVisitor &v);
    void accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter);

    KisTransformMask(const KisTransformMask& rhs);

    QRect decorateRect(KisPaintDeviceSP &src,
                       KisPaintDeviceSP &dst,
                       const QRect & rc,
                       PositionToFilthy maskPos) const;

    QRect changeRect(const QRect &rect, PositionToFilthy pos = N_FILTHY) const;
    QRect needRect(const QRect &rect, PositionToFilthy pos = N_FILTHY) const;

    QRect extent() const;
    QRect exactBounds() const;

    void setTransformParams(KisTransformMaskParamsInterfaceSP params);
    KisTransformMaskParamsInterfaceSP transformParams() const;

    void recaclulateStaticImage();
    KisPaintDeviceSP buildPreviewDevice();

    void setX(qint32 x);
    void setY(qint32 y);

private slots:
    void slotDelayedStaticUpdate();

signals:
    void initiateDelayedStaticUpdate() const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif //_KIS_TRANSFORM_MASK_
