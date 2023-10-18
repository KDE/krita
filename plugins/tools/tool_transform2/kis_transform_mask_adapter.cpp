/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_transform_mask_adapter.h"

#include <QTransform>
#include <QDomElement>

#include "tool_transform_args.h"
#include "kis_transform_utils.h"
#include "KisAnimatedTransformMaskParamsHolder.h"

#include "kis_node.h"


struct KisTransformMaskAdapter::Private
{
    QSharedPointer<ToolTransformArgs> args;
    bool isHidden {false};
    bool isInitialized {true};
};


KisTransformMaskAdapter::KisTransformMaskAdapter()
    : m_d(new Private)
{
    m_d->args.reset(new ToolTransformArgs());
}

KisTransformMaskAdapter::KisTransformMaskAdapter(const ToolTransformArgs &args, bool isHidden, bool isInitialized)
    : m_d(new Private)
{
    m_d->args = toQShared(new ToolTransformArgs(args));
    m_d->isHidden = isHidden;
    m_d->isInitialized = isInitialized;
}

KisTransformMaskAdapter::~KisTransformMaskAdapter()
{
}

QTransform KisTransformMaskAdapter::finalAffineTransform() const
{
    KisTransformUtils::MatricesPack m(*transformArgs());
    return m.finalTransform();
}

bool KisTransformMaskAdapter::isAffine() const
{
    const ToolTransformArgs args = *transformArgs();

    return args.mode() == ToolTransformArgs::FREE_TRANSFORM ||
            args.mode() == ToolTransformArgs::PERSPECTIVE_4POINT;
}

bool KisTransformMaskAdapter::isInitialized() const
{
    return m_d->isInitialized;
}

void KisTransformMaskAdapter::setHidden(bool value)
{
    m_d->isHidden = value;
}

bool KisTransformMaskAdapter::isHidden() const
{
    return m_d->isHidden;
}

void KisTransformMaskAdapter::transformDevice(KisNodeSP node, KisPaintDeviceSP src, KisPaintDeviceSP dst, bool forceSubPixelTranslation) const
{
    dst->prepareClone(src);

    KisProcessingVisitor::ProgressHelper helper(node);

    KisTransformUtils::transformDeviceWithCroppedDst(*transformArgs(), src, dst, &helper, forceSubPixelTranslation);
}

const QSharedPointer<ToolTransformArgs> KisTransformMaskAdapter::transformArgs() const {
    return m_d->args;
}

void KisTransformMaskAdapter::setBaseArgs(const ToolTransformArgs &args)
{
    *m_d->args = args;
}

QString KisTransformMaskAdapter::id() const
{
    return "tooltransformparams";
}

void KisTransformMaskAdapter::toXML(QDomElement *e) const
{
    m_d->args->toXML(e);
}

KisTransformMaskParamsInterfaceSP KisTransformMaskAdapter::fromXML(const QDomElement &e)
{
    return KisTransformMaskParamsInterfaceSP(
        new KisTransformMaskAdapter(ToolTransformArgs::fromXML(e)));
}

void KisTransformMaskAdapter::translateSrcAndDst(const QPointF &offset)
{
    m_d->args->translateSrcAndDst(offset);
}

void KisTransformMaskAdapter::transformSrcAndDst(const QTransform &t)
{
    m_d->args->transformSrcAndDst(t);
}

void KisTransformMaskAdapter::translateDstSpace(const QPointF &offset)
{
    m_d->args->translateDstSpace(offset);
}

QRect KisTransformMaskAdapter::nonAffineChangeRect(const QRect &rc)
{
    return KisTransformUtils::changeRect(*transformArgs(), rc);
}

QRect KisTransformMaskAdapter::nonAffineNeedRect(const QRect &rc, const QRect &srcBounds)
{
    return KisTransformUtils::needRect(*transformArgs(), rc, srcBounds);
}

KisKeyframeChannel *KisTransformMaskAdapter::getKeyframeChannel(const QString &id, KisDefaultBoundsBaseSP defaultBounds)
{
    Q_UNUSED(id);
    Q_UNUSED(defaultBounds);
    return 0;
}

KisTransformMaskParamsInterfaceSP KisTransformMaskAdapter::clone() const {
    return toQShared(new KisTransformMaskAdapter(*this->transformArgs(), this->isHidden(), this->isInitialized()));
}

bool KisTransformMaskAdapter::compareTransform(KisTransformMaskParamsInterfaceSP rhs) const
{
    QSharedPointer<KisTransformMaskAdapter> adapter = rhs.dynamicCast<KisTransformMaskAdapter>();
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(adapter, false);

    QSharedPointer<ToolTransformArgs> lhsArgs = transformArgs();
    QSharedPointer<ToolTransformArgs> rhsArgs = adapter->transformArgs();

    return lhsArgs->isSameMode(*rhsArgs);
}
