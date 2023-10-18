/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISDUMBTRANSFORMMASKPARAMS_H
#define KISDUMBTRANSFORMMASKPARAMS_H

#include "kritatransformmaskstubs_export.h"
#include "kis_transform_mask_params_interface.h"


class QDomElement;

class KRITATRANSFORMMASKSTUBS_EXPORT KisDumbTransformMaskParams : public KisTransformMaskParamsInterface
{
public:
    KisDumbTransformMaskParams();
    KisDumbTransformMaskParams(const QTransform &transform);
    KisDumbTransformMaskParams(bool isHidden);
    ~KisDumbTransformMaskParams() override;


    QTransform finalAffineTransform() const override;
    bool isAffine() const override;
    bool isHidden() const override;
    void setHidden(bool value) override;
    void transformDevice(KisNodeSP node, KisPaintDeviceSP src, KisPaintDeviceSP dst, bool forceSubPixelTranslation) const override;

    QString id() const override;
    void toXML(QDomElement *e) const override;
    static KisTransformMaskParamsInterfaceSP fromXML(const QDomElement &e);

    void translateSrcAndDst(const QPointF &offset) override;
    void transformSrcAndDst(const QTransform &t) override;
    void translateDstSpace(const QPointF &offset) override;

    // for testing purposes only
    QTransform testingGetTransform() const;
    void testingSetTransform(const QTransform &t);

    QRect nonAffineChangeRect(const QRect &rc) override;
    QRect nonAffineNeedRect(const QRect &rc, const QRect &srcBounds) override;

    bool isAnimated() const;
    KisKeyframeChannel *getKeyframeChannel(const QString &id, KisDefaultBoundsBaseSP defaultBounds);

    KisTransformMaskParamsInterfaceSP clone() const override;

    bool compareTransform(KisTransformMaskParamsInterfaceSP rhs) const override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISDUMBTRANSFORMMASKPARAMS_H
