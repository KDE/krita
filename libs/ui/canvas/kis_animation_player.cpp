/*
 *  SPDX-FileCopyrightText: 2015 Jouni Pentikäinen <joupent@gmail.com>
 *  SPDX-FileCopyrightText: 2021 Eoin O'Neill <eoinoneill1991@gmail.com>
 *  SPDX-FileCopyrightText: 2021 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_animation_player.h"

#include "KisElapsedTimer.h"
#include <QTimer>
#include <QtMath>

#include "kis_global.h"
#include "kis_algebra_2d.h"

#include "kis_config.h"
#include "kis_config_notifier.h"
#include "kis_image.h"
#include "kis_canvas2.h"
#include "kis_animation_frame_cache.h"
#include "kis_signal_auto_connection.h"
#include "kis_image_animation_interface.h"
#include "kis_time_span.h"
#include "kis_signal_compressor.h"
#include "animation/KisFrameDisplayProxy.h"
#include <KisDocument.h>
#include <QFileInfo>
#include <QAudioDecoder>
#include <QThread>
#include "KisSyncedAudioPlayback.h"
#include "kis_signal_compressor_with_param.h"
#include "kis_image_barrier_locker.h"
#include "kis_layer_utils.h"
#include "KisDecoratedNodeInterface.h"
#include "kis_keyframe_channel.h"
#include "kis_algebra_2d.h"
#include <mlt++/MltProducer.h>
#include "animation/KisMediaConsumer.h"

#include "kis_image_config.h"
#include <limits>

#include "KisViewManager.h"
#include "kis_icon_utils.h"

#include "KisPart.h"
#include "dialogs/KisAsyncAnimationCacheRenderDialog.h"
#include "KisRollingMeanAccumulatorWrapper.h"
#include "kis_onion_skin_compositor.h"

#include <atomic>

//TEMP
#include <mlt++/MltFactory.h>
#include <mlt++/MltProfile.h>
#include <mlt++/MltProducer.h>
#include <mlt++/MltConsumer.h>
static void TESTonConsumerFrameShow(mlt_consumer c, void* p_self, mlt_frame p_frame) { // TEMP
    const auto self = static_cast<KisAnimationPlayer*>(p_self);
    Mlt::Frame frame(p_frame);
    self->sigTESTdisplayFrameAsync(frame.get_position());
}

qint64 framesToMSec(qreal value, int fps) {
    return qRound(value / fps * 1000.0);
}

qreal msecToFrames(qint64 value, int fps) {
    return qreal(value) * fps / 1000.0;
}

int framesToScaledTimeMS(qreal frame, int fps, qreal playbackSpeed) {
    return qRound(framesToMSec(frame, fps) / playbackSpeed);
}

qreal scaledTimeToFrames(qint64 time, int fps, qreal playbackSpeed) {
    return msecToFrames(time, fps) * playbackSpeed;
}

class PlaybackHandle : public QObject {
    Q_OBJECT
public:
    PlaybackHandle(int originFrame, KisAnimationPlayer* parent = nullptr)
        : QObject(parent)
        , m_originFrame(originFrame)
    {
        connect(&m_cancelTrigger, SIGNAL(output()), parent, SLOT(stop()));
    }

    ~PlaybackHandle() {
        restore();
    }

    PlaybackHandle() = delete;
    PlaybackHandle(const PlaybackHandle&) = delete;
    PlaybackHandle& operator= (const PlaybackHandle&) = delete;

    int originFrame() { return m_originFrame; }

    KisTimeSpan playbackRange() const {
        return m_playbackRange;
    }

    void setPlaybackRange(KisTimeSpan p_playbackRange) {
        m_playbackRange = p_playbackRange;
    }

    void prepare(KisCanvas2* canvas)
    {
        KIS_ASSERT(canvas); // Sanity check...
        m_canvas = canvas;

        const KisTimeSpan range = canvas->image()->animationInterface()->playbackRange();
        setPlaybackRange(range);

        // Initialize and optimize playback environment...
        if (canvas->frameCache()) {
            KisImageConfig cfg(true);

            const int dimensionLimit = cfg.useAnimationCacheFrameSizeLimit() ?
                        cfg.animationCacheFrameSizeLimit() : std::numeric_limits<int>::max();

            const int largestDimension = KisAlgebra2D::maxDimension(canvas->image()->bounds());

            const QRect regionOfInterest =
                        cfg.useAnimationCacheRegionOfInterest() && largestDimension > dimensionLimit ?
                            canvas->regionOfInterest() : canvas->coordinatesConverter()->imageRectInImagePixels();

            const QRect minimalRect =
                    canvas->coordinatesConverter()->widgetRectInImagePixels().toAlignedRect() &
                    canvas->coordinatesConverter()->imageRectInImagePixels();

            canvas->frameCache()->dropLowQualityFrames(range, regionOfInterest, minimalRect);
            canvas->setRenderingLimit(regionOfInterest);

            // Preemptively cache all frames...
            KisAsyncAnimationCacheRenderDialog dlg(canvas->frameCache(), range);
            dlg.setRegionOfInterest(regionOfInterest);
            dlg.regenerateRange(canvas->viewManager());
        } else {
            KisImageBarrierLocker locker(canvas->image());
            KisLayerUtils::recursiveApplyNodes(canvas->image()->root(), [this](KisNodeSP node){
                KisDecoratedNodeInterface* decoratedNode = dynamic_cast<KisDecoratedNodeInterface*>(node.data());
                if (decoratedNode && decoratedNode->decorationsVisible()) {
                    decoratedNode->setDecorationsVisible(false, false);
                    m_disabledDecoratedNodes.append(node);
                }
            });
        }

        // Setup appropriate interrupt connections...
        m_cancelStrokeConnections.addConnection(
                canvas->image().data(), SIGNAL(sigUndoDuringStrokeRequested()),
                &m_cancelTrigger, SLOT(tryFire()));

        m_cancelStrokeConnections.addConnection(
                canvas->image().data(), SIGNAL(sigStrokeCancellationRequested()),
                &m_cancelTrigger, SLOT(tryFire()));

        // We only want to stop on stroke end when running on a system
        // without cache / opengl / graphics driver support!
        if (canvas->frameCache()) {
            m_cancelStrokeConnections.addConnection(
                    canvas->image().data(), SIGNAL(sigStrokeEndRequested()),
                    &m_cancelTrigger, SLOT(tryFire()));
        }
    }

    void restore() {
        m_cancelStrokeConnections.clear();

        if (m_canvas) {
            if (m_canvas->frameCache()) {
                m_canvas->setRenderingLimit(QRect());
            } else {
                KisImageBarrierLocker locker(m_canvas->image());
                Q_FOREACH(KisNodeWSP disabledNode, m_disabledDecoratedNodes) {
                    KisDecoratedNodeInterface* decoratedNode = dynamic_cast<KisDecoratedNodeInterface*>(disabledNode.data());
                    if (decoratedNode) {
                        decoratedNode->setDecorationsVisible(true, true);
                    }
                }
                m_disabledDecoratedNodes.clear();
            }

            m_canvas = nullptr;
        }
    }


Q_SIGNALS:
    void finishedSeeking();

private:
    int m_originFrame; //!< The frame user started playback from.
    KisSignalAutoConnectionsStore m_cancelStrokeConnections;
    SingleShotSignal m_cancelTrigger;
    QVector<KisNodeWSP> m_disabledDecoratedNodes;

    KisCanvas2* m_canvas;

    KisTimeSpan m_playbackRange;
};

#include "kis_animation_player.moc"

struct KisAnimationPlayer::Private
{
public:
    Private(KisAnimationPlayer* p_self, KisCanvas2* p_canvas)
        : canvas(p_canvas)
        //, mediaConsumer( new KisMediaConsumer(p_self) )
        , displayProxy( new KisFrameDisplayProxy(p_canvas, p_self) )
        , playbackStatisticsCompressor(1000, KisSignalCompressor::FIRST_INACTIVE)
    {
    }

    KisCanvas2 *canvas;
    PlaybackState state;
    QScopedPointer<PlaybackHandle> playbackHandle;
    //QScopedPointer<KisMediaConsumer> mediaConsumer;
    QScopedPointer<KisFrameDisplayProxy> displayProxy;

    KisSignalCompressor playbackStatisticsCompressor;

    //TEMP DELETE
    QScopedPointer<Mlt::Profile> TESTprofile;
    QScopedPointer<Mlt::Consumer> TESTconsumer;
    QScopedPointer<Mlt::Producer> TESTproducer;
};

KisAnimationPlayer::KisAnimationPlayer(KisCanvas2 *canvas)
    : QObject(canvas)
    , m_d(new Private(this, canvas))
{
    setPlaybackState(STOPPED);
    setPlaybackSpeedNormalized(1.0f);

    /*connect(KisConfigNotifier::instance(),
            &KisConfigNotifier::dropFramesModeChanged,
            this,
            &KisAnimationPlayer::updateDropFramesMode);
    updateDropFramesMode();

    connect(&m_d->playbackStatisticsCompressor,ENTER_FUNCTION(); SIGNAL(timeout()),
            this, SIGNAL(sigPlaybackStatisticsUpdated()));*/

    /*connect(m_d->mediaConsumer.data(), &KisMediaConsumer::sigFrameShow, this, [this](int p_frame){
        KIS_ASSERT(m_d->displayProxy);
        if (m_d->state == PLAYING) {
            m_d->displayProxy->displayFrame(p_frame);
        }
    });*/

    connect(m_d->displayProxy.data(), SIGNAL(sigDisplayFrameChanged()), this, SIGNAL(sigFrameChanged()));

    // Grow to new playback range when new frames added (configurable)...
    connect(m_d->canvas->image()->animationInterface(), &KisImageAnimationInterface::sigKeyframeAdded, this, [this](const KisKeyframeChannel*, int time){
        if (m_d->canvas && m_d->canvas->image()) {
            KisImageAnimationInterface* animInterface = m_d->canvas->image()->animationInterface();
            KisConfig cfg(true);
            if (animInterface && cfg.adaptivePlaybackRange()) {
                KisTimeSpan desiredPlaybackRange = animInterface->fullClipRange();
                desiredPlaybackRange.include(time);
                animInterface->setFullClipRange(desiredPlaybackRange);
            }
        }
    });

    /*connect(m_d->canvas->image()->animationInterface(), &KisImageAnimationInterface::sigFramerateChanged, this, [this](){
       m_d->mediaConsumer->setFrameRate(m_d->canvas->image()->animationInterface()->framerate());
    });
    m_d->mediaConsumer->setFrameRate(m_d->canvas->image()->animationInterface()->framerate());*/

    /*connect(m_d->canvas->imageView()->document(), &KisDocument::sigAudioTracksChanged, this, &KisAnimationPlayer::setupAudioTracks);
    setupAudioTracks();*/





    //TEMP
    Mlt::Factory::init();

    connect(this, SIGNAL(sigTESTdisplayFrameAsync(int)), this, SLOT(TESTdisplayFrame(int)), Qt::QueuedConnection);

    m_d->TESTprofile.reset(new Mlt::Profile);

    m_d->TESTproducer.reset(new Mlt::Producer(*m_d->TESTprofile, "count"));
    //m_d->TESTproducer.reset(new Mlt::Producer(*m_d->TESTprofile, "count"));

    m_d->TESTconsumer.reset(new Mlt::Consumer(*m_d->TESTprofile, "sdl2_audio"));
    m_d->TESTconsumer->connect_producer(*m_d->TESTproducer);
    m_d->TESTconsumer->listen("consumer-frame-show", this, (mlt_listener)TESTonConsumerFrameShow);
}

