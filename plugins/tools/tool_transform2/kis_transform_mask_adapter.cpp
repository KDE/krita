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
#include "kis_animated_transform_parameters.h"

#include "kis_node.h"


struct KisTransformMaskAdapter::Private
{
    QSharedPointer<ToolTransformArgs> args;
};


KisTransformMaskAdapter::KisTransformMaskAdapter()
    : m_d(new Private)
{
    m_d->args.reset(new ToolTransformArgs());
}

KisTransformMaskAdapter::KisTransformMaskAdapter(const ToolTransformArgs &args)
    : m_d(new Private)
{
    m_d->args = toQShared(new ToolTransformArgs(args));
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

bool KisTransformMaskAdapter::isHidden() const
{
    return false;
}

void KisTransformMaskAdapter::transformDevice(KisNodeSP node, KisPaintDeviceSP src, KisPaintDeviceSP dst) const
{
    dst->prepareClone(src);

    KisProcessingVisitor::ProgressHelper helper(node);

    KisTransformUtils::transformDevice(*transformArgs(), src, dst, &helper);
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

void KisTransformMaskAdapter::translate(const QPointF &offset)
{
    m_d->args->translate(offset);
}

void KisTransformMaskAdapter::transformSrcAndDst(const QTransform &t)
{
    m_d->args->transformSrcAndDst(t);
}

QRect KisTransformMaskAdapter::nonAffineChangeRect(const QRect &rc)
{
    return KisTransformUtils::changeRect(*transformArgs(), rc);
}

QRect KisTransformMaskAdapter::nonAffineNeedRect(const QRect &rc, const QRect &srcBounds)
{
    return KisTransformUtils::needRect(*transformArgs(), rc, srcBounds);
}

bool KisTransformMaskAdapter::isAnimated() const
{
    return false;
}

KisKeyframeChannel *KisTransformMaskAdapter::getKeyframeChannel(const QString &id, KisDefaultBoundsBaseSP defaultBounds)
{
    Q_UNUSED(id);
    Q_UNUSED(defaultBounds);
    return 0;
}

void KisTransformMaskAdapter::clearChangedFlag()
{}

bool KisTransformMaskAdapter::hasChanged() const
{
    return false;
}

KisTransformMaskParamsInterfaceSP KisTransformMaskAdapter::clone() const {
    return toQShared(new KisTransformMaskAdapter(*this->transformArgs()));
}

#include "kis_transform_mask_params_factory_registry.h"

struct ToolTransformParamsRegistrar {
    ToolTransformParamsRegistrar() {
        KisTransformMaskParamsFactory f(KisTransformMaskAdapter::fromXML);
        KisTransformMaskParamsFactoryRegistry::instance()->addFactory("tooltransformparams", f);
    }
};
static ToolTransformParamsRegistrar __toolTransformParamsRegistrar;

