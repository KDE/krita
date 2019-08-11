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
#include "KisDelayedUpdateNodeInterface.h"

/**
   Transform a layer according to a matrix transform
*/

class KRITAIMAGE_EXPORT KisTransformMask : public KisEffectMask, public KisDelayedUpdateNodeInterface
{
    Q_OBJECT

public:

    /**
     * Create an empty filter mask.
     */
    KisTransformMask();

    ~KisTransformMask() override;

    QIcon icon() const override;

    KisNodeSP clone() const override {
        return KisNodeSP(new KisTransformMask(*this));
    }

    KisPaintDeviceSP paintDevice() const override;

    bool accept(KisNodeVisitor &v) override;
    void accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter) override;

    KisTransformMask(const KisTransformMask& rhs);

    QRect decorateRect(KisPaintDeviceSP &src,
                       KisPaintDeviceSP &dst,
                       const QRect & rc,
                       PositionToFilthy maskPos) const override;

    QRect changeRect(const QRect &rect, PositionToFilthy pos = N_FILTHY) const override;
    QRect needRect(const QRect &rect, PositionToFilthy pos = N_FILTHY) const override;

    QRect extent() const override;
    QRect exactBounds() const override;
    QRect sourceDataBounds() const;

    void setTransformParams(KisTransformMaskParamsInterfaceSP params);
    KisTransformMaskParamsInterfaceSP transformParams() const;

    void recaclulateStaticImage();
    KisPaintDeviceSP buildPreviewDevice();

    void setX(qint32 x) override;
    void setY(qint32 y) override;

    void forceUpdateTimedNode() override;
    bool hasPendingTimedUpdates() const override;

    void threadSafeForceStaticImageUpdate();

protected:
    KisKeyframeChannel *requestKeyframeChannel(const QString &id) override;

Q_SIGNALS:
    void sigInternalForceStaticImageUpdate();

private Q_SLOTS:
    void slotDelayedStaticUpdate();

    void slotInternalForceStaticImageUpdate();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif //_KIS_TRANSFORM_MASK_
