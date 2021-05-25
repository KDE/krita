/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_TRANSFORM_MASK_PARAMS_INTERFACE_H
#define __KIS_TRANSFORM_MASK_PARAMS_INTERFACE_H

#include "kritaimage_export.h"
#include "kis_types.h"
#include "kis_default_bounds_base.h"

#include <QScopedPointer>


class QTransform;
class QDomElement;
class KisKeyframeChannel;

class KRITAIMAGE_EXPORT KisTransformMaskParamsInterface
{
public:
    virtual ~KisTransformMaskParamsInterface();

    virtual QTransform finalAffineTransform() const = 0;
    virtual bool isAffine() const = 0;
    virtual bool isHidden() const = 0;

    virtual void transformDevice(KisNodeSP node, KisPaintDeviceSP src, KisPaintDeviceSP dst) const = 0;

    virtual QString id() const = 0;
    virtual void toXML(QDomElement *e) const = 0;

    virtual void translate(const QPointF &offset) = 0;
    virtual void transformSrcAndDst(const QTransform &t) = 0;

    virtual QRect nonAffineChangeRect(const QRect &rc) = 0;
    virtual QRect nonAffineNeedRect(const QRect &rc, const QRect &srcBounds) = 0;

    virtual void clearChangedFlag() = 0;
    virtual bool hasChanged() const = 0;

    virtual KisTransformMaskParamsInterfaceSP clone() const = 0;
};

class KRITAIMAGE_EXPORT KisAnimatedTransformParamsInterface
{
public:
    virtual ~KisAnimatedTransformParamsInterface();

    virtual KisKeyframeChannel* requestKeyframeChannel(const QString &id, KisNodeWSP parent) = 0;
    virtual KisKeyframeChannel* getKeyframeChannel(const KoID& koid) const = 0;
    virtual void setKeyframeChannel(const QString &name, QSharedPointer<KisKeyframeChannel> kcsp) = 0;
    virtual QList<KisKeyframeChannel*> copyChannelsFrom(const KisAnimatedTransformParamsInterface* other) = 0;
};

class QDomElement;

class KRITAIMAGE_EXPORT KisDumbTransformMaskParams : public KisTransformMaskParamsInterface
{
public:
    KisDumbTransformMaskParams();
    KisDumbTransformMaskParams(const QTransform &transform);
    KisDumbTransformMaskParams(bool isHidden);
    ~KisDumbTransformMaskParams() override;


    QTransform finalAffineTransform() const override;
    bool isAffine() const override;
    bool isHidden() const override;
    void transformDevice(KisNodeSP node, KisPaintDeviceSP src, KisPaintDeviceSP dst) const override;

    QString id() const override;
    void toXML(QDomElement *e) const override;
    static KisTransformMaskParamsInterfaceSP fromXML(const QDomElement &e);

    void translate(const QPointF &offset) override;
    void transformSrcAndDst(const QTransform &t) override;

    // for tesing purposes only
    QTransform testingGetTransform() const;
    void testingSetTransform(const QTransform &t);

    QRect nonAffineChangeRect(const QRect &rc) override;
    QRect nonAffineNeedRect(const QRect &rc, const QRect &srcBounds) override;

    bool isAnimated() const;
    KisKeyframeChannel *getKeyframeChannel(const QString &id, KisDefaultBoundsBaseSP defaultBounds);
    void clearChangedFlag() override;
    bool hasChanged() const override;

    KisTransformMaskParamsInterfaceSP clone() const override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_TRANSFORM_MASK_PARAMS_INTERFACE_H */
