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

struct KisAnimatedTransformMaskParameters::Private
{
    using TransformChannels = QHash<QString, QSharedPointer<KisScalarKeyframeChannel>>;

    Private() :
        hidden(false),
        hash()
    {
        transformChannels.insert(KisKeyframeChannel::PositionX.id(), nullptr);
        transformChannels.insert(KisKeyframeChannel::PositionY.id(), nullptr);
        transformChannels.insert(KisKeyframeChannel::ScaleX.id(), nullptr);
        transformChannels.insert(KisKeyframeChannel::ScaleY.id(), nullptr);
        transformChannels.insert(KisKeyframeChannel::ShearX.id(), nullptr);
        transformChannels.insert(KisKeyframeChannel::ShearY.id(), nullptr);
        transformChannels.insert(KisKeyframeChannel::RotationX.id(), nullptr);
        transformChannels.insert(KisKeyframeChannel::RotationY.id(), nullptr);
        transformChannels.insert(KisKeyframeChannel::RotationZ.id(), nullptr);
    }

    TransformChannels transformChannels;

    bool hidden;
    quint64 hash;
};

KoID chanNameToKoID(const QString &name) {
    KoID channelId;

    if (name == KisKeyframeChannel::PositionX.id()) {
        channelId = KisKeyframeChannel::PositionX;
    } else if (name == KisKeyframeChannel::PositionY.id()) {
        channelId = KisKeyframeChannel::PositionY;
    } else if (name == KisKeyframeChannel::ScaleX.id()) {
        channelId = KisKeyframeChannel::ScaleX;
    } else if (name == KisKeyframeChannel::ScaleY.id()) {
        channelId = KisKeyframeChannel::ScaleY;
    } else if (name == KisKeyframeChannel::ShearX.id()) {
        channelId = KisKeyframeChannel::ShearX;
    } else if (name == KisKeyframeChannel::ShearY.id()) {
        channelId = KisKeyframeChannel::ShearY;
    } else if (name == KisKeyframeChannel::RotationX.id()) {
        channelId = KisKeyframeChannel::RotationX;
    } else if (name == KisKeyframeChannel::RotationY.id()) {
        channelId = KisKeyframeChannel::RotationY;
    } else if (name == KisKeyframeChannel::RotationZ.id()) {
        channelId = KisKeyframeChannel::RotationZ;
    } else {
        channelId = KoID();
    }

    return channelId;
}

KoID chanIdToKoID(const QString &id) {
    KoID channelId;
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
        channelId = KoID();
    }

    return channelId;
}

KisAnimatedTransformMaskParameters::KisAnimatedTransformMaskParameters()
    : KisTransformMaskAdapter(),
      m_d(new Private())
{
}


KisAnimatedTransformMaskParameters::KisAnimatedTransformMaskParameters(const KisTransformMaskAdapter *staticTransform)
    : KisTransformMaskAdapter(*staticTransform->transformArgs()),
      m_d(new Private())
{
    clearChangedFlag();
}

KisAnimatedTransformMaskParameters::KisAnimatedTransformMaskParameters(const KisAnimatedTransformMaskParameters &rhs)
    : KisTransformMaskAdapter(*rhs.transformArgs()),
      m_d(new Private())
{
    m_d->hash = rhs.m_d->hash;
}


KisAnimatedTransformMaskParameters::~KisAnimatedTransformMaskParameters()
{
}

