/*
 *  SPDX-FileCopyrightText: 2016 Jouni Pentikäinen <joupent@gmail.com>
 *  SPDX-FileCopyrightText: 2021 Eoin O'Neill<eoinoneill1991@gmail.com>
 *  SPDX-FileCopyrightText: 2021 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_animated_transform_parameters.h"
#include "kis_scalar_keyframe_channel.h"
#include "tool_transform_args.h"
#include "kis_time_span.h"
#include "kis_transform_mask.h"
#include "kis_image.h"
#include "kis_transform_utils.h"
#include <QHash>

#include <kis_lod_transform.h>
#include <kis_lod_capable_layer_offset.h>
#include "KisChangeValueCommand.h"

namespace KisLodSwitchingWrapperDetail
{
template<>
ToolTransformArgs syncLodNValue<ToolTransformArgs>(const ToolTransformArgs &value, int lod) {
    ToolTransformArgs args(value);
    args.scale3dSrcAndDst(KisLodTransform::lodToScale(lod));
    return args;
}
} // namespace KisLodSwitchingWrapperDetail

using KisLogCapableTransformArgs = KisLodSwitchingWrapper<ToolTransformArgs>;

struct KisAnimatedTransformMaskParamsHolder::Private
{
    using TransformChannels = QHash<QString, QSharedPointer<KisScalarKeyframeChannel>>;

    Private(KisDefaultBoundsBaseSP _defaultBounds)
        : baseArgs(_defaultBounds)
        , defaultBounds(_defaultBounds)
        , hash()
    {
    }

    Private(Private& rhs)
        : baseArgs(rhs.baseArgs)
        , defaultBounds(rhs.defaultBounds)
        , isHidden(rhs.isHidden)
        , hash(rhs.hash)
    {

        Q_FOREACH(QString otherKey, rhs.transformChannels.keys()) {
            if (rhs.transformChannels[otherKey]){
                transformChannels.insert(otherKey, toQShared(new KisScalarKeyframeChannel(*rhs.transformChannels[otherKey])));
            }
        }

    }

    TransformChannels transformChannels;
    KisLogCapableTransformArgs baseArgs;
    KisDefaultBoundsBaseSP defaultBounds;
    bool isHidden {false};
    bool isInitialized {false};

    quint64 hash;
};

KisAnimatedTransformMaskParamsHolder::KisAnimatedTransformMaskParamsHolder(KisDefaultBoundsBaseSP defaultBounds)
    : m_d(new Private(defaultBounds))
{
}

KisAnimatedTransformMaskParamsHolder::KisAnimatedTransformMaskParamsHolder(const KisAnimatedTransformMaskParamsHolder &rhs)
    : m_d(new Private(*rhs.m_d))
{
}

KisAnimatedTransformMaskParamsHolder::~KisAnimatedTransformMaskParamsHolder()
{
}

const QSharedPointer<ToolTransformArgs> KisAnimatedTransformMaskParamsHolder::transformArgs() const
{
    QSharedPointer<ToolTransformArgs> args(new ToolTransformArgs(*m_d->baseArgs));

    if (m_d->transformChannels.isEmpty()) return args;
    if (m_d->defaultBounds->currentLevelOfDetail() > 0) return args;


    auto channelFor = [this] (const QString &id) -> KisScalarKeyframeChannel* {
        KisScalarKeyframeChannel *channel = this->m_d->transformChannels.value(id, nullptr).data();

        if (channel && channel->keyframeCount() > 0) {
            return channel;
        }

        return  nullptr;
    };

    {
        // Position

        KisScalarKeyframeChannel *posXChannel = channelFor(KisKeyframeChannel::PositionX.id());
        KisScalarKeyframeChannel *posYChannel = channelFor(KisKeyframeChannel::PositionY.id());

        if (posXChannel || posYChannel) {
            qreal xPosition = posXChannel ? posXChannel->currentValue() : args->transformedCenter().x();
            qreal yPosition = posYChannel ? posYChannel->currentValue() : args->transformedCenter().y();

            args->setTransformedCenter(QPointF(xPosition, yPosition));
        }
    }

    {
        // Scale

        KisScalarKeyframeChannel *scaleXChannel = channelFor(KisKeyframeChannel::ScaleX.id());
        KisScalarKeyframeChannel *scaleYChannel = channelFor(KisKeyframeChannel::ScaleY.id());

        if (scaleXChannel || scaleYChannel) {
            qreal xScale = scaleXChannel ? scaleXChannel->currentValue() : args->scaleX();
            qreal yScale = scaleYChannel ? scaleYChannel->currentValue() : args->scaleY();

            args->setScaleX(xScale);
            args->setScaleY(yScale);
        }
    }

    {
        // Shear

        KisScalarKeyframeChannel *shearXChannel = channelFor(KisKeyframeChannel::ShearX.id());
        KisScalarKeyframeChannel *shearYChannel = channelFor(KisKeyframeChannel::ShearY.id());

        if (shearXChannel || shearYChannel) {
            qreal xShear = shearXChannel ? shearXChannel->currentValue() : args->shearX();
            qreal yShear = shearYChannel ? shearYChannel->currentValue() : args->shearY();

            args->setShearX(xShear);
            args->setShearY(yShear);
        }
    }

    {
        // Rotation

        KisScalarKeyframeChannel *rotationXChannel = channelFor(KisKeyframeChannel::RotationX.id());
        KisScalarKeyframeChannel *rotationYChannel = channelFor(KisKeyframeChannel::RotationY.id());
        KisScalarKeyframeChannel *rotationZChannel = channelFor(KisKeyframeChannel::RotationZ.id());

        if (rotationXChannel || rotationYChannel || rotationZChannel) {
            qreal xRotation = rotationXChannel ? kisDegreesToRadians(rotationXChannel->currentValue()) : args->aX();
            qreal yRotation = rotationYChannel ? kisDegreesToRadians(rotationYChannel->currentValue()) : args->aY();
            qreal zRotation = rotationZChannel ? kisDegreesToRadians(rotationZChannel->currentValue()) : args->aZ();

            args->setAX(xRotation);
            args->setAY(yRotation);
            args->setAZ(zRotation);
        }
    }

    return args;
}

void KisAnimatedTransformMaskParamsHolder::setDefaultBounds(KisDefaultBoundsBaseSP bounds)
{
    m_d->defaultBounds = bounds;
    m_d->baseArgs.setDefaultBounds(bounds);

    Q_FOREACH(QSharedPointer<KisScalarKeyframeChannel> channel, m_d->transformChannels) {
        channel->setDefaultBounds(bounds);
    }
}

KisDefaultBoundsBaseSP KisAnimatedTransformMaskParamsHolder::defaultBounds() const
{
    return m_d->defaultBounds;
}

KisKeyframeChannel *KisAnimatedTransformMaskParamsHolder::requestKeyframeChannel(const QString &id)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!m_d->transformChannels.contains(id), m_d->transformChannels.value(id).data());

    /**
     * We shouldn't create the channels in lodN mode
     */
    KIS_SAFE_ASSERT_RECOVER_NOOP(m_d->defaultBounds->currentLevelOfDetail() <= 0);

    const KoID channelId = KisKeyframeChannel::channelIdToKoId(id);
    KisScalarKeyframeChannel *channel = new KisScalarKeyframeChannel(channelId, m_d->defaultBounds);
    channel->setDefaultValue(defaultValueForScalarChannel(channelId));
    channel->setDefaultInterpolationMode(KisScalarKeyframe::Linear);
    m_d->transformChannels.insert(id, toQShared(channel));


    // TODO: recover or remove this link!!!
    //
    //    channel->connect(m_d->transformChannels[name].data(), &KisKeyframeChannel::sigAnyKeyframeChange, [this] () {
    //        this->clearChangedFlag();
    //    });

    return channel;
}

