/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2022 Emmet O'Neill <emmetoneill.pdx@gmail.com>
   SPDX-FileCopyrightText: 2022 Eoin O'Neill <eoinoneill1991@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KisPlaybackEngine.h"

#include "kis_canvas2.h"
#include "KisCanvasAnimationState.h"
#include "kis_image_animation_interface.h"
#include "kis_raster_keyframe_channel.h"
#include "animation/KisFrameDisplayProxy.h"
#include "KisViewManager.h"
#include "kis_config.h"

#include "kis_onion_skin_compositor.h"

struct KisPlaybackEngine::Private {
public:
    Private() {
    }

    ~Private() {
    }

    KisCanvas2* activeCanvas {nullptr};
    bool dropFramesMode {true};
};

//=====

KisPlaybackEngine::KisPlaybackEngine(QObject *parent)
    : QObject(parent)
    , m_d(new Private)
{
    KisConfig cfg(true);
    m_d->dropFramesMode = cfg.animationDropFrames();
}

KisPlaybackEngine::~KisPlaybackEngine()
{
}

void KisPlaybackEngine::play()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(activeCanvas() && activeCanvas()->animationState());
    activeCanvas()->animationState()->setPlaybackState(PLAYING);
}

void KisPlaybackEngine::pause()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(activeCanvas() && activeCanvas()->animationState());
    KisCanvasAnimationState* animationState = activeCanvas()->animationState();

    animationState->setPlaybackState(PAUSED);

    seek(animationState->displayProxy()->activeFrame(), SEEK_FINALIZE);
}

void KisPlaybackEngine::playPause()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(activeCanvas() && activeCanvas()->animationState());
    KisCanvasAnimationState* animationState = activeCanvas()->animationState();

    if (animationState->playbackState() == PLAYING) {
        pause();  
    } else {
        play();
    }
}

void KisPlaybackEngine::stop()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(activeCanvas() && activeCanvas()->animationState());
    KisCanvasAnimationState* animationState = activeCanvas()->animationState();

    if (animationState->playbackState() != STOPPED) {
        const boost::optional<int> origin = animationState->playbackOrigin();
        animationState->setPlaybackState(STOPPED);
        if (origin.has_value()) {
            seek(origin.value(), SEEK_FINALIZE);
        }
    } else if (animationState->displayProxy()->activeFrame() != 0) {
        KisImageAnimationInterface* ai = activeCanvas()->image()->animationInterface();
        KIS_SAFE_ASSERT_RECOVER_RETURN(ai);
        const int firstFrame = ai->documentPlaybackRange().start();
        seek(firstFrame, SEEK_FINALIZE | SEEK_PUSH_AUDIO);
    }
}

void KisPlaybackEngine::previousFrame()
{
    moveActiveFrameBy(-1);
}

void KisPlaybackEngine::nextFrame()
{
    moveActiveFrameBy(1);
}

void KisPlaybackEngine::previousKeyframe()
{
    if (!m_d->activeCanvas) return;
    KisCanvasAnimationState *animationState = m_d->activeCanvas->animationState();
    KIS_SAFE_ASSERT_RECOVER_RETURN(animationState);

    KisNodeSP node = m_d->activeCanvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    int currentFrame = animationState->displayProxy()->activeFrame();

    int destinationTime = -1;
    if (!keyframes->keyframeAt(currentFrame)) {
        destinationTime = keyframes->activeKeyframeTime(currentFrame);
    } else {
        destinationTime = keyframes->previousKeyframeTime(currentFrame);
    }

    if (keyframes->keyframeAt(destinationTime)) {
        if (animationState->playbackState() != STOPPED) {
            stop();
        }

        seek(destinationTime, SEEK_FINALIZE | SEEK_PUSH_AUDIO);
    }
}

