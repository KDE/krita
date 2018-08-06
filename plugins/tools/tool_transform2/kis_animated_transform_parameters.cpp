/*
 *  Copyright (c) 2016 Jouni Pentikäinen <joupent@gmail.com>
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

#include "kis_animated_transform_parameters.h"
#include "kis_scalar_keyframe_channel.h"
#include "kis_transform_args_keyframe_channel.h"
#include "tool_transform_args.h"
#include "kis_time_range.h"
#include "kis_transform_mask.h"

struct KisAnimatedTransformMaskParameters::Private
{
    Private() :
        hidden(false),
        argsCache()
    {}

    KisTransformArgsKeyframeChannel *rawArgsChannel{0};

    KisScalarKeyframeChannel *positionXchannel{0};
    KisScalarKeyframeChannel *positionYchannel{0};
    KisScalarKeyframeChannel *scaleXchannel{0};
    KisScalarKeyframeChannel *scaleYchannel{0};
    KisScalarKeyframeChannel *shearXchannel{0};
    KisScalarKeyframeChannel *shearYchannel{0};
    KisScalarKeyframeChannel *rotationXchannel{0};
    KisScalarKeyframeChannel *rotationYchannel{0};
    KisScalarKeyframeChannel *rotationZchannel{0};

    bool hidden;
    KisTimeRange validRange;

    ToolTransformArgs argsCache;

    ToolTransformArgs &currentRawArgs()
    {
        if (!rawArgsChannel) return argsCache;

        KisKeyframeSP keyframe = rawArgsChannel->currentlyActiveKeyframe();
        if (keyframe.isNull()) return argsCache;

        return rawArgsChannel->transformArgs(keyframe);
    }

    KisScalarKeyframeChannel *getChannel(KisScalarKeyframeChannel * Private::*field, const KoID &channelId, KisDefaultBoundsBaseSP defaultBounds)
    {
        KisScalarKeyframeChannel *channel = this->*field;

        if (!channel) {
            channel = this->*field = new KisScalarKeyframeChannel(channelId, -qInf(), qInf(), defaultBounds, KisKeyframe::Linear);
        }

        return channel;
    }
};

KisAnimatedTransformMaskParameters::KisAnimatedTransformMaskParameters()
    : KisTransformMaskAdapter(ToolTransformArgs()),
      m_d(new Private())
{
}

KisAnimatedTransformMaskParameters::KisAnimatedTransformMaskParameters(const KisTransformMaskAdapter *staticTransform)
    : KisTransformMaskAdapter(staticTransform->transformArgs()),
      m_d(new Private())
{
    m_d->argsCache = staticTransform->transformArgs();
}

KisAnimatedTransformMaskParameters::~KisAnimatedTransformMaskParameters()
{}

QPointF getInterpolatedPoint(QPointF def, KisScalarKeyframeChannel *xChannel, KisScalarKeyframeChannel *yChannel)
{
    if (xChannel) {
        qreal x = xChannel->currentValue();
        if (!qIsNaN(x)) def.setX(x);
    }

    if (yChannel) {
        qreal y = yChannel->currentValue();
        if (!qIsNaN(y)) def.setY(y);
    }

    return def;
}

qreal getInterpolatedValue(KisScalarKeyframeChannel *channel, qreal defaultValue)
{
    if (!channel) return defaultValue;
    qreal value = channel->currentValue();
    if (qIsNaN(value)) return defaultValue;
    return value;
}

const ToolTransformArgs &KisAnimatedTransformMaskParameters::transformArgs() const
{
    m_d->argsCache = m_d->currentRawArgs();

    QPointF pos = getInterpolatedPoint(m_d->argsCache.transformedCenter(), m_d->positionXchannel, m_d->positionYchannel);
    m_d->argsCache.setTransformedCenter(pos);

    m_d->argsCache.setScaleX(getInterpolatedValue(m_d->scaleXchannel, m_d->argsCache.scaleX()));
    m_d->argsCache.setScaleY(getInterpolatedValue(m_d->scaleYchannel, m_d->argsCache.scaleY()));

    m_d->argsCache.setShearX(getInterpolatedValue(m_d->shearXchannel, m_d->argsCache.shearX()));
    m_d->argsCache.setShearY(getInterpolatedValue(m_d->shearYchannel, m_d->argsCache.shearY()));

    m_d->argsCache.setAX(normalizeAngle(getInterpolatedValue(m_d->rotationXchannel, m_d->argsCache.aX())));
    m_d->argsCache.setAY(normalizeAngle(getInterpolatedValue(m_d->rotationYchannel, m_d->argsCache.aY())));
    m_d->argsCache.setAZ(normalizeAngle(getInterpolatedValue(m_d->rotationZchannel, m_d->argsCache.aZ())));

    return m_d->argsCache;
}

QString KisAnimatedTransformMaskParameters::id() const
{
    return "animatedtransformparams";
}

void KisAnimatedTransformMaskParameters::toXML(QDomElement *e) const
{
    Q_UNUSED(e);
}

KisTransformMaskParamsInterfaceSP KisAnimatedTransformMaskParameters::fromXML(const QDomElement &e)
{
    Q_UNUSED(e);
    return toQShared(new KisAnimatedTransformMaskParameters());
}

KisTransformMaskParamsInterfaceSP KisAnimatedTransformMaskParameters::animate(KisTransformMaskParamsInterfaceSP params)
{
    KisTransformMaskParamsInterface *animatedParams;

    KisTransformMaskAdapter *tma = dynamic_cast<KisTransformMaskAdapter*>(params.data());
    if (tma) {
        animatedParams = new KisAnimatedTransformMaskParameters(tma);
    } else {
        animatedParams = new KisAnimatedTransformMaskParameters();
    }

    return toQShared(animatedParams);
}

void KisAnimatedTransformMaskParameters::translate(const QPointF &offset)
{
    ToolTransformArgs &args = m_d->currentRawArgs();
    args.translate(offset);
}

KisKeyframeChannel *KisAnimatedTransformMaskParameters::getKeyframeChannel(const QString &id, KisDefaultBoundsBaseSP defaultBounds)
{
    if (id == KisKeyframeChannel::TransformArguments.id()) {
        if (!m_d->rawArgsChannel) {
            m_d->rawArgsChannel = new KisTransformArgsKeyframeChannel(KisKeyframeChannel::TransformArguments, defaultBounds, m_d->currentRawArgs());
        }
        return m_d->rawArgsChannel;
    } else {
        KisScalarKeyframeChannel * Private::*field = 0;
        KoID channelId;

        if (id == KisKeyframeChannel::TransformPositionX.id()) {
            channelId = KisKeyframeChannel::TransformPositionX;
            field = &Private::positionXchannel;
        } else if (id == KisKeyframeChannel::TransformPositionY.id()) {
            channelId =  KisKeyframeChannel::TransformPositionY;
            field = &Private::positionYchannel;
        } else if (id == KisKeyframeChannel::TransformScaleX.id()) {
            channelId =  KisKeyframeChannel::TransformScaleX;
            field = &Private::scaleXchannel;
        } else if (id == KisKeyframeChannel::TransformScaleY.id()) {
            channelId =  KisKeyframeChannel::TransformScaleY;
            field = &Private::scaleYchannel;
        } else if (id == KisKeyframeChannel::TransformShearX.id()) {
            channelId =  KisKeyframeChannel::TransformShearX;
            field = &Private::shearXchannel;
        } else if (id == KisKeyframeChannel::TransformShearY.id()) {
            channelId =  KisKeyframeChannel::TransformShearY;
            field = &Private::shearYchannel;
        } else if (id == KisKeyframeChannel::TransformRotationX.id()) {
            channelId =  KisKeyframeChannel::TransformRotationX;
            field = &Private::rotationXchannel;
        } else if (id == KisKeyframeChannel::TransformRotationY.id()) {
            channelId =  KisKeyframeChannel::TransformRotationY;
            field = &Private::rotationYchannel;
        } else if (id == KisKeyframeChannel::TransformRotationZ.id()) {
            channelId =  KisKeyframeChannel::TransformRotationZ;
            field = &Private::rotationZchannel;
        }

        if (field) {
            return m_d->getChannel(field, channelId, defaultBounds);
        }
    }

    return 0;
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
    int currentTime = (m_d->rawArgsChannel) ? m_d->rawArgsChannel->currentTime() : 0;

    KisTimeRange validRange = KisTimeRange::infinite(0);

    if (m_d->rawArgsChannel) validRange &= m_d->rawArgsChannel->identicalFrames(currentTime);
    if (m_d->positionXchannel) validRange &= m_d->positionXchannel->identicalFrames(currentTime);
    if (m_d->positionYchannel) validRange &= m_d->positionYchannel->identicalFrames(currentTime);

    m_d->validRange = validRange;
}

bool KisAnimatedTransformMaskParameters::hasChanged() const
{
    int currentTime = (m_d->rawArgsChannel) ? m_d->rawArgsChannel->currentTime() : 0;
    bool valid = m_d->validRange.contains(currentTime);
    return !valid;
}

bool KisAnimatedTransformMaskParameters::isAnimated() const
{
    return true;
}

void setScalarChannelValue(KisTransformMaskSP mask, const KoID &channelId, int time, qreal value, KUndo2Command *parentCommand)
{
    KisScalarKeyframeChannel *channel = dynamic_cast<KisScalarKeyframeChannel*>(mask->getKeyframeChannel(channelId.id(), true));
    KIS_ASSERT_RECOVER_RETURN(channel);
    new KisScalarKeyframeChannel::AddKeyframeCommand(channel, time, value, parentCommand);
}

void KisAnimatedTransformMaskParameters::addKeyframes(KisTransformMaskSP mask, int time, KisTransformMaskParamsInterfaceSP params, KUndo2Command *parentCommand)
{
    KisTransformMaskParamsInterfaceSP currentParams = mask->transformParams();
    if (dynamic_cast<KisAnimatedTransformMaskParameters*>(currentParams.data()) == 0) {
        mask->setTransformParams(animate(currentParams));
    }

    if (params.isNull()) {
        params = currentParams;
    }

    ToolTransformArgs args;
    auto *adapterParams = dynamic_cast<KisTransformMaskAdapter*>(params.data());

    if (adapterParams) {
        args = adapterParams->transformArgs();
    } else {
        if (params->isHidden()) return;
        args.setOriginalCenter(mask->exactBounds().center());
        args.setTransformedCenter(args.originalCenter()); // offset?
    }

    KisTransformArgsKeyframeChannel *rawArgsChannel = dynamic_cast<KisTransformArgsKeyframeChannel*>(mask->getKeyframeChannel(KisKeyframeChannel::TransformArguments.id(), true));
    if (rawArgsChannel) {
        new KisTransformArgsKeyframeChannel::AddKeyframeCommand(rawArgsChannel, time, args, parentCommand);
    }

    setScalarChannelValue(mask, KisKeyframeChannel::TransformPositionX, time, args.transformedCenter().x(), parentCommand);
    setScalarChannelValue(mask, KisKeyframeChannel::TransformPositionY, time, args.transformedCenter().y(), parentCommand);
    setScalarChannelValue(mask, KisKeyframeChannel::TransformScaleX,    time, args.scaleX(), parentCommand);
    setScalarChannelValue(mask, KisKeyframeChannel::TransformScaleY,    time, args.scaleY(), parentCommand);
    setScalarChannelValue(mask, KisKeyframeChannel::TransformShearX,    time, args.shearX(), parentCommand);
    setScalarChannelValue(mask, KisKeyframeChannel::TransformShearY,    time, args.shearY(), parentCommand);
    setScalarChannelValue(mask, KisKeyframeChannel::TransformRotationX, time, args.aX(), parentCommand);
    setScalarChannelValue(mask, KisKeyframeChannel::TransformRotationY, time, args.aY(), parentCommand);
    setScalarChannelValue(mask, KisKeyframeChannel::TransformRotationZ, time, args.aZ(), parentCommand);
}

#include "kis_transform_mask_params_factory_registry.h"

struct AnimatedTransformParamsRegistrar {
    AnimatedTransformParamsRegistrar() {
        KisTransformMaskParamsFactory f(KisAnimatedTransformMaskParameters::fromXML);
        KisTransformMaskParamsFactoryRegistry::instance()->addFactory("animatedtransformparams", f);

        KisAnimatedTransformMaskParamsFactory a(KisAnimatedTransformMaskParameters::animate);
        KisTransformMaskParamsFactoryRegistry::instance()->setAnimatedParamsFactory(a);

        KisTransformMaskKeyframeFactory k(KisAnimatedTransformMaskParameters::addKeyframes);
        KisTransformMaskParamsFactoryRegistry::instance()->setKeyframeFactory(k);
    }
};
static AnimatedTransformParamsRegistrar __animatedTransformParamsRegistrar;