KisAnimationPlayer::~KisAnimationPlayer()
{
    Mlt::Factory::close();
}

KisAnimationPlayer::PlaybackState KisAnimationPlayer::playbackState()
{
    return m_d->state;
}

void KisAnimationPlayer::updateDropFramesMode()
{
    KisConfig cfg(true);
}

void KisAnimationPlayer::play()
{
    ENTER_FUNCTION() << "START" << ppVar(m_d->displayProxy->visibleFrame()) << ppVar(m_d->TESTproducer->position()) << ppVar(m_d->TESTconsumer->position());

    m_d->TESTconsumer->start();
    setPlaybackState(PLAYING);

    ENTER_FUNCTION() << "END" << ppVar(m_d->displayProxy->visibleFrame()) << ppVar(m_d->TESTproducer->position()) << ppVar(m_d->TESTconsumer->position());

    /*KIS_ASSERT(m_d->canvas);
    ENTER_FUNCTION() << "START" << ppVar(m_d->displayProxy->visibleFrame()) << ppVar(m_d->mediaConsumer->playhead());

    if (!m_d->playbackHandle) {
        m_d->playbackHandle.reset(new PlaybackHandle(m_d->displayProxy->visibleFrame(), this));
    }

    m_d->playbackHandle->prepare(m_d->canvas);
    setPlaybackState(PLAYING);
    m_d->mediaConsumer->seek(m_d->displayProxy->visibleFrame());

    ENTER_FUNCTION() << "END" << ppVar(m_d->displayProxy->visibleFrame()) << ppVar(m_d->mediaConsumer->playhead());*/
}