const QSharedPointer<ToolTransformArgs> KisAnimatedTransformMaskParameters::transformArgs() const
{
    QSharedPointer<ToolTransformArgs> args( new ToolTransformArgs(*KisTransformMaskAdapter::transformArgs()) );

    if (m_d->transformChannels[KisKeyframeChannel::PositionX.id()] || m_d->transformChannels[KisKeyframeChannel::PositionY.id()]) { // Position

        bool hasXKeys = m_d->transformChannels[KisKeyframeChannel::PositionX.id()] ? m_d->transformChannels[KisKeyframeChannel::PositionX.id()]->keyframeCount() > 0 : false;
        bool hasYKeys = m_d->transformChannels[KisKeyframeChannel::PositionY.id()] ? m_d->transformChannels[KisKeyframeChannel::PositionY.id()]->keyframeCount() > 0 : false;


        qreal xPosition = hasXKeys ? m_d->transformChannels[KisKeyframeChannel::PositionX.id()]->currentValue() : args->transformedCenter().x();
        qreal yPosition = hasYKeys ? m_d->transformChannels[KisKeyframeChannel::PositionY.id()]->currentValue() : args->transformedCenter().y();

        args->setTransformedCenter(QPointF(xPosition, yPosition));
    }

    {   // Scale
        bool hasXKeys = m_d->transformChannels[KisKeyframeChannel::ScaleX.id()] ? m_d->transformChannels[KisKeyframeChannel::ScaleX.id()]->keyframeCount() > 0 : false;
        bool hasYKeys = m_d->transformChannels[KisKeyframeChannel::ScaleY.id()] ? m_d->transformChannels[KisKeyframeChannel::ScaleY.id()]->keyframeCount() > 0 : false;

        qreal xScale = hasXKeys ? m_d->transformChannels[KisKeyframeChannel::ScaleX.id()]->currentValue() : args->scaleX();
        qreal yScale = hasYKeys ? m_d->transformChannels[KisKeyframeChannel::ScaleY.id()]->currentValue() : args->scaleY();

        args->setScaleX(xScale);
        args->setScaleY(yScale);
    }

    {   // Shear
        bool hasXKeys = m_d->transformChannels[KisKeyframeChannel::ShearX.id()] ? m_d->transformChannels[KisKeyframeChannel::ShearX.id()]->keyframeCount() > 0 : false;
        bool hasYKeys = m_d->transformChannels[KisKeyframeChannel::ShearY.id()] ? m_d->transformChannels[KisKeyframeChannel::ShearY.id()]->keyframeCount() > 0 : false;

        qreal xShear = hasXKeys ? m_d->transformChannels[KisKeyframeChannel::ShearX.id()]->currentValue() : args->shearX();
        qreal yShear = hasYKeys ? m_d->transformChannels[KisKeyframeChannel::ShearY.id()]->currentValue() : args->shearY();

        args->setShearX(xShear);
        args->setShearY(yShear);
    }

    {   // Rotation
        bool hasXKeys = m_d->transformChannels[KisKeyframeChannel::RotationX.id()] ? m_d->transformChannels[KisKeyframeChannel::RotationX.id()]->keyframeCount() > 0 : false;
        bool hasYKeys = m_d->transformChannels[KisKeyframeChannel::RotationY.id()] ? m_d->transformChannels[KisKeyframeChannel::RotationY.id()]->keyframeCount() > 0 : false;
        bool hasZKeys = m_d->transformChannels[KisKeyframeChannel::RotationZ.id()] ? m_d->transformChannels[KisKeyframeChannel::RotationZ.id()]->keyframeCount() > 0 : false;

        qreal xRotRad = hasXKeys ? degToRad(m_d->transformChannels[KisKeyframeChannel::RotationX.id()]->currentValue()) : args->aX();
        qreal yRotRad = hasYKeys ? degToRad(m_d->transformChannels[KisKeyframeChannel::RotationY.id()]->currentValue()) : args->aY();
        qreal zRotRad = hasZKeys ? degToRad(m_d->transformChannels[KisKeyframeChannel::RotationZ.id()]->currentValue()) : args->aZ();

        args->setAX(xRotRad);
        args->setAY(yRotRad);
        args->setAZ(zRotRad);
    }

    // Translate image to accommodate for rotation offset.
    if (args->mode() == ToolTransformArgs::FREE_TRANSFORM) {
        QPointF pos = args->transformedCenter() + getRotationalTranslationOffset(*args);
        args->setTransformedCenter(pos);
    }

    return args;
}

QString KisAnimatedTransformMaskParameters::id() const
{
    return "animatedtransformparams";
}

void KisAnimatedTransformMaskParameters::toXML(QDomElement *e) const
{
    return KisTransformMaskAdapter::toXML(e);
}


