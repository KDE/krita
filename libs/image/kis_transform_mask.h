/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
     * Create an empty transform mask.
     */
    KisTransformMask(KisImageWSP image, const QString &name);

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
    KisPaintDeviceSP buildSourcePreviewDevice();

    /**
     * Transform Tool may override mask's device for the sake of
     * in-stack preview
     */
    void overrideStaticCacheDevice(KisPaintDeviceSP device);

    qint32 x() const override;
    qint32 y() const override;

    void setX(qint32 x) override;
    void setY(qint32 y) override;

    void forceUpdateTimedNode() override;
    bool hasPendingTimedUpdates() const override;

    void threadSafeForceStaticImageUpdate();

    void syncLodCache() override;

    KisPaintDeviceList getLodCapableDevices() const override;

protected:
    KisKeyframeChannel *requestKeyframeChannel(const QString &id) override;
    bool supportsKeyframeChannel(const QString &id) override;

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