void KisAnimationPlayer::pause()
{
    ENTER_FUNCTION() << "START" << ppVar(m_d->displayProxy->visibleFrame()) << ppVar(m_d->TESTproducer->position()) << ppVar(m_d->TESTconsumer->position());

    m_d->TESTconsumer->stop();
    resync();
    setPlaybackState(PAUSED);

    ENTER_FUNCTION() << "END" << ppVar(m_d->displayProxy->visibleFrame()) << ppVar(m_d->TESTproducer->position()) << ppVar(m_d->TESTconsumer->position());

    /*ENTER_FUNCTION() << "START" << ppVar(m_d->displayProxy->visibleFrame()) << ppVar(m_d->mediaConsumer->playhead());
    if (playbackState() == PLAYING) {
        KIS_ASSERT(m_d->playbackHandle);
        setPlaybackState(PAUSED);
        m_d->playbackHandle->restore();
        m_d->mediaConsumer->seek(m_d->displayProxy->visibleFrame());

    }

    ENTER_FUNCTION() << "END" << ppVar(m_d->displayProxy->visibleFrame()) << ppVar(m_d->mediaConsumer->playhead());*/
}

void KisAnimationPlayer::playPause()
{
    if (m_d->state == PLAYING) {
        pause();
    } else {
        play();
    }
}

