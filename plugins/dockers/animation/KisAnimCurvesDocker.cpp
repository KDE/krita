/*
 *  SPDX-FileCopyrightText: 2016 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QTreeView>
#include <QSplitter>
#include <QToolBar>
#include <QScroller>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QMenu>
#include <QToolButton>

#include "KisAnimCurvesDocker.h"
#include "KisAnimCurvesModel.h"
#include "KisAnimCurvesView.h"
#include "KisAnimCurvesChannelsModel.h"
#include "KisAnimCurvesChannelDelegate.h"

#include "kis_animation_player.h"
#include "kis_keyframe_channel.h"

#include "kis_image_animation_interface.h"
#include "KisAnimUtils.h"
#include "kis_image_config.h"

#include "KisDocument.h"
#include "kis_canvas2.h"
#include "kis_shape_controller.h"
#include "kis_signal_auto_connection.h"
#include "KisViewManager.h"
#include "kis_node_manager.h"
#include "kis_animation_frame_cache.h"
#include "klocalizedstring.h"
#include "kis_icon_utils.h"
#include "kis_action_manager.h"
#include "kis_action.h"
#include "kis_transport_controls.h"
#include "kis_int_parse_spin_box.h"
#include "kis_slider_spin_box.h"
#include "kis_double_parse_spin_box.h"
#include "kis_zoom_button.h"
#include "kis_signals_blocker.h"
#include "kis_time_span.h"

KisAnimCurvesDockerTitlebar::KisAnimCurvesDockerTitlebar(QWidget* parent) :
    KisUtilityTitleBar(new QLabel(i18n("Animation Curves"), parent), parent)
{
    // Transport Controls...
    transport = new KisTransportControls(this);
    widgetAreaLayout->addWidget(transport);
    widgetAreaLayout->addSpacing(SPACING_UNIT);

    // Frame Register...
    sbFrameRegister = new KisIntParseSpinBox(this);
    sbFrameRegister->setToolTip(i18n("Frame register"));
    sbFrameRegister->setPrefix("#  ");
    sbFrameRegister->setRange(0, MAX_FRAMES);
    widgetAreaLayout->addWidget(sbFrameRegister);
    widgetAreaLayout->addSpacing(SPACING_UNIT);

    {   // Drop Frames..
        btnDropFrames = new QToolButton(this);
        widgetAreaLayout->addWidget(btnDropFrames);

        // Playback Speed..
        sbSpeed = new KisSliderSpinBox(this);
        sbSpeed->setRange(25, 200);
        sbSpeed->setSingleStep(5);
        sbSpeed->setValue(100);
        sbSpeed->setPrefix("Speed: ");
        sbSpeed->setSuffix(" %");
        sbSpeed->setToolTip(i18n("Preview playback speed"));
        widgetAreaLayout->addWidget(sbSpeed);
    }

    widgetAreaLayout->addSpacing(SPACING_UNIT);

    {   // Frame ops...
        QWidget *widget = new QWidget(this);
        QHBoxLayout *layout = new QHBoxLayout(widget);
        layout->setSpacing(0);
        layout->setContentsMargins(0,0,0,0);

        // Add/Remove Key..
        btnAddKey = new QToolButton(this);
        btnAddKey->setAutoRaise(true);
        layout->addWidget(btnAddKey);

        btnRemoveKey = new QToolButton(this);
        btnRemoveKey->setAutoRaise(true);
        layout->addWidget(btnRemoveKey);

        layout->addSpacing(SPACING_UNIT);

        // Interpolation Modes..
        btnInterpConstant = new QToolButton(this);
        btnInterpConstant->setAutoRaise(true);
        layout->addWidget(btnInterpConstant);
        btnInterpLinear = new QToolButton(this);
        btnInterpLinear->setAutoRaise(true);
        layout->addWidget(btnInterpLinear);
        btnInterpBezier = new QToolButton(this);
        btnInterpBezier->setAutoRaise(true);
        layout->addWidget(btnInterpBezier);

        layout->addSpacing(SPACING_UNIT);

        // Tangent Modes..
        btnTangentSharp = new QToolButton(this);
        btnTangentSharp->setAutoRaise(true);
        layout->addWidget(btnTangentSharp);
        btnTangentSmooth = new QToolButton(this);
        btnTangentSmooth->setAutoRaise(true);
        layout->addWidget(btnTangentSmooth);

        widgetAreaLayout->addWidget(widget);
    }

    widgetAreaLayout->addSpacing(SPACING_UNIT);

    sbValueRegister = new KisDoubleParseSpinBox(this);
    sbValueRegister->setPrefix("Val: ");
    sbValueRegister->setRange(-99000.f, 99000.f);
    widgetAreaLayout->addWidget(sbValueRegister);

    widgetAreaLayout->addSpacing(SPACING_UNIT);

    // Zoom buttons..
    btnZoomFitRange = new QToolButton(this);
    btnZoomFitRange->setAutoRaise(true);
    widgetAreaLayout->addWidget(btnZoomFitRange);

    btnZoomFitCurve = new QToolButton(this);
    btnZoomFitCurve->setAutoRaise(true);
    widgetAreaLayout->addWidget(btnZoomFitCurve);

    btnZoomHori = new KisZoomButton(this);
    btnZoomHori->setAutoRaise(true);
    btnZoomHori->setIcon(KisIconUtils::loadIcon("zoom-horizontal"));
    btnZoomHori->setIconSize(QSize(20, 20)); // this icon is very small on windows if no explicitly set
    widgetAreaLayout->addWidget(btnZoomHori);

    btnZoomVert = new KisZoomButton(this);
    btnZoomVert->setAutoRaise(true);
    btnZoomVert->setIcon(KisIconUtils::loadIcon("zoom-vertical"));
    btnZoomVert->setIconSize(QSize(20, 20)); // this icon is very small on windows if no explicitly set
    widgetAreaLayout->addWidget(btnZoomVert);

    widgetAreaLayout->addStretch();

    {   // Menus..
        QWidget *widget = new QWidget(this);

        QHBoxLayout *layout = new QHBoxLayout(widget);
        layout->setSpacing(0);
        layout->setContentsMargins(SPACING_UNIT,0,0,0);

        // Onion skins menu.
        btnOnionSkinsMenu = new QPushButton(KisIconUtils::loadIcon("onion_skin_options"), "", this);
        btnOnionSkinsMenu->setToolTip(i18n("Onion skins menu"));
        btnOnionSkinsMenu->setFlat(true);
        layout->addWidget(btnOnionSkinsMenu);

        // Audio menu..
        btnAudioMenu = new QPushButton(KisIconUtils::loadIcon("audio-none"), "", this);
        btnAudioMenu->setToolTip(i18n("Audio menu"));
        btnAudioMenu->setFlat(true);
        btnAudioMenu->hide(); // (NOTE: Hidden for now while audio features develop.)
        layout->addWidget(btnAudioMenu);

        {   // Settings menu..
            btnSettingsMenu = new QToolButton(this);
            btnSettingsMenu->setIcon(KisIconUtils::loadIcon("view-choose"));
            btnSettingsMenu->setToolTip(i18n("Animation settings menu"));
            btnSettingsMenu->setAutoRaise(true);

            QWidget *settingsMenuWidget = new QWidget(this);
            QHBoxLayout *settingsMenuLayout = new QHBoxLayout(settingsMenuWidget);

            QWidget *fields = new QWidget(settingsMenuWidget);
            QFormLayout *fieldsLayout = new QFormLayout(fields);

            sbStartFrame = new KisIntParseSpinBox(settingsMenuWidget);
            sbStartFrame->setMaximum(10000);
            fieldsLayout->addRow(i18n("Clip Start: "), sbStartFrame);

            sbEndFrame = new KisIntParseSpinBox(settingsMenuWidget);
            sbEndFrame->setMaximum(10000);
            fieldsLayout->addRow(i18n("Clip End: "), sbEndFrame);

            sbFrameRate = new KisIntParseSpinBox(settingsMenuWidget);
            sbFrameRate->setMinimum(0);
            sbFrameRate->setMaximum(180);
            fieldsLayout->addRow(i18n("Frame Rate: "), sbFrameRate);

            QWidget *buttons = new QWidget(settingsMenuWidget);
            QVBoxLayout *buttonsLayout = new QVBoxLayout(buttons);
            buttonsLayout->setAlignment(Qt::AlignTop);

            settingsMenuLayout->addWidget(fields);
            settingsMenuLayout->addWidget(buttons);

            layout->addWidget(btnSettingsMenu);

            QMenu *settingsPopMenu = new QMenu(this);
            QWidgetAction *settingsMenuAction = new QWidgetAction(this);
            settingsMenuAction->setDefaultWidget(settingsMenuWidget);
            settingsPopMenu->addAction(settingsMenuAction);

            btnSettingsMenu->setPopupMode(QToolButton::InstantPopup);
            btnSettingsMenu->setMenu(settingsPopMenu);
        }

        widgetAreaLayout->addWidget(widget);
    }
}

struct KisAnimCurvesDocker::Private
{
    Private(QWidget *parent)
        : titlebar(new KisAnimCurvesDockerTitlebar(parent))
        , curvesModel(new KisAnimCurvesModel(parent))
        , mainWindow(nullptr)
    {
        channelTreeModel = new KisAnimCurvesChannelsModel(curvesModel, parent);
    }

    KisAnimCurvesDockerTitlebar *titlebar;

    KisAnimCurvesModel *curvesModel;
    KisAnimCurvesView *curvesView;

    KisAnimCurvesChannelsModel *channelTreeModel;
    QTreeView *channelTreeView;

    KisMainWindow *mainWindow;
    QPointer<KisCanvas2> canvas;
    KisSignalAutoConnectionsStore canvasConnections;
};

KisAnimCurvesDocker::KisAnimCurvesDocker()
    : QDockWidget(i18n("Animation Curves"))
    , m_d(new Private(this))
{
    QWidget *mainWidget = new QWidget(0);
    mainWidget->setLayout(new QVBoxLayout());
    setWidget(mainWidget);

    QSplitter *mainSplitter = new QSplitter(this);
    mainWidget->layout()->addWidget(mainSplitter);

    {   // Channel Tree..
        m_d->channelTreeView = new QTreeView(this);
        m_d->channelTreeView->setModel(m_d->channelTreeModel);
        m_d->channelTreeView->setHeaderHidden(true);
        KisAnimCurvesChannelDelegate *listDelegate = new KisAnimCurvesChannelDelegate(this);
        m_d->channelTreeView->setItemDelegate(listDelegate);
    }

    {   // Curves View..
        m_d->curvesView = new KisAnimCurvesView(this);
        m_d->curvesView->setModel(m_d->curvesModel);
    }

    mainSplitter->addWidget(m_d->channelTreeView);
    mainSplitter->setStretchFactor(0, 1);
    mainSplitter->addWidget(m_d->curvesView);
    mainSplitter->setStretchFactor(1, 10);

    // Kinetic Scrolling..
    QScroller *scroller = KisKineticScroller::createPreconfiguredScroller(m_d->channelTreeView);
    if (scroller){
        connect(scroller, SIGNAL(stateChanged(QScroller::State)),
                this, SLOT(slotScrollerStateChanged(QScroller::State)));
    }

    connect(m_d->channelTreeModel, &KisAnimCurvesChannelsModel::rowsInserted,
            this, &KisAnimCurvesDocker::slotListRowsInserted);

    // Titlebar Widget..
    setTitleBarWidget(m_d->titlebar);

    connect(m_d->titlebar->btnOnionSkinsMenu, &QPushButton::released, [this](){
        if (m_d->mainWindow) {
            QDockWidget *docker = m_d->mainWindow->dockWidget("OnionSkinsDocker");
            if (docker) {
                docker->setVisible(!docker->isVisible());
            }
        }
    });

    connect(m_d->titlebar->btnZoomHori, &KisZoomButton::zoom, [this](qreal zoomDelta){
        if (m_d->curvesView) {
            m_d->curvesView->changeZoom(Qt::Horizontal, zoomDelta);
        }
    });

    connect(m_d->titlebar->btnZoomVert, &KisZoomButton::zoom, [this](qreal zoomDelta){
        if (m_d->curvesView) {
            m_d->curvesView->changeZoom(Qt::Vertical, zoomDelta);
        }
    });

    connect(m_d->curvesView, SIGNAL(activated(QModelIndex)), this, SLOT(slotActiveNodeUpdate(QModelIndex)));
    connect(m_d->curvesView, SIGNAL(activeDataChanged(QModelIndex)), this, SLOT(slotActiveNodeUpdate(QModelIndex)));
    connect(m_d->titlebar->sbValueRegister, SIGNAL(valueChanged(double)), this, SLOT(slotValueRegisterChanged(double)));
}

KisAnimCurvesDocker::~KisAnimCurvesDocker()
{}

void KisAnimCurvesDocker::setCanvas(KoCanvasBase *canvas)
{
    if (canvas && m_d->canvas == canvas) return;

    if (m_d->canvas) {
        m_d->canvasConnections.clear();
        m_d->canvas->disconnectCanvasObserver(this);
        m_d->channelTreeModel->selectedNodesChanged(KisNodeList());
        m_d->canvas->animationPlayer()->disconnect(this);
        m_d->titlebar->transport->disconnect(m_d->canvas->animationPlayer());
        m_d->titlebar->sbFrameRegister->disconnect(m_d->canvas->animationPlayer());
        m_d->titlebar->sbSpeed->disconnect(m_d->canvas->animationPlayer());

        if (m_d->canvas->image()) {
            m_d->canvas->image()->animationInterface()->disconnect(this);
            m_d->titlebar->sbStartFrame->disconnect(m_d->canvas->image()->animationInterface());
            m_d->titlebar->sbEndFrame->disconnect(m_d->canvas->image()->animationInterface());
            m_d->titlebar->sbFrameRate->disconnect(m_d->canvas->image()->animationInterface());
        }
    }

    m_d->canvas = dynamic_cast<KisCanvas2*>(canvas);
    setEnabled(m_d->canvas != 0);

    if (m_d->canvas) {
        KisDocument *doc = static_cast<KisDocument*>(m_d->canvas->imageView()->document());
        KisShapeController *kritaShapeController = dynamic_cast<KisShapeController*>(doc->shapeController());
        m_d->channelTreeModel->setDummiesFacade(kritaShapeController);

        m_d->curvesModel->setImage(m_d->canvas->image());
        m_d->curvesModel->setFrameCache(m_d->canvas->frameCache());
        m_d->curvesModel->setAnimationPlayer(m_d->canvas->animationPlayer());

        m_d->canvasConnections.addConnection(
            m_d->canvas->viewManager()->nodeManager(), SIGNAL(sigUiNeedChangeSelectedNodes(KisNodeList)),
            m_d->channelTreeModel, SLOT(selectedNodesChanged(KisNodeList))
        );

        m_d->canvasConnections.addConnection(
            m_d->canvas->viewManager()->nodeManager(), SIGNAL(sigNodeActivated(KisNodeSP)),
            this, SLOT(slotNodeActivated(KisNodeSP))
        );

        m_d->channelTreeModel->clear();
        m_d->channelTreeModel->selectedNodesChanged(m_d->canvas->viewManager()->nodeManager()->selectedNodes());

        {   // Reinitialize titlebar widgets..
            KisSignalsBlocker blocker(m_d->titlebar->sbStartFrame,
                                      m_d->titlebar->sbEndFrame,
                                      m_d->titlebar->sbFrameRate,
                                      m_d->titlebar->sbSpeed,
                                      m_d->titlebar->sbFrameRegister,
                                      m_d->titlebar->sbValueRegister);

            KisImageAnimationInterface *animinterface = m_d->canvas->image()->animationInterface();
            m_d->titlebar->sbStartFrame->setValue(animinterface->fullClipRange().start());
            m_d->titlebar->sbEndFrame->setValue(animinterface->fullClipRange().end());
            m_d->titlebar->sbFrameRate->setValue(animinterface->framerate());
            m_d->titlebar->sbSpeed->setValue(100);
            m_d->titlebar->sbFrameRegister->setValue(animinterface->currentTime());

            QModelIndex activeIndex = m_d->curvesView->currentIndex();
            m_d->titlebar->sbValueRegister->setEnabled(activeIndex.isValid());
            m_d->titlebar->sbValueRegister->setValue( activeIndex.isValid() ?
                                                      activeIndex.data(KisAnimCurvesModel::ScalarValueRole).toReal() : 0.0f);
        }

        connect(m_d->titlebar->transport, SIGNAL(back()), m_d->canvas->animationPlayer(), SLOT(previousFrame()));
        connect(m_d->titlebar->transport, SIGNAL(stop()), m_d->canvas->animationPlayer(), SLOT(stop()));
        connect(m_d->titlebar->transport, SIGNAL(playPause()), m_d->canvas->animationPlayer(), SLOT(playPause()));
        connect(m_d->titlebar->transport, SIGNAL(forward()), m_d->canvas->animationPlayer(), SLOT(nextFrame()));
        connect(m_d->titlebar->sbFrameRegister, SIGNAL(valueChanged(int)), m_d->canvas->animationPlayer(), SLOT(seek(int)));
        connect(m_d->titlebar->sbSpeed, SIGNAL(valueChanged(int)), m_d->canvas->animationPlayer(), SLOT(setPlaybackSpeedPercent(int)));

        connect(m_d->titlebar->sbFrameRate, SIGNAL(valueChanged(int)), m_d->canvas->image()->animationInterface(), SLOT(setFramerate(int)));
        connect(m_d->titlebar->sbStartFrame, SIGNAL(valueChanged(int)), m_d->canvas->image()->animationInterface(), SLOT(setFullClipRangeStartTime(int)));
        connect(m_d->titlebar->sbEndFrame, SIGNAL(valueChanged(int)), m_d->canvas->image()->animationInterface(), SLOT(setFullClipRangeEndTime(int)));

        connect(m_d->canvas->animationPlayer(), SIGNAL(sigPlaybackStateChanged(bool)), m_d->titlebar->sbFrameRegister, SLOT(setDisabled(bool)));
        connect(m_d->canvas->animationPlayer(), SIGNAL(sigPlaybackStateChanged(bool)), m_d->titlebar->transport, SLOT(setPlaying(bool)));
        connect(m_d->canvas->animationPlayer(), SIGNAL(sigFrameChanged()), this, SLOT(updateFrameRegister()));
        connect(m_d->canvas->animationPlayer(), SIGNAL(sigPlaybackStopped()), this, SLOT(updateFrameRegister()));
        connect(m_d->canvas->animationPlayer(), SIGNAL(sigPlaybackSpeedChanged(double)), this, SLOT(handlePlaybackSpeedChange(double)));

        connect(m_d->canvas->image()->animationInterface(), SIGNAL(sigUiTimeChanged(int)), this, SLOT(updateFrameRegister()));
        connect(m_d->canvas->image()->animationInterface(), SIGNAL(sigFullClipRangeChanged()), SLOT(handleClipRangeChange()));
        connect(m_d->canvas->image()->animationInterface(), SIGNAL(sigFramerateChanged()), this, SLOT(handleFrameRateChange()));
    }
}

void KisAnimCurvesDocker::unsetCanvas()
{
    setCanvas(0);
}

void KisAnimCurvesDocker::setViewManager(KisViewManager *view)
{
    m_d->mainWindow = view->mainWindow();

    connect(view->mainWindow(), SIGNAL(themeChanged()), this, SLOT(slotUpdateIcons()));
    slotUpdateIcons();

    KisActionManager* actionManager = view->actionManager();

    KisAction* action = actionManager->createAction("insert_opacity_keyframe");
    action->setIcon(KisIconUtils::loadIcon("keyframe-add"));
    connect(action, SIGNAL(triggered(bool)),
            this, SLOT(slotAddOpacityKey()));
    m_d->titlebar->btnAddKey->setDefaultAction(action);

    action = actionManager->createAction("remove_opacity_keyframe");
    action->setIcon(KisIconUtils::loadIcon("keyframe-remove"));
    connect(action, SIGNAL(triggered(bool)),
            this, SLOT(slotRemoveOpacityKey()));
    m_d->titlebar->btnRemoveKey->setDefaultAction(action);

    action = actionManager->createAction("interpolation_constant");
    action->setIcon(KisIconUtils::loadIcon("interpolation_constant"));
    action->setToolTip(i18n("Hold constant value. No interpolation."));
    connect(action, &KisAction::triggered,
            m_d->curvesView, &KisAnimCurvesView::applyConstantMode);
    m_d->titlebar->btnInterpConstant->setDefaultAction(action);

    action = actionManager->createAction("interpolation_linear");
    action->setIcon(KisIconUtils::loadIcon("interpolation_linear"));
    action->setToolTip(i18n("Linear interpolation."));
    connect(action, &KisAction::triggered,
            m_d->curvesView, &KisAnimCurvesView::applyLinearMode);
    m_d->titlebar->btnInterpLinear->setDefaultAction(action);

    action = actionManager->createAction("interpolation_bezier");
    action->setIcon(KisIconUtils::loadIcon("interpolation_bezier"));
    action->setToolTip(i18n("Bezier curve interpolation."));
    connect(action, &KisAction::triggered,
            m_d->curvesView, &KisAnimCurvesView::applyBezierMode);
    m_d->titlebar->btnInterpBezier->setDefaultAction(action);

    action = actionManager->createAction("tangents_sharp");
    action->setIcon(KisIconUtils::loadIcon("interpolation_sharp"));
    action->setToolTip(i18n("Sharp interpolation tangents."));
    connect(action, &KisAction::triggered,
            m_d->curvesView, &KisAnimCurvesView::applySharpMode);
    m_d->titlebar->btnTangentSharp->setDefaultAction(action);

    action = actionManager->createAction("tangents_smooth");
    action->setIcon(KisIconUtils::loadIcon("interpolation_smooth"));
    action->setToolTip(i18n("Smooth interpolation tangents."));
    connect(action, &KisAction::triggered,
            m_d->curvesView, &KisAnimCurvesView::applySmoothMode);
    m_d->titlebar->btnTangentSmooth->setDefaultAction(action);

    action = actionManager->createAction("zoom_to_fit_range");
    action->setIcon(KisIconUtils::loadIcon("zoom-fit"));
    action->setToolTip(i18n("Zoom view to fit channel range."));
    connect(action, &KisAction::triggered,
            m_d->curvesView, &KisAnimCurvesView::zoomToFitChannel);
    m_d->titlebar->btnZoomFitRange->setDefaultAction(action);

    action = actionManager->createAction("zoom_to_fit_curve");
    action->setIcon(KisIconUtils::loadIcon("zoom-fit-curve"));
    action->setToolTip(i18n("Zoom view to fit curve."));
    connect(action, &KisAction::triggered,
            m_d->curvesView, &KisAnimCurvesView::zoomToFitCurve);
    m_d->titlebar->btnZoomFitCurve->setDefaultAction(action);

    {
        action = actionManager->createAction("drop_frames");
        m_d->titlebar->btnDropFrames->setDefaultAction(action);
        connect(action, &KisAction::triggered, [](bool dropFrames){
            KisConfig cfg(false);
            if (dropFrames != cfg.animationDropFrames()) {
                cfg.setAnimationDropFrames(dropFrames);
                //updatePlaybackStatistics();
            }
        });

        KisConfig config(true);
        action->setChecked(config.animationDropFrames());
    }
}

void KisAnimCurvesDocker::addKeyframe(const QString &channelIdentity)
{
    if (!m_d->canvas) return;

    KisNodeSP node = m_d->canvas->viewManager()->activeNode();
    if (!node) return;

    const int time = m_d->canvas->image()->animationInterface()->currentTime();

    KisAnimUtils::createKeyframeLazy(m_d->canvas->image(), node, channelIdentity, time, true);
}

void KisAnimCurvesDocker::removeKeyframe(const QString &channel)
{
    if (!m_d->canvas) return;

    KisNodeSP node = m_d->canvas->viewManager()->activeNode();
    if (!node) return;

    QItemSelectionModel* selectionModel = m_d->curvesView->selectionModel();
    QModelIndexList selected = selectionModel ? selectionModel->selectedIndexes() : QModelIndexList();

    if (selected.count() > 0) {
        Q_FOREACH(const QModelIndex& selection, selected) {
            KisAnimUtils::removeKeyframe(m_d->canvas->image(), node, channel, selection.column());
        }
    } else {
        const int time = m_d->canvas->image()->animationInterface()->currentTime();
        KisAnimUtils::removeKeyframe(m_d->canvas->image(), node, channel, time);
    }
}

void KisAnimCurvesDocker::slotScrollerStateChanged(QScroller::State state)
{
    KisKineticScroller::updateCursor(m_d->channelTreeView, state);
}

void KisAnimCurvesDocker::slotNodeActivated(KisNodeSP node)
{
    if (!node) return;
    m_d->titlebar->btnAddKey->setEnabled(node->supportsKeyframeChannel(KisKeyframeChannel::Opacity.id()));
}

void KisAnimCurvesDocker::updateFrameRegister(){
    if (!m_d->canvas && !m_d->canvas->image()) {
        return;
    }

    const int frame = m_d->canvas->animationPlayer()->isPlaying() ?
                m_d->canvas->animationPlayer()->visibleFrame() :
                m_d->canvas->image()->animationInterface()->currentUITime();

    m_d->titlebar->sbFrameRegister->setValue(frame);
}

void KisAnimCurvesDocker::handleFrameRateChange()
{
    if (!m_d->canvas || !m_d->canvas->image()) return;

    KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();
    m_d->titlebar->sbFrameRate->setValue(animInterface->framerate());
}

void KisAnimCurvesDocker::handleClipRangeChange()
{
    if (!m_d->canvas || !m_d->canvas->image()) return;

    KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();

    m_d->titlebar->sbStartFrame->setValue(animInterface->fullClipRange().start());
    m_d->titlebar->sbEndFrame->setValue(animInterface->fullClipRange().end());
}

void KisAnimCurvesDocker::handlePlaybackSpeedChange(double normalizedSpeed)
{
    m_d->titlebar->sbSpeed->setValue(normalizedSpeed * 100);
}

void KisAnimCurvesDocker::slotUpdateIcons()
{
}

void KisAnimCurvesDocker::slotAddAllEnabledKeys()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->canvas && m_d->canvas->viewManager());
    // remember current node's opacity and set it once we create a new opacity keyframe
    KisNodeSP node = m_d->canvas->viewManager()->activeNode();
    KIS_SAFE_ASSERT_RECOVER_RETURN(node);

    /* Once we have more than one supported scalar key value,
     * we should add a dropdown check-box set of actions that can
     * enable and disable keys. For now, since opacity is the only
     * key officially supported, we will just presume opacity. */
    if (node->supportsKeyframeChannel(KisKeyframeChannel::Opacity.id())) {
        addKeyframe(KisKeyframeChannel::Opacity.id());
    }
}