KisKeyframeChannel *KisAnimatedTransformMaskParameters::requestKeyframeChannel(const QString &id, KisNodeWSP parent)
{
    KoID channelId = chanIdToKoID(id);
    if (m_d->transformChannels[channelId.id()].isNull()) {
        setKeyframeChannel(channelId.id(), toQShared(new KisScalarKeyframeChannel(channelId, new KisDefaultBoundsNodeWrapper(parent))));
        m_d->transformChannels[channelId.id()]->setNode(parent);
    }

    return m_d->transformChannels[channelId.id()].data();
}

void KisAnimatedTransformMaskParameters::setKeyframeChannel(const QString &name, QSharedPointer<KisKeyframeChannel> kcsp) {
    if (kcsp.dynamicCast<KisScalarKeyframeChannel>()) {
        m_d->transformChannels[name] = kcsp.dynamicCast<KisScalarKeyframeChannel>();
        m_d->transformChannels[name]->setDefaultValue(defaultValueForScalarChannel(name));
        m_d->transformChannels[name]->setDefaultInterpolationMode(KisScalarKeyframe::Linear);
        m_d->transformChannels[name]->connect(m_d->transformChannels[name].data(), &KisKeyframeChannel::sigChannelUpdated, [this](const KisTimeSpan&, const QRect&){
            this->clearChangedFlag();
        });
    }
}

KisKeyframeChannel *KisAnimatedTransformMaskParameters::getKeyframeChannel(const KoID &koid) const
{
    return m_d->transformChannels[koid.id()].data();
}

QList<KisKeyframeChannel *> KisAnimatedTransformMaskParameters::copyChannelsFrom(const KisAnimatedTransformParamsInterface *other)
{
    QList<KisKeyframeChannel*> chans;

    for (Private::TransformChannels::iterator i = m_d->transformChannels.begin();
         i != m_d->transformChannels.end(); i++) {

        KoID channelId = chanNameToKoID(i.key());

        const KisScalarKeyframeChannel* src = dynamic_cast<KisScalarKeyframeChannel*>(other->getKeyframeChannel(channelId));

        if (!src) continue;

        setKeyframeChannel(channelId.id(), toQShared(new KisScalarKeyframeChannel(*src)));
        chans.append(getKeyframeChannel(channelId));
    }

    return chans;
}

qreal KisAnimatedTransformMaskParameters::defaultValueForScalarChannel(QString name)
{
    KoID channelID = chanNameToKoID(name);
    if (channelID == KisKeyframeChannel::PositionX) {
        return KisTransformMaskAdapter::transformArgs()->originalCenter().x();
    } else if (channelID == KisKeyframeChannel::PositionY) {
        return KisTransformMaskAdapter::transformArgs()->originalCenter().y();
    } else if (channelID == KisKeyframeChannel::ScaleX || channelID == KisKeyframeChannel::ScaleY) {
        return 1.0f;
    } else {
        return 0.0f;
    }
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
    m_d->hash = generateStateHash();
}

bool KisAnimatedTransformMaskParameters::hasChanged() const
{
    return m_d->hash != generateStateHash();
}

bool KisAnimatedTransformMaskParameters::isAnimated() const
{
    return true;
}

QPointF KisAnimatedTransformMaskParameters::getRotationalTranslationOffset(const ToolTransformArgs &args) const {
    KisTransformUtils::MatricesPack m(args);

    QTransform t1 = m.TS * m.SC * m.S;
    QTransform t2 = m.projectedP;

    QPointF orig1 = t1.map(args.originalCenter() - args.rotationCenterOffset());
    QPointF translationOffset = t2.map(orig1) - orig1;

    return translationOffset;
}

KisTransformMaskParamsInterfaceSP KisAnimatedTransformMaskParameters::clone() const
{
    return toQShared(new KisAnimatedTransformMaskParameters(*this));
}

KisTransformMaskParamsInterfaceSP KisAnimatedTransformMaskParameters::fromXML(const QDomElement &e)
{
    QSharedPointer<KisAnimatedTransformMaskParameters> p = toQShared(new KisAnimatedTransformMaskParameters());
    p->setBaseArgs(ToolTransformArgs::fromXML(e));
    return p;
}