void KisAnimationPlayer::stop()
{
    ENTER_FUNCTION() << "START" << ppVar(m_d->displayProxy->visibleFrame()) << ppVar(m_d->TESTproducer->position()) << ppVar(m_d->TESTconsumer->position());

    m_d->TESTconsumer->stop();
    m_d->TESTproducer->seek(0);
    setPlaybackState(STOPPED);

    ENTER_FUNCTION() << "END" << ppVar(m_d->displayProxy->visibleFrame()) << ppVar(m_d->TESTproducer->position()) << ppVar(m_d->TESTconsumer->position());


    /*ENTER_FUNCTION() << "START" << ppVar(m_d->displayProxy->visibleFrame()) << ppVar(m_d->mediaConsumer->playhead());
    KisImageAnimationInterface* animation = m_d->canvas->image()->animationInterface();
    if(m_d->state == STOPPED) {
        KIS_SAFE_ASSERT_RECOVER_RETURN(animation);
        const int startFrame = animation->fullClipRange().start();
        scrub(startFrame);
    } else if (m_d->playbackHandle) {
        const int origin = m_d->playbackHandle->originFrame();
        m_d->playbackHandle->restore();
        m_d->playbackHandle.reset();
        setPlaybackState(STOPPED);
        scrub(origin, false);
    }

    ENTER_FUNCTION() << "END" << ppVar(m_d->displayProxy->visibleFrame()) << ppVar(m_d->mediaConsumer->playhead());*/
}

void KisAnimationPlayer::scrub(int frameIndex, bool preferCachedFrames)
{
    if (!m_d->canvas || !m_d->canvas->image()) return;


    if (m_d->state != PLAYING) {
        //m_d->mediaConsumer->seek(frameIndex);
        m_d->displayProxy->displayFrame(frameIndex);
    }
}

void KisAnimationPlayer::resync()
{
    m_d->TESTproducer->seek(m_d->displayProxy->visibleFrame());
}

