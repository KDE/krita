/*
 *  SPDX-FileCopyrightText: 2016 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_animated_transform_parameters.h"
#include "kis_scalar_keyframe_channel.h"
#include "kis_transform_args_keyframe_channel.h"
#include "tool_transform_args.h"
#include "kis_time_span.h"
#include "kis_transform_mask.h"
#include <QHash>

struct KisAnimatedTransformMaskParameters::Private
{
    Private() :
        hidden(false)
    {
        transformChannels.insert(KisKeyframeChannel::PositionX.name(), nullptr);
        transformChannels.insert(KisKeyframeChannel::PositionY.name(), nullptr);
        transformChannels.insert(KisKeyframeChannel::ScaleX.name(), nullptr);
        transformChannels.insert(KisKeyframeChannel::ScaleY.name(), nullptr);
        transformChannels.insert(KisKeyframeChannel::ShearX.name(), nullptr);
        transformChannels.insert(KisKeyframeChannel::ShearY.name(), nullptr);
        transformChannels.insert(KisKeyframeChannel::RotationX.name(), nullptr);
        transformChannels.insert(KisKeyframeChannel::RotationY.name(), nullptr);
        transformChannels.insert(KisKeyframeChannel::RotationZ.name(), nullptr);
    }

    QHash<QString, QSharedPointer<KisScalarKeyframeChannel>> transformChannels;

    bool hidden;
    quint64 hash;
//    KisTimeSpan identicalRange;

//    void clearChangedFlag() {
//        KisTimeSpan validRange = KisTimeSpan::infinite(0);

//        for (QHash<QString, QSharedPointer<KisScalarKeyframeChannel>>::const_iterator e = transformChannels.constBegin();
//             e; e++) {
//            if (e.value().isNull()) continue;

//            validRange &= e.value()->identicalFrames(e.value()->currentTime());
//        }

//        identicalRange = validRange;
//    }
};

KisAnimatedTransformMaskParameters::KisAnimatedTransformMaskParameters()
    : KisTransformMaskAdapter(ToolTransformArgs()),
      m_d(new Private())
{
}

KisAnimatedTransformMaskParameters::KisAnimatedTransformMaskParameters(const KisTransformMaskAdapter *staticTransform)
    : KisTransformMaskAdapter(*staticTransform->transformArgs()),
      m_d(new Private())
{
}

KisAnimatedTransformMaskParameters::~KisAnimatedTransformMaskParameters()
{
    ENTER_FUNCTION();
}

const QSharedPointer<ToolTransformArgs> KisAnimatedTransformMaskParameters::transformArgs() const
{
    QSharedPointer<ToolTransformArgs> args = KisTransformMaskAdapter::transformArgs();

    if (m_d->transformChannels[KisKeyframeChannel::PositionX.name()] || m_d->transformChannels[KisKeyframeChannel::PositionY.name()]) {
        qreal xposition = m_d->transformChannels[KisKeyframeChannel::PositionX.name()] ? m_d->transformChannels[KisKeyframeChannel::PositionX.name()]->currentValue() : args->transformedCenter().x();
        qreal yposition = m_d->transformChannels[KisKeyframeChannel::PositionY.name()] ? m_d->transformChannels[KisKeyframeChannel::PositionY.name()]->currentValue() : args->transformedCenter().y();
        args->setTransformedCenter(QPointF(xposition, yposition));
    }

    qreal xScale = m_d->transformChannels[KisKeyframeChannel::ScaleX.name()] ? m_d->transformChannels[KisKeyframeChannel::ScaleX.name()]->currentValue() : args->scaleX();
    qreal yScale = m_d->transformChannels[KisKeyframeChannel::ScaleY.name()] ? m_d->transformChannels[KisKeyframeChannel::ScaleY.name()]->currentValue() : args->scaleY();
    args->setScaleX(xScale);
    args->setScaleY(yScale);

    qreal xShear = m_d->transformChannels[KisKeyframeChannel::ShearX.name()] ? m_d->transformChannels[KisKeyframeChannel::ShearX.name()]->currentValue() : args->shearX();
    qreal yShear = m_d->transformChannels[KisKeyframeChannel::ShearY.name()] ? m_d->transformChannels[KisKeyframeChannel::ShearY.name()]->currentValue() : args->shearY();
    args->setShearX(xShear);
    args->setShearY(yShear);

    qreal xRot = m_d->transformChannels[KisKeyframeChannel::RotationX.name()] ? m_d->transformChannels[KisKeyframeChannel::RotationX.name()]->currentValue() : args->aX();
    qreal yRot = m_d->transformChannels[KisKeyframeChannel::RotationY.name()] ? m_d->transformChannels[KisKeyframeChannel::RotationY.name()]->currentValue() : args->aY();
    qreal zRot = m_d->transformChannels[KisKeyframeChannel::RotationZ.name()] ? m_d->transformChannels[KisKeyframeChannel::RotationZ.name()]->currentValue() : args->aZ();
    args->setAX(xRot);
    args->setAY(yRot);
    args->setAZ(zRot);

    return args;
}

QString KisAnimatedTransformMaskParameters::id() const
{
    return "animatedtransformparams";
}

void KisAnimatedTransformMaskParameters::toXML(QDomElement *e) const
{
    Q_UNUSED(e);
}

void KisAnimatedTransformMaskParameters::translate(const QPointF &offset)
{
    QSharedPointer<ToolTransformArgs> args = transformArgs();
    args->translate(offset);
}

KisKeyframeChannel *KisAnimatedTransformMaskParameters::getKeyframeChannel(const QString &id, KisNodeSP parent)
{
    KoID channelId;

    ENTER_FUNCTION() << id;


    //TODO -- clean this up.
    if (id == KisKeyframeChannel::PositionX.id()) {
        channelId = KisKeyframeChannel::PositionX;
    } else if (id == KisKeyframeChannel::PositionY.id()) {
        channelId = KisKeyframeChannel::PositionY;
    } else if (id == KisKeyframeChannel::ScaleX.id()) {
        channelId = KisKeyframeChannel::ScaleX;
    } else if (id == KisKeyframeChannel::ScaleY.id()) {
        channelId = KisKeyframeChannel::ScaleY;
    } else if (id == KisKeyframeChannel::ShearX.id()) {
        channelId = KisKeyframeChannel::ShearX;
    } else if (id == KisKeyframeChannel::ShearY.id()) {
        channelId = KisKeyframeChannel::ShearY;
    } else if (id == KisKeyframeChannel::RotationX.id()) {
        channelId = KisKeyframeChannel::RotationX;
    } else if (id == KisKeyframeChannel::RotationY.id()) {
        channelId = KisKeyframeChannel::RotationY;
    } else if (id == KisKeyframeChannel::RotationZ.id()) {
        channelId = KisKeyframeChannel::RotationZ;
    } else {
        return nullptr;
    }


    if (!m_d->transformChannels[channelId.name()]) {
        m_d->transformChannels[channelId.name()].reset(new KisScalarKeyframeChannel(channelId, new KisDefaultBoundsNodeWrapper(parent)));
        m_d->transformChannels[channelId.name()]->setDefaultValue(0);
        m_d->transformChannels[channelId.name()]->setDefaultInterpolationMode(KisScalarKeyframe::Linear);
        m_d->transformChannels[channelId.name()]->connect(m_d->transformChannels[channelId.name()].data(), &KisKeyframeChannel::sigChannelUpdated, [this](const KisTimeSpan&, const QRect&){
            this->clearChangedFlag();
        });
    }

    return m_d->transformChannels[channelId.name()].data();
}

bool KisAnimatedTransformMaskParameters::isHidden() const
{
    return m_d->hidden;
}

void KisAnimatedTransformMaskParameters::setHidden(bool hidden)
{
    m_d->hidden = hidden;
}

void KisAnimatedTransformMaskParameters::clearChangedFlag()
{
    m_d->hash = reinterpret_cast<quint64>(transformArgs().data());
}

bool KisAnimatedTransformMaskParameters::hasChanged() const
{
    return m_d->hash == reinterpret_cast<quint64>(transformArgs().data());
}

bool KisAnimatedTransformMaskParameters::isAnimated() const
{
    return true;
}

KisTransformMaskParamsInterfaceSP KisAnimatedTransformMaskParameters::fromXML(const QDomElement &e)
{
    Q_UNUSED(e);
    return toQShared(new KisAnimatedTransformMaskParameters());
}

KisTransformMaskParamsInterfaceSP KisAnimatedTransformMaskParameters::makeAnimated(KisTransformMaskParamsInterfaceSP params)
{
    KisTransformMaskParamsInterface *animatedParams;

    KisTransformMaskAdapter *tma = dynamic_cast<KisTransformMaskAdapter*>(params.data());
    if (tma) {
        animatedParams = new KisAnimatedTransformMaskParameters(tma);
    } else {
        animatedParams = new KisAnimatedTransformMaskParameters();
    }

    animatedParams->clearChangedFlag();

    return toQShared(animatedParams);
}

void KisAnimatedTransformMaskParameters::makeScalarKeyframeOnMask(KisTransformMaskSP mask, const KoID &channelId, int time, qreal value, KUndo2Command *parentCommand)
{
    KisScalarKeyframeChannel *channel = dynamic_cast<KisScalarKeyframeChannel*>(mask->getKeyframeChannel(channelId.id(), true));
    if (channel){
        channel->addScalarKeyframe(time, value, parentCommand);
    }
}


void KisAnimatedTransformMaskParameters::addKeyframes(KisTransformMaskSP mask, int time, KisTransformMaskParamsInterfaceSP params, KUndo2Command *parentCommand)
{
    KisTransformMaskParamsInterfaceSP currentParams = mask->transformParams();
    if (dynamic_cast<KisAnimatedTransformMaskParameters*>(currentParams.data()) == 0) {
        mask->setTransformParams(makeAnimated(currentParams));
    }

    if (params.isNull()) {
        params = currentParams;
    }

    ToolTransformArgs args;
    auto *adapterParams = dynamic_cast<KisTransformMaskAdapter*>(params.data());

    if (adapterParams) {
        args = *adapterParams->transformArgs();
    } else {
        if (params->isHidden()) return;
        args.setOriginalCenter(mask->exactBounds().center());
        args.setTransformedCenter(args.originalCenter()); // offset?
    }

    KisTransformMaskAdapter *tma = dynamic_cast<KisTransformMaskAdapter*>(mask->transformParams().data());
    if (tma) {
        tma->setBaseArgs(args);
    }


    makeScalarKeyframeOnMask(mask, KisKeyframeChannel::PositionX, time, args.transformedCenter().x(), parentCommand);
    makeScalarKeyframeOnMask(mask, KisKeyframeChannel::PositionY, time, args.transformedCenter().y(), parentCommand);

    makeScalarKeyframeOnMask(mask, KisKeyframeChannel::ScaleX, time, args.scaleX(), parentCommand);
    makeScalarKeyframeOnMask(mask, KisKeyframeChannel::ScaleY, time, args.scaleY(), parentCommand);

    makeScalarKeyframeOnMask(mask, KisKeyframeChannel::ShearX, time, args.shearX(), parentCommand);
    makeScalarKeyframeOnMask(mask, KisKeyframeChannel::ShearY, time, args.shearY(), parentCommand);

    makeScalarKeyframeOnMask(mask, KisKeyframeChannel::RotationX, time, args.aX(), parentCommand);
    makeScalarKeyframeOnMask(mask, KisKeyframeChannel::RotationY, time, args.aY(), parentCommand);
    makeScalarKeyframeOnMask(mask, KisKeyframeChannel::RotationZ, time, args.aZ(), parentCommand);


}

#include "kis_transform_mask_params_factory_registry.h"

struct AnimatedTransformParamsRegistrar {
    AnimatedTransformParamsRegistrar() {
        KisTransformMaskParamsFactory f(KisAnimatedTransformMaskParameters::fromXML);
        KisTransformMaskParamsFactoryRegistry::instance()->addFactory("animatedtransformparams", f);

        KisAnimatedTransformMaskParamsFactory a(KisAnimatedTransformMaskParameters::makeAnimated);
        KisTransformMaskParamsFactoryRegistry::instance()->setAnimatedParamsFactory(a);

        KisTransformMaskKeyframeFactory k(KisAnimatedTransformMaskParameters::addKeyframes);
        KisTransformMaskParamsFactoryRegistry::instance()->setKeyframeFactory(k);
    }
};
static AnimatedTransformParamsRegistrar __animatedTransformParamsRegistrar;