KisTransformMaskParamsInterfaceSP KisAnimatedTransformMaskParameters::makeAnimated(KisTransformMaskParamsInterfaceSP params, const KisTransformMaskSP mask)
{
    KisAnimatedTransformMaskParameters* animMask;
    QSharedPointer<KisTransformMaskAdapter> tma = params.dynamicCast<KisTransformMaskAdapter>();
    if (tma) {
        animMask = new KisAnimatedTransformMaskParameters(tma.data());
    } else {
        animMask = new KisAnimatedTransformMaskParameters();
        ToolTransformArgs args;
        args.setOriginalCenter(mask->sourceDataBounds().center());
        animMask->setBaseArgs(args);
    }

    animMask->clearChangedFlag();

    return toQShared(animMask);
}

void makeScalarKeyframeOnMask(KisTransformMaskSP mask, const KoID &channelId, int time, qreal value, KUndo2Command *parentCommand)
{
    KisScalarKeyframeChannel *channel = dynamic_cast<KisScalarKeyframeChannel*>(mask->getKeyframeChannel(channelId.id(), false));
    if (channel){
        channel->addScalarKeyframe(time, value, parentCommand);
    }
}

void setScalarKeyframeOnMask(KisTransformMaskSP mask, const KoID &channelId, int time, qreal value, KUndo2Command *parentCommand)
{
    KisScalarKeyframeChannel *channel = dynamic_cast<KisScalarKeyframeChannel*>(mask->getKeyframeChannel(channelId.id(), false));
    if (channel && channel->keyframeAt(time)){
        KisScalarKeyframeSP keyframe = channel->keyframeAt<KisScalarKeyframe>(time);
        keyframe->setValue(value, parentCommand);
    }
}

ToolTransformArgs fetchToolTransformArgs(KisTransformMaskSP mask, KisTransformMaskParamsInterfaceSP desiredParams) {
    ToolTransformArgs desiredArgs;
    KisTransformMaskParamsInterfaceSP currentParams = mask->transformParams();
    KisTransformMaskAdapter* desiredParamsAdapter = dynamic_cast<KisTransformMaskAdapter*>(desiredParams.data());

    if (desiredParamsAdapter) {
        desiredArgs = *desiredParamsAdapter->transformArgs();
    }

    KisTransformMaskAdapter *tma = dynamic_cast<KisTransformMaskAdapter*>(mask->transformParams().data());
    if (tma) {
        tma->setBaseArgs(desiredArgs);
    }

    // Undo any translation offset intended for origin-offset based rotation.
    KisAnimatedTransformMaskParameters* animatedTransformMask = dynamic_cast<KisAnimatedTransformMaskParameters*>(currentParams.data());
    if (animatedTransformMask && desiredArgs.mode() == ToolTransformArgs::FREE_TRANSFORM) {
        QPointF pos = desiredArgs.transformedCenter() - animatedTransformMask->getRotationalTranslationOffset(desiredArgs);
        desiredArgs.setTransformedCenter(pos);
    }

    return desiredArgs;
}


void KisAnimatedTransformMaskParameters::addKeyframes(KisTransformMaskSP mask, int currentTime, KisTransformMaskParamsInterfaceSP desiredParams, KUndo2Command *parentCommand)
{
    KisTransformMaskParamsInterfaceSP currentParams = mask->transformParams();
    if (dynamic_cast<KisAnimatedTransformMaskParameters*>(currentParams.data()) == 0) {
        mask->setTransformParams(makeAnimated(currentParams, mask));
        currentParams = mask->transformParams();
        KIS_ASSERT(currentParams);
    }

    // If there's no desired param passed, use the current parameters.
    if (desiredParams.isNull()) {
        desiredParams = currentParams;
        return;
    }

    ToolTransformArgs desiredArgs = fetchToolTransformArgs(mask, desiredParams);

    makeScalarKeyframeOnMask(mask, KisKeyframeChannel::PositionX, currentTime, desiredArgs.transformedCenter().x(), parentCommand);
    makeScalarKeyframeOnMask(mask, KisKeyframeChannel::PositionY, currentTime, desiredArgs.transformedCenter().y(), parentCommand);

    makeScalarKeyframeOnMask(mask, KisKeyframeChannel::ScaleX, currentTime, desiredArgs.scaleX(), parentCommand);
    makeScalarKeyframeOnMask(mask, KisKeyframeChannel::ScaleY, currentTime, desiredArgs.scaleY(), parentCommand);

    makeScalarKeyframeOnMask(mask, KisKeyframeChannel::ShearX, currentTime, desiredArgs.shearX(), parentCommand);
    makeScalarKeyframeOnMask(mask, KisKeyframeChannel::ShearY, currentTime, desiredArgs.shearY(), parentCommand);

    makeScalarKeyframeOnMask(mask, KisKeyframeChannel::RotationX, currentTime, radToDeg(desiredArgs.aX()), parentCommand);
    makeScalarKeyframeOnMask(mask, KisKeyframeChannel::RotationY, currentTime, radToDeg(desiredArgs.aY()), parentCommand);
    makeScalarKeyframeOnMask(mask, KisKeyframeChannel::RotationZ, currentTime, radToDeg(desiredArgs.aZ()), parentCommand);
}