void KisAnimationPlayer::previousFrame()
{
    if (!m_d->canvas) return;
    KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();

    const int startFrame = animInterface->playbackRange().start();
    const int endFrame = animInterface->playbackRange().end();

    int frame = visibleFrame() - 1;

    if (frame < startFrame || frame >  endFrame) {
        frame = endFrame;
    }

    if (frame >= 0) {
        if (m_d->state != STOPPED) {
            stop();
        }

        scrub(frame);
    }
}

void KisAnimationPlayer::nextFrame()
{
    if (!m_d->canvas) return;
    KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();

    const int startFrame = animInterface->playbackRange().start();
    const int endFrame = animInterface->playbackRange().end();

    int frame = visibleFrame() + 1;

    if (frame > endFrame || frame < startFrame ) {
        frame = startFrame;
    }

    if (frame >= 0) {
        if (m_d->state != STOPPED) {
            stop();
        }

        scrub(frame);
    }
}

void KisAnimationPlayer::previousKeyframe()
{
    if (!m_d->canvas) return;

    KisNodeSP node = m_d->canvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    int currentFrame = visibleFrame();

    int destinationTime = -1;
    if (!keyframes->keyframeAt(currentFrame)) {
        destinationTime = keyframes->activeKeyframeTime(currentFrame);
    } else {
        destinationTime = keyframes->previousKeyframeTime(currentFrame);
    }

    if (keyframes->keyframeAt(destinationTime)) {
        if (m_d->state != STOPPED) {
            stop();
        }

        scrub(destinationTime);
    }
}

void KisAnimationPlayer::nextKeyframe()
{
    if (!m_d->canvas) return;
    KisNodeSP node = m_d->canvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    int currentTime = visibleFrame();

    int destinationTime = -1;
    if (keyframes->activeKeyframeAt(currentTime)) {
        destinationTime = keyframes->nextKeyframeTime(currentTime);
    }

    if (keyframes->keyframeAt(destinationTime)) {
        // Jump to next key...
        if (m_d->state != STOPPED) {
            stop();
        }

        scrub(destinationTime);
    } else {
        // Jump ahead by estimated timing...
        const int activeKeyTime = keyframes->activeKeyframeTime(currentTime);
        const int previousKeyTime = keyframes->previousKeyframeTime(activeKeyTime);

        if (previousKeyTime != -1) {
            if (m_d->state != STOPPED) {
                stop();
            }

            const int timing = activeKeyTime - previousKeyTime;
            scrub(currentTime + timing);
        }
    }
}

int KisAnimationPlayer::visibleFrame()
{
    return m_d->displayProxy->visibleFrame();
}

void KisAnimationPlayer::previousMatchingKeyframe()
{
    if (!m_d->canvas) return;

    KisNodeSP node = m_d->canvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    int time = visibleFrame();

    KisKeyframeSP currentKeyframe = keyframes->keyframeAt(time);
    int destinationTime = keyframes->activeKeyframeTime(time);
    const int desiredColor = currentKeyframe ? currentKeyframe->colorLabel() : keyframes->keyframeAt(destinationTime)->colorLabel();
    previousKeyframeWithColor(desiredColor);
}

void KisAnimationPlayer::nextMatchingKeyframe()
{
    if (!m_d->canvas) return;

    KisNodeSP node = m_d->canvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    int time = visibleFrame();

    if (!keyframes->activeKeyframeAt(time)) {
        return;
    }

    int destinationTime = keyframes->activeKeyframeTime(time);
    nextKeyframeWithColor(keyframes->keyframeAt(destinationTime)->colorLabel());
}

void KisAnimationPlayer::previousUnfilteredKeyframe()
{
    previousKeyframeWithColor(KisOnionSkinCompositor::instance()->colorLabelFilter());
}

void KisAnimationPlayer::nextUnfilteredKeyframe()
{
    nextKeyframeWithColor(KisOnionSkinCompositor::instance()->colorLabelFilter());
}

void KisAnimationPlayer::nextKeyframeWithColor(int color)
{
    QSet<int> validColors;
    validColors.insert(color);
    nextKeyframeWithColor(validColors);
}

