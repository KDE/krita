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

struct KisAnimatedTransformMaskParameters::Private
{
    Private() :
        rawArgsChannel(0),
        positionXchannel(0),
        positionYchannel(0),
        hidden(false),
        argsCache()
    {}

    KisTransformArgsKeyframeChannel *rawArgsChannel;

    KisScalarKeyframeChannel *positionXchannel;
    KisScalarKeyframeChannel *positionYchannel;

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

const ToolTransformArgs &KisAnimatedTransformMaskParameters::transformArgs() const
{
    m_d->argsCache = m_d->currentRawArgs();

    QPointF pos = getInterpolatedPoint(m_d->argsCache.transformedCenter(), m_d->positionXchannel, m_d->positionYchannel);
    m_d->argsCache.setTransformedCenter(pos);

    return m_d->argsCache;
}

QString KisAnimatedTransformMaskParameters::id() const
{
    return "animatedtransformparams";
}

void KisAnimatedTransformMaskParameters::toXML(QDomElement *e) const
{
}

KisTransformMaskParamsInterfaceSP KisAnimatedTransformMaskParameters::fromXML(const QDomElement &e)
{
    return toQShared(new KisAnimatedTransformMaskParameters());
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
            m_d->rawArgsChannel = new KisTransformArgsKeyframeChannel(KisKeyframeChannel::TransformArguments, defaultBounds);
        }
        return m_d->rawArgsChannel;
    }

    if (id == KisKeyframeChannel::TransformPositionX.id()) {
        if (!m_d->positionXchannel) {
            m_d->positionXchannel = new KisScalarKeyframeChannel(KisKeyframeChannel::TransformPositionX, -qInf(), qInf(), defaultBounds, KisKeyframe::Linear);
        }
        return m_d->positionXchannel;
    }

    if (id == KisKeyframeChannel::TransformPositionY.id()) {
        if (!m_d->positionYchannel) {
            m_d->positionYchannel = new KisScalarKeyframeChannel(KisKeyframeChannel::TransformPositionY, -qInf(), qInf(), defaultBounds, KisKeyframe::Linear);
        }
        return m_d->positionYchannel;
    }

    return 0;
}

KisTransformMaskParamsInterfaceSP KisAnimatedTransformMaskParameters::enableAnimation()
{
    return KisTransformMaskParamsInterfaceSP();
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

#include "kis_transform_mask_params_factory_registry.h"

struct AnimatedTransformParamsRegistrar {
    AnimatedTransformParamsRegistrar() {
        KisTransformMaskParamsFactory f(KisAnimatedTransformMaskParameters::fromXML);
        KisTransformMaskParamsFactoryRegistry::instance()->addFactory("animatedtransformparams", f);
    }
};
static AnimatedTransformParamsRegistrar __animatedTransformParamsRegistrar;