KisKeyframeChannel *KisAnimatedTransformMaskParamsHolder::getKeyframeChannel(const QString &id) const
{
    return m_d->transformChannels.value(id, nullptr).data();
}

qreal KisAnimatedTransformMaskParamsHolder::defaultValueForScalarChannel(const KoID &id)
{
    QSharedPointer<ToolTransformArgs> args = transformArgs();

    if (id == KisKeyframeChannel::PositionX) {
        return args->transformedCenter().x();
    } else if (id == KisKeyframeChannel::PositionY) {
        return args->transformedCenter().y();
    } else if (id == KisKeyframeChannel::ScaleX) {
        return args->scaleX();
    } else if (id == KisKeyframeChannel::ScaleY) {
        return args->scaleY();
    } else if (id == KisKeyframeChannel::ShearX) {
        return args->shearX();
    } else if (id == KisKeyframeChannel::ShearY) {
        return args->shearY();
    } else if (id == KisKeyframeChannel::RotationX) {
        return kisRadiansToDegrees(args->aX());
    } else if (id == KisKeyframeChannel::RotationY) {
        return kisRadiansToDegrees(args->aY());
    } else if (id == KisKeyframeChannel::RotationZ) {
        return kisRadiansToDegrees(args->aZ());
    } else {
        return 0.0f;
    }
}

void KisAnimatedTransformMaskParamsHolder::clearChangedFlag()
{
    m_d->hash = generateStateHash();
}

bool KisAnimatedTransformMaskParamsHolder::hasChanged() const
{
    return m_d->hash != generateStateHash();
}

void KisAnimatedTransformMaskParamsHolder::syncLodCache()
{
    m_d->baseArgs.syncLodCache();
}