void KisAnimCurvesDocker::slotAddOpacityKey()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->canvas && m_d->canvas->viewManager());

    KisNodeSP node = m_d->canvas->viewManager()->activeNode();
    KIS_SAFE_ASSERT_RECOVER_RETURN(node);

    if (node->supportsKeyframeChannel(KisKeyframeChannel::Opacity.id())) {
        addKeyframe(KisKeyframeChannel::Opacity.id());
    }
}

void KisAnimCurvesDocker::slotRemoveOpacityKey()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->canvas && m_d->canvas->viewManager());

    KisNodeSP node = m_d->canvas->viewManager()->activeNode();
    KIS_SAFE_ASSERT_RECOVER_RETURN(node);
    if (node->supportsKeyframeChannel(KisKeyframeChannel::Opacity.id())) {
        removeKeyframe(KisKeyframeChannel::Opacity.id());
    }
}

void KisAnimCurvesDocker::slotListRowsInserted(const QModelIndex &parentIndex, int first, int last)
{
    // Auto-expand nodes on the tree
    for (int r=first; r<=last; r++) {
        QModelIndex index = m_d->channelTreeModel->index(r, 0, parentIndex);
        m_d->channelTreeView->expand(index);
    }
}

void KisAnimCurvesDocker::slotValueRegisterChanged(double value){
    if (!m_d->curvesModel)
        return;

    QModelIndex current = m_d->curvesView->currentIndex();
    if (current.isValid() && m_d->curvesView->indexHasKey(current)) {
        m_d->curvesModel->setData(current, value, KisAnimCurvesModel::ScalarValueRole);
    }
}

void KisAnimCurvesDocker::slotActiveNodeUpdate(const QModelIndex index)
{
    if (index.isValid() && m_d->curvesView->indexHasKey(index)) {
        QVariant variant = m_d->curvesModel->data(index, KisAnimCurvesModel::ScalarValueRole);
        m_d->titlebar->sbValueRegister->setEnabled(variant.isValid());
        m_d->titlebar->sbValueRegister->setValue(variant.isValid() ? variant.toReal() : 0.0);
    } else {
        m_d->titlebar->sbValueRegister->setEnabled(false);
    }
}