void KisAnimatedTransformMaskParameters::setKeyframes(KisTransformMaskSP mask, int currentTime, KisTransformMaskParamsInterfaceSP desiredParams, KUndo2Command *parentCommand)
{
    KisTransformMaskParamsInterfaceSP currentParams = mask->transformParams();
    if (dynamic_cast<KisAnimatedTransformMaskParameters*>(currentParams.data()) == 0) {
        mask->setTransformParams(makeAnimated(currentParams, mask));
        currentParams = mask->transformParams();
        KIS_ASSERT(currentParams);
    }

    // If there's no desired param passed, use the current parameters.
    if (desiredParams.isNull()) {
        desiredParams = currentParams;
        return;
    }

    ToolTransformArgs desiredArgs = fetchToolTransformArgs(mask, desiredParams);

    setScalarKeyframeOnMask(mask, KisKeyframeChannel::PositionX, currentTime, desiredArgs.transformedCenter().x(), parentCommand);
    setScalarKeyframeOnMask(mask, KisKeyframeChannel::PositionY, currentTime, desiredArgs.transformedCenter().y(), parentCommand);

    setScalarKeyframeOnMask(mask, KisKeyframeChannel::ScaleX, currentTime, desiredArgs.scaleX(), parentCommand);
    setScalarKeyframeOnMask(mask, KisKeyframeChannel::ScaleY, currentTime, desiredArgs.scaleY(), parentCommand);

    setScalarKeyframeOnMask(mask, KisKeyframeChannel::ShearX, currentTime, desiredArgs.shearX(), parentCommand);
    setScalarKeyframeOnMask(mask, KisKeyframeChannel::ShearY, currentTime, desiredArgs.shearY(), parentCommand);

    setScalarKeyframeOnMask(mask, KisKeyframeChannel::RotationX, currentTime, radToDeg(desiredArgs.aX()), parentCommand);
    setScalarKeyframeOnMask(mask, KisKeyframeChannel::RotationY, currentTime, radToDeg(desiredArgs.aY()), parentCommand);
    setScalarKeyframeOnMask(mask, KisKeyframeChannel::RotationZ, currentTime, radToDeg(desiredArgs.aZ()), parentCommand);
}

void KisAnimatedTransformMaskParameters::removeKeyframes(KisTransformMaskSP mask, int currentTime)
{
    QList<KoID> lists = { KisKeyframeChannel::PositionX
                        , KisKeyframeChannel::PositionY
                        , KisKeyframeChannel::ScaleX
                        , KisKeyframeChannel::ScaleY
                        , KisKeyframeChannel::ShearX
                        , KisKeyframeChannel::ShearY
                        , KisKeyframeChannel::RotationX
                        , KisKeyframeChannel::RotationY
                        , KisKeyframeChannel::RotationZ };

    for (int i = 0; i < lists.length(); i++) {
        KoID koid = lists[i];
        KisKeyframeChannel* channel = mask->getKeyframeChannel(koid.id(), false);
        if (channel && channel->keyframeAt(currentTime)) {
            channel->removeKeyframe(currentTime);
        }
    }
}

quint64 KisAnimatedTransformMaskParameters::generateStateHash() const
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