KisAnimatedTransformParamsHolderInterfaceSP KisAnimatedTransformMaskParamsHolder::clone() const
{
    return toQShared(new KisAnimatedTransformMaskParamsHolder(*this));
}

KisTransformMaskParamsInterfaceSP KisAnimatedTransformMaskParamsHolder::bakeIntoParams() const
{
    return toQShared(new KisTransformMaskAdapter(*transformArgs(), m_d->isHidden, m_d->isInitialized));
}

void KisAnimatedTransformMaskParamsHolder::setParamsAtCurrentPosition(const KisTransformMaskParamsInterface *params, KUndo2Command *parentCommand)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->transformChannels.isEmpty() || m_d->transformChannels.size() == 9);

    const KisTransformMaskAdapter *adapter = dynamic_cast<const KisTransformMaskAdapter*>(params);
    KIS_SAFE_ASSERT_RECOVER_RETURN(adapter);

    makeChangeValueCommand<&Private::isHidden>(m_d.data(), adapter->isHidden(), parentCommand);
    makeChangeValueCommand<&Private::isInitialized>(m_d.data(), adapter->isInitialized(), parentCommand);

    ToolTransformArgs args = *adapter->transformArgs();

    setNewTransformArgs(*adapter->transformArgs(), parentCommand);
}

void KisAnimatedTransformMaskParamsHolder::setNewTransformArgs(const ToolTransformArgs &args, KUndo2Command *parentCommand)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->transformChannels.isEmpty() || m_d->transformChannels.size() == 9);

    struct ChangeParamsCommand : KisChangeValueCommand<&Private::baseArgs, KisLogCapableTransformArgs::LodState>
    {
        ChangeParamsCommand(Private *base,
                            const KisLogCapableTransformArgs::LodState &newValue,
                            KUndo2Command *parent = nullptr)
            : KisChangeValueCommand(base, newValue, parent)
        {
            KIS_SAFE_ASSERT_RECOVER_NOOP(m_oldValue.first == m_newValue.first);
        }
    };

    new ChangeParamsCommand(m_d.data(), std::make_pair(m_d->defaultBounds->currentLevelOfDetail(), args), parentCommand);

    if (m_d->transformChannels.isEmpty()) return;
    if (m_d->defaultBounds->currentLevelOfDetail() > 0) return;

    const int currentTime = m_d->defaultBounds->currentTime();

    auto setKeyframe = [this] (const KoID &channelId, int time, qreal value, KUndo2Command *parentCommand)
    {
        KisScalarKeyframeChannel *channel = m_d->transformChannels.value(channelId.id()).data();
        KIS_SAFE_ASSERT_RECOVER_RETURN(channel);

        if (channel->keyframeAt(time)){
            KisScalarKeyframeSP keyframe = channel->keyframeAt<KisScalarKeyframe>(time);
            keyframe->setValue(value, parentCommand);
        } else {
            channel->addScalarKeyframe(time, value, parentCommand);
        }
    };

    setKeyframe(KisKeyframeChannel::PositionX, currentTime, args.transformedCenter().x(), parentCommand);
    setKeyframe(KisKeyframeChannel::PositionY, currentTime, args.transformedCenter().y(), parentCommand);

    setKeyframe(KisKeyframeChannel::ScaleX, currentTime, args.scaleX(), parentCommand);
    setKeyframe(KisKeyframeChannel::ScaleY, currentTime, args.scaleY(), parentCommand);

    setKeyframe(KisKeyframeChannel::ShearX, currentTime, args.shearX(), parentCommand);
    setKeyframe(KisKeyframeChannel::ShearY, currentTime, args.shearY(), parentCommand);

    setKeyframe(KisKeyframeChannel::RotationX, currentTime, kisRadiansToDegrees(args.aX()), parentCommand);
    setKeyframe(KisKeyframeChannel::RotationY, currentTime, kisRadiansToDegrees(args.aY()), parentCommand);
    setKeyframe(KisKeyframeChannel::RotationZ, currentTime, kisRadiansToDegrees(args.aZ()), parentCommand);
}

quint64 KisAnimatedTransformMaskParamsHolder::generateStateHash() const
{
    return qHash(transformArgs()->transformedCenter().x())
            ^ qHash(transformArgs()->transformedCenter().y())
            ^ qHash(transformArgs()->originalCenter().x())
            ^ qHash(transformArgs()->originalCenter().y())
            ^ qHash(transformArgs()->rotationCenterOffset().x())
            ^ qHash(transformArgs()->rotationCenterOffset().y())
            ^ qHash(transformArgs()->scaleX())
            ^ qHash(transformArgs()->scaleY())
            ^ qHash(transformArgs()->aX())
            ^ qHash(transformArgs()->aY())
            ^ qHash(transformArgs()->aZ())
            ^ qHash(transformArgs()->alpha());
}