void KisPlaybackEngine::nextKeyframe()
{
    if (!m_d->activeCanvas) return;
    KisCanvasAnimationState *animationState = m_d->activeCanvas->animationState();
    KIS_SAFE_ASSERT_RECOVER_RETURN(animationState);

    KisNodeSP node = m_d->activeCanvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    int currentTime = animationState->displayProxy()->activeFrame();

    int destinationTime = -1;
    if (keyframes->activeKeyframeAt(currentTime)) {
        destinationTime = keyframes->nextKeyframeTime(currentTime);
    }

    if (keyframes->keyframeAt(destinationTime)) {
        // Jump to next key...
        if (animationState->playbackState() != STOPPED) {
            stop();
        }

        seek(destinationTime, SEEK_FINALIZE | SEEK_PUSH_AUDIO);
    } else {
        // Jump ahead by estimated timing...
        const int activeKeyTime = keyframes->activeKeyframeTime(currentTime);
        const int previousKeyTime = keyframes->previousKeyframeTime(activeKeyTime);

        if (previousKeyTime != -1) {
            if (animationState->playbackState() != STOPPED) {
                stop();
            }

            const int timing = activeKeyTime - previousKeyTime;
            seek(currentTime + timing, SEEK_FINALIZE | SEEK_PUSH_AUDIO);
        }
    }
}

void KisPlaybackEngine::firstFrame()
{
    if (!m_d->activeCanvas) return;
    KisCanvasAnimationState *animationState = m_d->activeCanvas->animationState();
    KIS_SAFE_ASSERT_RECOVER_RETURN(animationState);
    KisImageAnimationInterface *animInterface = m_d->activeCanvas->image()->animationInterface();

    const int startFrame = animInterface->activePlaybackRange().start();

    if (animationState->playbackState() != STOPPED) {
        stop();
    }

    seek(startFrame, SEEK_FINALIZE | SEEK_PUSH_AUDIO);
}

void KisPlaybackEngine::lastFrame()
{
    if (!m_d->activeCanvas) return;
    KisCanvasAnimationState *animationState = m_d->activeCanvas->animationState();
    KIS_SAFE_ASSERT_RECOVER_RETURN(animationState);
    KisImageAnimationInterface *animInterface = m_d->activeCanvas->image()->animationInterface();

    const int endFrame = animInterface->activePlaybackRange().end();

    if (animationState->playbackState() != STOPPED) {
        stop();
    }

    seek(endFrame, SEEK_FINALIZE | SEEK_PUSH_AUDIO);
}

void KisPlaybackEngine::previousMatchingKeyframe()
{
    if (!m_d->activeCanvas) return;
    KisCanvasAnimationState *animationState = m_d->activeCanvas->animationState();
    KIS_SAFE_ASSERT_RECOVER_RETURN(animationState);

    KisNodeSP node = m_d->activeCanvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    int time = animationState->displayProxy()->activeFrame();

    KisKeyframeSP currentKeyframe = keyframes->keyframeAt(time);
    int destinationTime = keyframes->activeKeyframeTime(time);
    const int desiredColor = currentKeyframe ? currentKeyframe->colorLabel() : keyframes->keyframeAt(destinationTime)->colorLabel();
    previousKeyframeWithColor(desiredColor);
}

void KisPlaybackEngine::nextMatchingKeyframe()
{
    if (!m_d->activeCanvas) return;
    KisCanvasAnimationState *animationState = m_d->activeCanvas->animationState();
    KIS_SAFE_ASSERT_RECOVER_RETURN(animationState);

    KisNodeSP node = m_d->activeCanvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    int time = animationState->displayProxy()->activeFrame();

    if (!keyframes->activeKeyframeAt(time)) {
        return;
    }

    int destinationTime = keyframes->activeKeyframeTime(time);
    nextKeyframeWithColor(keyframes->keyframeAt(destinationTime)->colorLabel());
}

void KisPlaybackEngine::previousUnfilteredKeyframe()
{
    previousKeyframeWithColor(KisOnionSkinCompositor::instance()->colorLabelFilter());
}

void KisPlaybackEngine::nextUnfilteredKeyframe()
{
    nextKeyframeWithColor(KisOnionSkinCompositor::instance()->colorLabelFilter());
}

void KisPlaybackEngine::setDropFramesMode(bool value)
{
    if (value != m_d->dropFramesMode) {
        m_d->dropFramesMode = value;
        Q_EMIT sigDropFramesModeChanged(value);
    }
}

bool KisPlaybackEngine::dropFrames() const
{
    return m_d->dropFramesMode;
}

int KisPlaybackEngine::frameWrap(int frame, int startFrame, int endFrame)
{
    // Since Krita has always considered the end frame as inclusive, we need
    // to make sure our wrap method respects that as well.
    const int inclusiveEndFrame = endFrame + 1;
    frame = ((frame - startFrame) % (inclusiveEndFrame - startFrame)) + startFrame;

    if (frame - startFrame < 0) {
        frame += (inclusiveEndFrame - startFrame);
    }

    return frame;
}

