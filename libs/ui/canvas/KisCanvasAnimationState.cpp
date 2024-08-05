/*
 *  SPDX-FileCopyrightText: 2015 Jouni Pentikäinen <joupent@gmail.com>
 *  SPDX-FileCopyrightText: 2021 Eoin O'Neill <eoinoneill1991@gmail.com>
 *  SPDX-FileCopyrightText: 2021 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisCanvasAnimationState.h"

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
#include <QThread>
#include "kis_signal_compressor_with_param.h"
#include "KisImageBarrierLock.h"
#include "kis_layer_utils.h"
#include "KisDecoratedNodeInterface.h"
#include "kis_keyframe_channel.h"
#include "kis_algebra_2d.h"
#include "KisPlaybackEngine.h"

#include "kis_image_config.h"
#include <limits>

#include "KisViewManager.h"
#include "kis_icon_utils.h"

#include "KisPart.h"
#include "dialogs/KisAsyncAnimationCacheRenderDialog.h"
#include "KisRollingMeanAccumulatorWrapper.h"
#include "kis_onion_skin_compositor.h"

#include <atomic>

class SingleShotSignal : public QObject {
    Q_OBJECT
public:
    SingleShotSignal(QObject* parent = nullptr)
        : QObject(parent)
        , lock(false)
    {
    }

    ~SingleShotSignal() {}

public Q_SLOTS:
    void tryFire() {
        if (!lock) {
            lock = true;
            Q_EMIT output();
        }
    }

Q_SIGNALS:
    void output();

private:
    bool lock;

};


//=====

/** @brief PlaybackEnvironment (Private)
 * Constructs and deconstructs the necessary viewing conditions when animation playback begins and ends.
 * This includes disabling and enabling onion skins based on playback condition and other such tasks.
 * Also keeps track of original origin frame of initial play command, so play/pause can work while stop
 * will always return to the origin of playback (where the user first pressed play from the stopped state.)
 */
class CanvasPlaybackEnvironment : public QObject {
    Q_OBJECT
public:
    CanvasPlaybackEnvironment(int originFrame, KisCanvasAnimationState* parent = nullptr)
        : QObject(parent)
        , m_originFrame(originFrame)
    {
        connect(&m_cancelTrigger, SIGNAL(output()), parent, SIGNAL(sigCancelPlayback()));
    }

    ~CanvasPlaybackEnvironment() {
        restore();
    }

    CanvasPlaybackEnvironment() = delete;
    CanvasPlaybackEnvironment(const CanvasPlaybackEnvironment&) = delete;
    CanvasPlaybackEnvironment& operator= (const CanvasPlaybackEnvironment&) = delete;

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

        const KisTimeSpan range = canvas->image()->animationInterface()->activePlaybackRange();
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
            KisImageBarrierLock lock(canvas->image());
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
                KisImageBarrierLock lock(m_canvas->image());
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
    void sigPlaybackStatisticsUpdated();

private:
    int m_originFrame; //!< The frame user started playback from.
    KisSignalAutoConnectionsStore m_cancelStrokeConnections;
    SingleShotSignal m_cancelTrigger;
    QVector<KisNodeWSP> m_disabledDecoratedNodes;

    KisCanvas2* m_canvas;

    KisTimeSpan m_playbackRange;
};

// Needed for QObject definition outside of header file.
#include "KisCanvasAnimationState.moc"

struct KisCanvasAnimationState::Private
{
public:
    Private(KisCanvas2* p_canvas)
        : canvas(p_canvas)
        , displayProxy( new KisFrameDisplayProxy(p_canvas) )
        , playbackEnvironment( nullptr )
    {
        m_statsTimer.setInterval(1000);
    }

    KisCanvas2 *canvas;
    PlaybackState state;
    QScopedPointer<KisFrameDisplayProxy> displayProxy;
    QScopedPointer<QFileInfo> media; // TODO: Should we just get this from the document instead?
    QScopedPointer<CanvasPlaybackEnvironment> playbackEnvironment;

    QTimer m_statsTimer;

    qreal playbackSpeed {1.0};
};