void KisAnimationPlayer::nextKeyframeWithColor(const QSet<int> &validColors)
{
    if (!m_d->canvas) return;

    KisNodeSP node = m_d->canvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    int time = visibleFrame();

    if (!keyframes->activeKeyframeAt(time)) {
        return;
    }

    int destinationTime = keyframes->activeKeyframeTime(time);
    while ( keyframes->keyframeAt(destinationTime) &&
            ((destinationTime == time) ||
            !validColors.contains(keyframes->keyframeAt(destinationTime)->colorLabel()))) {

        destinationTime = keyframes->nextKeyframeTime(destinationTime);
    }

    if (keyframes->keyframeAt(destinationTime)) {
        if (m_d->state != STOPPED) {
            stop();
        }

        scrub(destinationTime);
    }
}

void KisAnimationPlayer::previousKeyframeWithColor(int color)
{
    QSet<int> validColors;
    validColors.insert(color);
    previousKeyframeWithColor(validColors);
}

void KisAnimationPlayer::previousKeyframeWithColor(const QSet<int> &validColors)
{
    if (!m_d->canvas) return;

    KisNodeSP node = m_d->canvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    int time = visibleFrame();

    int destinationTime = keyframes->activeKeyframeTime(time);
    while (keyframes->keyframeAt(destinationTime) &&
           ((destinationTime == time) ||
           !validColors.contains(keyframes->keyframeAt(destinationTime)->colorLabel()))) {

        destinationTime = keyframes->previousKeyframeTime(destinationTime);
    }

    if (keyframes->keyframeAt(destinationTime)) {
        if (m_d->state != STOPPED) {
            stop();
        }

        scrub(destinationTime);
    }
}

KisTimeSpan KisAnimationPlayer::activePlaybackRange()
{
    if (!m_d->canvas || !m_d->canvas->image()) {
        return KisTimeSpan::infinite(0);
    }

    const KisImageAnimationInterface *animation = m_d->canvas->image()->animationInterface();
    return animation->playbackRange();
}

void KisAnimationPlayer::setupAudioTracks()
{
    /*ENTER_FUNCTION();
    if (!m_d->canvas || !m_d->canvas->imageView()) {
        return;
    }

    KisDocument* doc = m_d->canvas->imageView()->document();
    if (doc) {
        QVector<QFileInfo> files = doc->getAudioTracks();
        if (doc->getAudioTracks().isEmpty()) {
            return;
        }

        //Only get first file for now and make that a producer...
        QFileInfo toLoad = files.first();
        if (toLoad.exists()) {
            QSharedPointer<Mlt::Producer> producer( new Mlt::Producer(*m_d->mediaConsumer->getProfile(), toLoad.absoluteFilePath().toUtf8().data()));
            m_d->mediaConsumer->setProducer(producer);
        }
    }*/
}

qreal KisAnimationPlayer::playbackSpeed()
{
    Q_UNIMPLEMENTED();
    return 1.0;
}

void KisAnimationPlayer::setPlaybackSpeedPercent(int value)
{
    const float normalizedSpeed = value / 100.0;
    setPlaybackSpeedNormalized(normalizedSpeed);
}

void KisAnimationPlayer::setPlaybackSpeedNormalized(double value)
{
    Q_UNIMPLEMENTED();
    Q_UNUSED(value);
    /*
    if (m_d->playbackEnvironment->playbackSpeed() != value) {
        m_d->playbackEnvironment->setPlaybackSpeed( value );
        emit sigPlaybackSpeedChanged(m_d->playbackEnvironment->playbackSpeed());
    }*/
}

void KisAnimationPlayer::TESTdisplayFrame(int frame)
{
    ENTER_FUNCTION() << ppVar(frame);
    /* NOTE:
     * Because the consumer can have queued frames (in the hopper)
     * it's possible that the consumer will continue running for
     * a brief period after we've requested a stop. In this case,
     * we just ignore them and resynchronize consumer later... */
    if (m_d->state == PLAYING) {
        m_d->displayProxy->displayFrame(frame);
    }
}

void KisAnimationPlayer::setPlaybackState(PlaybackState p_state)
{
    if (m_d->state != p_state) {
        m_d->state = p_state;
        if (m_d->state == PLAYING) {
            //m_d->mediaConsumer->setMode(KisMediaConsumer::PULL);
        } else {
            //m_d->mediaConsumer->setMode(KisMediaConsumer::PUSH);
        }
        emit sigPlaybackStateChanged(m_d->state);
    }
}