KisCanvas2 *KisPlaybackEngine::activeCanvas() const
{
    return m_d->activeCanvas;
}

void KisPlaybackEngine::setCanvas(KoCanvasBase *p_canvas)
{
    if (m_d->activeCanvas) {
        KisCanvasAnimationState* animState = m_d->activeCanvas->animationState();
        KIS_SAFE_ASSERT_RECOVER_RETURN(animState);

        animState->disconnect(this);
    }

    KisCanvas2* canvas = dynamic_cast<KisCanvas2*>(p_canvas);
    m_d->activeCanvas = canvas;

    if (m_d->activeCanvas) {
        KisCanvasAnimationState* animState = m_d->activeCanvas->animationState();
        KIS_SAFE_ASSERT_RECOVER_RETURN(animState);

        connect(animState, &KisCanvasAnimationState::sigCancelPlayback,
                this, &KisPlaybackEngine::stop);

        /**
         * TODO: This forced updates causes image recalculation on every document
         * switch, which is weird and even causes some crashes on closing
         * many documents at once (which is a separate bug it seems). Why
         * document switch should forcefully regeneare the canvas?
         */
//        KisImageAnimationInterface* animInterface = m_d->activeCanvas->image()->animationInterface();
//        KisFrameDisplayProxy* displayProxy = animState->displayProxy();
//        if (animState->playbackState() != PLAYING) {
//            displayProxy->displayFrame(animInterface->currentTime(), false);
//        }
    }
}

void KisPlaybackEngine::unsetCanvas()
{
    m_d->activeCanvas = nullptr;
}

void KisPlaybackEngine::moveActiveFrameBy(int frames)
{
    if (!m_d->activeCanvas) return;
    KisCanvasAnimationState *animationState = m_d->activeCanvas->animationState();
    KIS_SAFE_ASSERT_RECOVER_RETURN(animationState);
    KisImageAnimationInterface *animInterface = m_d->activeCanvas->image()->animationInterface();

    int frame = animationState->displayProxy()->activeFrame() + frames;

    const int startFrame = animInterface->activePlaybackRange().start();
    const int endFrame = animInterface->activePlaybackRange().end();

    frame = frameWrap(frame, startFrame, endFrame);

    KIS_SAFE_ASSERT_RECOVER_RETURN(frame >= 0);

    if (animationState->playbackState() != STOPPED) {
        stop();
    }

    seek(frame, SEEK_FINALIZE | SEEK_PUSH_AUDIO);
}

void KisPlaybackEngine::nextKeyframeWithColor(int color)
{
    QSet<int> validColors;
    validColors.insert(color);
    nextKeyframeWithColor(validColors);
}

void KisPlaybackEngine::nextKeyframeWithColor(const QSet<int> &validColors)
{
    if (!m_d->activeCanvas) return;
    KisCanvasAnimationState *animationState = m_d->activeCanvas->animationState();
    KIS_SAFE_ASSERT_RECOVER_RETURN(animationState);

    KisNodeSP node = m_d->activeCanvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    int time = animationState->displayProxy()->activeFrame();

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
        if (animationState->playbackState() != STOPPED) {
            stop();
        }

        seek(destinationTime, SEEK_FINALIZE | SEEK_PUSH_AUDIO);
    }
}

void KisPlaybackEngine::previousKeyframeWithColor(int color)
{
    QSet<int> validColors;
    validColors.insert(color);
    previousKeyframeWithColor(validColors);
}

void KisPlaybackEngine::previousKeyframeWithColor(const QSet<int> &validColors)
{
    if (!m_d->activeCanvas) return;
    KisCanvasAnimationState *animationState = m_d->activeCanvas->animationState();
    KIS_SAFE_ASSERT_RECOVER_RETURN(animationState);

    KisNodeSP node = m_d->activeCanvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    int time = animationState->displayProxy()->activeFrame();

    int destinationTime = keyframes->activeKeyframeTime(time);
    while (keyframes->keyframeAt(destinationTime) &&
           ((destinationTime == time) ||
           !validColors.contains(keyframes->keyframeAt(destinationTime)->colorLabel()))) {

        destinationTime = keyframes->previousKeyframeTime(destinationTime);
    }

    if (keyframes->keyframeAt(destinationTime)) {
        if (animationState->playbackState() != STOPPED) {
            stop();
        }

        seek(destinationTime, SEEK_FINALIZE | SEEK_PUSH_AUDIO);
    }
}