KisCanvasAnimationState::KisCanvasAnimationState(KisCanvas2 *canvas)
    : QObject(canvas)
    , m_d(new Private(canvas))
{
    setPlaybackState(STOPPED);

    // Handle image-internal frame change case...
    connect(m_d->displayProxy.data(), SIGNAL(sigFrameChange()), this, SIGNAL(sigFrameChanged()));

    // Grow to new playback range when new frames added (configurable)...
    connect(m_d->canvas->image()->animationInterface(), &KisImageAnimationInterface::sigKeyframeAdded, this, [this](const KisKeyframeChannel*, int time){
        if (m_d->canvas && m_d->canvas->image()) {
            KisImageAnimationInterface* animInterface = m_d->canvas->image()->animationInterface();
            KisConfig cfg(true);
            if (animInterface && cfg.adaptivePlaybackRange()) {
                KisTimeSpan desiredPlaybackRange = animInterface->documentPlaybackRange();
                desiredPlaybackRange.include(time);
                animInterface->setDocumentRange(desiredPlaybackRange);
            }
        }
    });

    connect(m_d->canvas->imageView()->document(), &KisDocument::sigAudioTracksChanged, this, &KisCanvasAnimationState::setupAudioTracks);
    connect(m_d->canvas->imageView()->document(), &KisDocument::sigAudioLevelChanged, this, &KisCanvasAnimationState::sigAudioLevelChanged);
    connect(&m_d->m_statsTimer, SIGNAL(timeout()), this, SIGNAL(sigPlaybackStatisticsUpdated()));
}

KisCanvasAnimationState::~KisCanvasAnimationState()
{
}

PlaybackState KisCanvasAnimationState::playbackState()
{
    return m_d->state;
}

boost::optional<QFileInfo> KisCanvasAnimationState::mediaInfo()
{
    if (m_d->media) {
        return boost::optional<QFileInfo>(*m_d->media);
    } else {
        return boost::none;
    }
}

qreal KisCanvasAnimationState::currentVolume()
{
    if (m_d->canvas && m_d->canvas->imageView() && m_d->canvas->imageView()->document()) {
        return m_d->canvas->imageView()->document()->getAudioLevel();
    } else {
        return 0.0;
    }
}

boost::optional<int> KisCanvasAnimationState::playbackOrigin()
{
    if (m_d->playbackEnvironment) {
        return boost::optional<int>(m_d->playbackEnvironment->originFrame());
    } else {
        return boost::none;
    }
}

KisFrameDisplayProxy *KisCanvasAnimationState::displayProxy()
{
    return m_d->displayProxy.data();
}

void KisCanvasAnimationState::setPlaybackSpeed(qreal value)
{
    if (!qFuzzyCompare(value, m_d->playbackSpeed)) {
        m_d->playbackSpeed = value;
        Q_EMIT sigPlaybackSpeedChanged(value);
    }
}

qreal KisCanvasAnimationState::playbackSpeed() const
{
    return m_d->playbackSpeed;
}

void KisCanvasAnimationState::setupAudioTracks()
{
    if (!m_d->canvas || !m_d->canvas->imageView()) {
        return;
    }

    KisDocument* doc = m_d->canvas->imageView()->document();
    if (doc) {
        QVector<QFileInfo> files = doc->getAudioTracks();

        if (doc->getAudioTracks().isEmpty()) {
            m_d->media.reset();
        } else {
            //Only get first file for now and make that a producer...
            QFileInfo toLoad = files.first();
            KIS_SAFE_ASSERT_RECOVER_RETURN(toLoad.exists());
            m_d->media.reset(new QFileInfo(toLoad));

            // Once media is attached we upgrade to the MLT-based playbackEngine...
            KisPart::instance()->upgradeToPlaybackEngineMLT(m_d->canvas);
        }

        Q_EMIT sigPlaybackMediaChanged();
    }
}

void KisCanvasAnimationState::showFrame(int frame, bool finalize)
{
    m_d->displayProxy->displayFrame(frame, finalize);
}

KisTimeSpan KisCanvasAnimationState::activePlaybackRange()
{
    if (!m_d->canvas || !m_d->canvas->image()) {
        return KisTimeSpan::infinite(0);
    }

    const KisImageAnimationInterface *animation = m_d->canvas->image()->animationInterface();
    return animation->activePlaybackRange();
}

void KisCanvasAnimationState::setPlaybackState(PlaybackState p_state)
{
    if (m_d->state != p_state) {
        m_d->state = p_state;
        if (m_d->state == PLAYING) {
            if (!m_d->playbackEnvironment) {
                m_d->playbackEnvironment.reset(new CanvasPlaybackEnvironment(m_d->displayProxy->activeFrame(), this));
                connect(m_d->playbackEnvironment.data(), SIGNAL(sigPlaybackStatisticsUpdated()),
                        this, SIGNAL(sigPlaybackStatisticsUpdated()));
            }

            m_d->playbackEnvironment->prepare(m_d->canvas);

            m_d->m_statsTimer.start();
            Q_EMIT sigPlaybackStatisticsUpdated();

        } else {
            if (m_d->playbackEnvironment) {
                m_d->playbackEnvironment->restore();
            }

            if (m_d->state == STOPPED) {
                m_d->playbackEnvironment.reset();
            }

            m_d->m_statsTimer.stop();
            Q_EMIT sigPlaybackStatisticsUpdated();
        }

        Q_EMIT sigPlaybackStateChanged(m_d->state);
    }
}
