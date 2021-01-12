/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisAnimTimelineFramesView.h"

#include "KisAnimTimelineFramesModel.h"
#include "KisAnimTimelineTimeHeader.h"
#include "KisAnimTimelineLayersHeader.h"
#include "timeline_insert_keyframe_dialog.h"
#include "KisAnimTimelineFrameDelegate.h"

#include <QPainter>
#include <QApplication>
#include <QDropEvent>
#include <QMenu>
#include <QScrollBar>
#include <QScroller>
#include <QDrag>
#include <QKeySequence>
#include <QInputDialog>
#include <QClipboard>
#include <QMimeData>
#include <QLayout>
#include <QScreen>
#include "config-qtmultimedia.h"

#include "KSharedConfig"
#include "KisKineticScroller.h"

#include "kis_zoom_button.h"
#include "kis_icon_utils.h"
#include "KisAnimUtils.h"
#include "kis_custom_modifiers_catcher.h"
#include "kis_action.h"
#include "kis_signal_compressor.h"
#include "kis_time_span.h"
#include "kis_color_label_selector_widget.h"
#include "kis_layer_filter_widget.h"
#include "kis_keyframe_channel.h"
#include "kis_slider_spin_box.h"
#include "kis_signals_blocker.h"
#include "kis_image_config.h"
#include "kis_zoom_scrollbar.h"
#include "KisImportExportManager.h"
#include "KoFileDialog.h"
#include "KisIconToolTip.h"

typedef QPair<QRect, QModelIndex> QItemViewPaintPair;
typedef QList<QItemViewPaintPair> QItemViewPaintPairs;

void resizeToMinimalSize(QAbstractButton *w, int minimalSize);
inline bool isIndexDragEnabled(QAbstractItemModel *model, const QModelIndex &index);

struct KisAnimTimelineFramesView::Private
{
    Private(KisAnimTimelineFramesView *_q)
        : q(_q)
        , fps(1)
        , dragInProgress(false)
        , dragWasSuccessful(false)
        , modifiersCatcher(0)
        , kineticScrollInfiniteFrameUpdater()
        , selectionChangedCompressor(300, KisSignalCompressor::FIRST_INACTIVE)
    {
        kineticScrollInfiniteFrameUpdater.setTimerType(Qt::CoarseTimer);
    }

    KisAnimTimelineFramesView *q;
    KisAnimTimelineFramesModel *model;

    KisAnimTimelineTimeHeader *horizontalRuler;
    KisAnimTimelineLayersHeader *layersHeader;

    int fps;
    QPoint initialDragPanValue;
    QPoint initialDragPanPos;

    QToolButton *addLayersButton;
    KisAction *pinLayerToTimelineAction;

    QToolButton *audioOptionsButton;

    KisColorLabelSelectorWidget *colorSelector;
    QWidgetAction *colorSelectorAction;
    KisColorLabelSelectorWidget *multiframeColorSelector;
    QWidgetAction *multiframeColorSelectorAction;

    QMenu *audioOptionsMenu;
    QAction *openAudioAction;
    QAction *audioMuteAction;
    KisSliderSpinBox *volumeSlider;

    QMenu *layerEditingMenu;
    QMenu *existingLayersMenu;

    TimelineInsertKeyframeDialog *insertKeyframeDialog;

    KisZoomButton *zoomDragButton;

    bool dragInProgress;
    bool dragWasSuccessful;

    KisCustomModifiersCatcher *modifiersCatcher;
    QPoint lastPressedPosition;
    Qt::KeyboardModifiers lastPressedModifier;

    QTimer kineticScrollInfiniteFrameUpdater;

    KisSignalCompressor selectionChangedCompressor;

    QStyleOptionViewItem viewOptionsV4() const;
    QItemViewPaintPairs draggablePaintPairs(const QModelIndexList &indexes, QRect *r) const;
    QPixmap renderToPixmap(const QModelIndexList &indexes, QRect *r) const;

    KisIconToolTip tip;

    KisActionManager *actionMan = 0;
};

KisAnimTimelineFramesView::KisAnimTimelineFramesView(QWidget *parent)
    : QTableView(parent),
      m_d(new Private(this))
{
    m_d->modifiersCatcher = new KisCustomModifiersCatcher(this);
    m_d->modifiersCatcher->addModifier("pan-zoom", Qt::Key_Space);
    m_d->modifiersCatcher->addModifier("offset-frame", Qt::Key_Shift);

    setCornerButtonEnabled(false);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    setItemDelegate(new KisAnimTimelineFrameDelegate(this));

    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDefaultDropAction(Qt::MoveAction);

    m_d->horizontalRuler = new KisAnimTimelineTimeHeader(this);
    this->setHorizontalHeader(m_d->horizontalRuler);

    connect(m_d->horizontalRuler, SIGNAL(sigInsertColumnLeft()), SLOT(slotInsertKeyframeColumnLeft()));
    connect(m_d->horizontalRuler, SIGNAL(sigInsertColumnRight()), SLOT(slotInsertKeyframeColumnRight()));

    connect(m_d->horizontalRuler, SIGNAL(sigInsertMultipleColumns()), SLOT(slotInsertMultipleKeyframeColumns()));

    connect(m_d->horizontalRuler, SIGNAL(sigRemoveColumns()), SLOT(slotRemoveSelectedColumns()));
    connect(m_d->horizontalRuler, SIGNAL(sigRemoveColumnsAndShift()), SLOT(slotRemoveSelectedColumnsAndShift()));

    connect(m_d->horizontalRuler, SIGNAL(sigInsertHoldColumns()), SLOT(slotInsertHoldFrameColumn()));
    connect(m_d->horizontalRuler, SIGNAL(sigRemoveHoldColumns()), SLOT(slotRemoveHoldFrameColumn()));

    connect(m_d->horizontalRuler, SIGNAL(sigInsertHoldColumnsCustom()), SLOT(slotInsertMultipleHoldFrameColumns()));
    connect(m_d->horizontalRuler, SIGNAL(sigRemoveHoldColumnsCustom()), SLOT(slotRemoveMultipleHoldFrameColumns()));

    connect(m_d->horizontalRuler, SIGNAL(sigMirrorColumns()), SLOT(slotMirrorColumns()));

    connect(m_d->horizontalRuler, SIGNAL(sigCopyColumns()), SLOT(slotCopyColumns()));
    connect(m_d->horizontalRuler, SIGNAL(sigCutColumns()), SLOT(slotCutColumns()));
    connect(m_d->horizontalRuler, SIGNAL(sigPasteColumns()), SLOT(slotPasteColumns()));

    m_d->layersHeader = new KisAnimTimelineLayersHeader(this);

    m_d->layersHeader->setSectionResizeMode(QHeaderView::Fixed);

    m_d->layersHeader->setDefaultSectionSize(24);
    m_d->layersHeader->setMinimumWidth(60);
    m_d->layersHeader->setHighlightSections(true);
    this->setVerticalHeader(m_d->layersHeader);

    /********** Layer Menu ***********************************************************/

    m_d->layerEditingMenu = new QMenu(this);
    m_d->layerEditingMenu->addSection(i18n("Edit Layers:"));
    m_d->layerEditingMenu->addSeparator();

    m_d->layerEditingMenu->addAction(KisAnimUtils::newLayerActionName, this, SLOT(slotAddNewLayer()));
    m_d->layerEditingMenu->addAction(KisAnimUtils::removeLayerActionName, this, SLOT(slotRemoveLayer()));
    m_d->layerEditingMenu->addSeparator();
    m_d->existingLayersMenu = m_d->layerEditingMenu->addMenu(KisAnimUtils::pinExistingLayerActionName);

    connect(m_d->existingLayersMenu, SIGNAL(aboutToShow()), SLOT(slotUpdateLayersMenu()));
    connect(m_d->existingLayersMenu, SIGNAL(triggered(QAction*)), SLOT(slotAddExistingLayer(QAction*)));

    connect(m_d->layersHeader, SIGNAL(sigRequestContextMenu(QPoint)), SLOT(slotLayerContextMenuRequested(QPoint)));

    m_d->addLayersButton = new QToolButton(this);
    m_d->addLayersButton->setAutoRaise(true);
    m_d->addLayersButton->setIcon(KisIconUtils::loadIcon("addlayer"));
    m_d->addLayersButton->setIconSize(QSize(20, 20));
    m_d->addLayersButton->setPopupMode(QToolButton::InstantPopup);
    m_d->addLayersButton->setMenu(m_d->layerEditingMenu);

    /********** Audio Channel Menu *******************************************************/

    m_d->audioOptionsButton = new QToolButton(this);
    m_d->audioOptionsButton->setAutoRaise(true);
    m_d->audioOptionsButton->setIcon(KisIconUtils::loadIcon("audio-none"));
    m_d->audioOptionsButton->setIconSize(QSize(20, 20)); // very small on windows if not explicitly set
    m_d->audioOptionsButton->setPopupMode(QToolButton::InstantPopup);

    m_d->audioOptionsMenu = new QMenu(this);
    m_d->audioOptionsMenu->addSection(i18n("Edit Audio:"));
    m_d->audioOptionsMenu->addSeparator();

#ifndef HAVE_QT_MULTIMEDIA
    m_d->audioOptionsMenu->addSection(i18nc("@item:inmenu", "Audio playback is not supported in this build!"));
#endif

    m_d->openAudioAction= new QAction("XXX", this);
    connect(m_d->openAudioAction, SIGNAL(triggered()), this, SLOT(slotSelectAudioChannelFile()));
    m_d->audioOptionsMenu->addAction(m_d->openAudioAction);

    m_d->audioMuteAction = new QAction(i18nc("@item:inmenu", "Mute"), this);
    m_d->audioMuteAction->setCheckable(true);
    connect(m_d->audioMuteAction, SIGNAL(triggered(bool)), SLOT(slotAudioChannelMute(bool)));

    m_d->audioOptionsMenu->addAction(m_d->audioMuteAction);
    m_d->audioOptionsMenu->addAction(i18nc("@item:inmenu", "Remove audio"), this, SLOT(slotAudioChannelRemove()));

    m_d->audioOptionsMenu->addSeparator();

    m_d->volumeSlider = new KisSliderSpinBox(this);
    m_d->volumeSlider->setRange(0, 100);
    m_d->volumeSlider->setSuffix(i18n("%"));
    m_d->volumeSlider->setPrefix(i18nc("@item:inmenu, slider", "Volume:"));
    m_d->volumeSlider->setSingleStep(1);
    m_d->volumeSlider->setPageStep(10);
    m_d->volumeSlider->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);

    connect(m_d->volumeSlider, SIGNAL(valueChanged(int)), SLOT(slotAudioVolumeChanged(int)));

    QWidgetAction *volumeAction = new QWidgetAction(m_d->audioOptionsMenu);
    volumeAction->setDefaultWidget(m_d->volumeSlider);
    m_d->audioOptionsMenu->addAction(volumeAction);

    m_d->audioOptionsButton->setMenu(m_d->audioOptionsMenu);

    /********** Frame Editing Context Menu ***********************************************/

    m_d->colorSelector = new KisColorLabelSelectorWidget(this);
    MouseClickIgnore* clickIgnore = new MouseClickIgnore(this);
    m_d->colorSelector->installEventFilter(clickIgnore);
    m_d->colorSelectorAction = new QWidgetAction(this);
    m_d->colorSelectorAction->setDefaultWidget(m_d->colorSelector);
    connect(m_d->colorSelector, &KisColorLabelSelectorWidget::currentIndexChanged, this, &KisAnimTimelineFramesView::slotColorLabelChanged);

    m_d->multiframeColorSelector = new KisColorLabelSelectorWidget(this);
    m_d->multiframeColorSelector->installEventFilter(clickIgnore);
    m_d->multiframeColorSelectorAction = new QWidgetAction(this);
    m_d->multiframeColorSelectorAction->setDefaultWidget(m_d->multiframeColorSelector);
    connect(m_d->multiframeColorSelector, &KisColorLabelSelectorWidget::currentIndexChanged, this, &KisAnimTimelineFramesView::slotColorLabelChanged);

    /********** Insert Keyframes Dialog **************************************************/

    m_d->insertKeyframeDialog = new TimelineInsertKeyframeDialog(this);

    /********** Zoom Button **************************************************************/

    m_d->zoomDragButton = new KisZoomButton(this);
    m_d->zoomDragButton->setAutoRaise(true);
    m_d->zoomDragButton->setIcon(KisIconUtils::loadIcon("zoom-horizontal"));
    m_d->zoomDragButton->setIconSize(QSize(20, 20)); // this icon is very small on windows if no explicitly set

    m_d->zoomDragButton->setToolTip(i18nc("@info:tooltip", "Zoom Timeline. Hold down and drag left or right."));
    m_d->zoomDragButton->setPopupMode(QToolButton::InstantPopup);
    connect(m_d->zoomDragButton, SIGNAL(zoom(qreal)), SLOT(slotZoom(qreal)));

    /********** Zoom Scrollbar **************************************************************/

    KisZoomableScrollBar* hZoomableBar = new KisZoomableScrollBar(this);
    setHorizontalScrollBar(hZoomableBar);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBar(new KisZoomableScrollBar(this));
    hZoomableBar->setEnabled(false);

    connect(hZoomableBar, &KisZoomableScrollBar::valueChanged, m_d->horizontalRuler, &KisAnimTimelineTimeHeader::setPixelOffset);
    connect(hZoomableBar, SIGNAL(zoom(qreal)), this, SLOT(slotZoom(qreal)));
    connect(hZoomableBar, SIGNAL(overscroll(qreal)), SLOT(slotUpdateInfiniteFramesCount()));
    connect(hZoomableBar, SIGNAL(sliderReleased()), SLOT(slotUpdateInfiniteFramesCount()));

    /********** Kinetic Scrolling **************************************************************/

    {
        QScroller *scroller = KisKineticScroller::createPreconfiguredScroller(this);
        if (scroller) {
            connect(scroller, SIGNAL(stateChanged(QScroller::State)),
                    this, SLOT(slotScrollerStateChanged(QScroller::State)));

            connect(&m_d->kineticScrollInfiniteFrameUpdater, &QTimer::timeout, [this, scroller](){
                slotUpdateInfiniteFramesCount();
                scroller->resendPrepareEvent();
            });

            QScrollerProperties props = scroller->scrollerProperties();
            props.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, QScrollerProperties::OvershootAlwaysOff);
            props.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy, QScrollerProperties::OvershootAlwaysOff);
            scroller->setScrollerProperties(props);
        }
    }

    connect(&m_d->selectionChangedCompressor, SIGNAL(timeout()),
            SLOT(slotSelectionChanged()));
    connect(&m_d->selectionChangedCompressor, SIGNAL(timeout()),
            SLOT(slotUpdateFrameActions()));

    {
        QClipboard *cb = QApplication::clipboard();
        connect(cb, SIGNAL(dataChanged()), SLOT(slotUpdateFrameActions()));
    }

    setFramesPerSecond(12);
}

KisAnimTimelineFramesView::~KisAnimTimelineFramesView()
{}

void KisAnimTimelineFramesView::setModel(QAbstractItemModel *model)
{
    KisAnimTimelineFramesModel *framesModel = qobject_cast<KisAnimTimelineFramesModel*>(model);
    m_d->model = framesModel;

    QTableView::setModel(model);

    connect(m_d->model, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
            this, SLOT(slotHeaderDataChanged(Qt::Orientation,int,int)));

    connect(m_d->model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(slotDataChanged(QModelIndex,QModelIndex)));

    connect(m_d->model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(slotReselectCurrentIndex()));

    connect(m_d->model, SIGNAL(sigInfiniteTimelineUpdateNeeded()),
            this, SLOT(slotUpdateInfiniteFramesCount()));

    connect(m_d->model, SIGNAL(sigAudioChannelChanged()),
            this, SLOT(slotUpdateAudioActions()));

    connect(selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            &m_d->selectionChangedCompressor, SLOT(start()));

    connect(m_d->model, SIGNAL(sigEnsureRowVisible(int)), SLOT(slotEnsureRowVisible(int)));
    slotUpdateAudioActions();
}

void KisAnimTimelineFramesView::setActionManager(KisActionManager *actionManager)
{
    m_d->actionMan = actionManager;
    m_d->horizontalRuler->setActionManager(actionManager);

    if (actionManager) {
        KisAction *action = 0;

        action = m_d->actionMan->createAction("add_blank_frame");
        connect(action, SIGNAL(triggered()), SLOT(slotAddBlankFrame()));

        action = m_d->actionMan->createAction("add_duplicate_frame");
        connect(action, SIGNAL(triggered()), SLOT(slotAddDuplicateFrame()));

        action = m_d->actionMan->createAction("insert_keyframe_left");
        connect(action, SIGNAL(triggered()), SLOT(slotInsertKeyframeLeft()));

        action = m_d->actionMan->createAction("insert_keyframe_right");
        connect(action, SIGNAL(triggered()), SLOT(slotInsertKeyframeRight()));

        action = m_d->actionMan->createAction("insert_multiple_keyframes");
        connect(action, SIGNAL(triggered()), SLOT(slotInsertMultipleKeyframes()));

        action = m_d->actionMan->createAction("remove_frames_and_pull");
        connect(action, SIGNAL(triggered()), SLOT(slotRemoveSelectedFramesAndShift()));

        action = m_d->actionMan->createAction("remove_frames");
        connect(action, SIGNAL(triggered()), SLOT(slotRemoveSelectedFrames()));

        action = m_d->actionMan->createAction("insert_hold_frame");
        connect(action, SIGNAL(triggered()), SLOT(slotInsertHoldFrame()));

        action = m_d->actionMan->createAction("insert_multiple_hold_frames");
        connect(action, SIGNAL(triggered()), SLOT(slotInsertMultipleHoldFrames()));

        action = m_d->actionMan->createAction("remove_hold_frame");
        connect(action, SIGNAL(triggered()), SLOT(slotRemoveHoldFrame()));

        action = m_d->actionMan->createAction("remove_multiple_hold_frames");
        connect(action, SIGNAL(triggered()), SLOT(slotRemoveMultipleHoldFrames()));

        action = m_d->actionMan->createAction("mirror_frames");
        connect(action, SIGNAL(triggered()), SLOT(slotMirrorFrames()));

        action = m_d->actionMan->createAction("copy_frames");
        connect(action, SIGNAL(triggered()), SLOT(slotCopyFrames()));

        action = m_d->actionMan->createAction("copy_frames_as_clones");
        connect(action, &KisAction::triggered, [this](){clone(false);});

        action = m_d->actionMan->createAction("make_clones_unique");
        connect(action, SIGNAL(triggered()), SLOT(slotMakeClonesUnique()));

        action = m_d->actionMan->createAction("cut_frames");
        connect(action, SIGNAL(triggered()), SLOT(slotCutFrames()));

        action = m_d->actionMan->createAction("paste_frames");
        connect(action, SIGNAL(triggered()), SLOT(slotPasteFrames()));

        action = m_d->actionMan->createAction("set_start_time");
        connect(action, SIGNAL(triggered()), SLOT(slotSetStartTimeToCurrentPosition()));

        action = m_d->actionMan->createAction("set_end_time");
        connect(action, SIGNAL(triggered()), SLOT(slotSetEndTimeToCurrentPosition()));

        action = m_d->actionMan->createAction("update_playback_range");
        connect(action, SIGNAL(triggered()), SLOT(slotUpdatePlackbackRange()));

        action = m_d->actionMan->actionByName("pin_to_timeline");
        m_d->pinLayerToTimelineAction = action;
        m_d->layerEditingMenu->addAction(action);
    }
}

void KisAnimTimelineFramesView::updateGeometries()
{
    QTableView::updateGeometries();

    const int availableHeight = m_d->horizontalRuler->height();
    const int margin = 2;
    const int minimalSize = availableHeight - 2 * margin;

    resizeToMinimalSize(m_d->addLayersButton, minimalSize);
    resizeToMinimalSize(m_d->audioOptionsButton, minimalSize);
    resizeToMinimalSize(m_d->zoomDragButton, minimalSize);

    int x = 2 * margin;
    int y = (availableHeight - minimalSize) / 2;
    m_d->addLayersButton->move(x, 2 * y);
    m_d->audioOptionsButton->move(x + minimalSize + 2 * margin, 2 * y);

    const int availableWidth = m_d->layersHeader->width();

    x = availableWidth - margin - minimalSize;
    m_d->zoomDragButton->move(x, 2 * y);
}

void KisAnimTimelineFramesView::slotCanvasUpdate(KoCanvasBase *canvas)
{
    horizontalScrollBar()->setEnabled(canvas != nullptr);
}

void KisAnimTimelineFramesView::slotUpdateIcons()
{
    m_d->addLayersButton->setIcon(KisIconUtils::loadIcon("addlayer"));
    m_d->audioOptionsButton->setIcon(KisIconUtils::loadIcon("audio-none"));
    m_d->zoomDragButton->setIcon(KisIconUtils::loadIcon("zoom-horizontal"));
}

void KisAnimTimelineFramesView::slotUpdateLayersMenu()
{
    QAction *action = 0;

    m_d->existingLayersMenu->clear();

    QVariant value = model()->headerData(0, Qt::Vertical, KisAnimTimelineFramesModel::OtherLayersRole);
    if (value.isValid()) {
        KisAnimTimelineFramesModel::OtherLayersList list = value.value<KisAnimTimelineFramesModel::OtherLayersList>();

        int i = 0;
        Q_FOREACH (const KisAnimTimelineFramesModel::OtherLayer &l, list) {
            action = m_d->existingLayersMenu->addAction(l.name);
            action->setData(i++);
        }
    }
}

void KisAnimTimelineFramesView::slotUpdateFrameActions()
{
    if (!m_d->actionMan) return;

    const QModelIndexList editableIndexes = calculateSelectionSpan(false, true);
    const bool hasEditableFrames = !editableIndexes.isEmpty();

    bool hasExistingFrames = false;
    Q_FOREACH (const QModelIndex &index, editableIndexes) {
        if (model()->data(index, KisAnimTimelineFramesModel::FrameExistsRole).toBool()) {
            hasExistingFrames = true;
            break;
        }
    }

    auto enableAction = [this] (const QString &id, bool value) {
        KisAction *action = m_d->actionMan->actionByName(id);
        KIS_SAFE_ASSERT_RECOVER_RETURN(action);
        action->setEnabled(value);
    };

    enableAction("add_blank_frame", hasEditableFrames);
    enableAction("add_duplicate_frame", hasEditableFrames);

    enableAction("insert_keyframe_left", hasEditableFrames);
    enableAction("insert_keyframe_right", hasEditableFrames);
    enableAction("insert_multiple_keyframes", hasEditableFrames);

    enableAction("remove_frames", hasEditableFrames && hasExistingFrames);
    enableAction("remove_frames_and_pull", hasEditableFrames);

    enableAction("insert_hold_frame", hasEditableFrames);
    enableAction("insert_multiple_hold_frames", hasEditableFrames);

    enableAction("remove_hold_frame", hasEditableFrames);
    enableAction("remove_multiple_hold_frames", hasEditableFrames);

    enableAction("mirror_frames", hasEditableFrames && editableIndexes.size() > 1);

    enableAction("copy_frames", true);
    enableAction("cut_frames", hasEditableFrames);
}

void KisAnimTimelineFramesView::slotSelectionChanged()
{
    int minColumn = std::numeric_limits<int>::max();
    int maxColumn = std::numeric_limits<int>::min();

    foreach (const QModelIndex &idx, selectedIndexes()) {
        if (idx.column() > maxColumn) {
            maxColumn = idx.column();
        }

        if (idx.column() < minColumn) {
            minColumn = idx.column();
        }
    }

    KisTimeSpan range;
    if (maxColumn > minColumn) {
        range = KisTimeSpan::fromTimeWithDuration(minColumn, maxColumn - minColumn + 1);
    }

    if (m_d->model->isPlaybackPaused()) {
        m_d->model->stopPlayback();
    }

    m_d->model->setPlaybackRange(range);
}

void KisAnimTimelineFramesView::slotReselectCurrentIndex()
{
    QModelIndex index = currentIndex();
    currentChanged(index, index);
}

void KisAnimTimelineFramesView::slotSetStartTimeToCurrentPosition()
{
     m_d->model->setFullClipRangeStart(this->currentIndex().column());
}

void KisAnimTimelineFramesView::slotSetEndTimeToCurrentPosition()
{
    m_d->model->setFullClipRangeEnd(this->currentIndex().column());
}

void KisAnimTimelineFramesView::slotUpdatePlackbackRange()
{
    QSet<int> rows;
    int minColumn = 0;
    int maxColumn = 0;

    calculateSelectionMetrics(minColumn, maxColumn, rows);

    m_d->model->setFullClipRangeStart(minColumn);
    m_d->model->setFullClipRangeEnd(maxColumn);
}

void KisAnimTimelineFramesView::slotUpdateInfiniteFramesCount()
{
    const int lastVisibleFrame = m_d->horizontalRuler->estimateLastVisibleColumn();
    m_d->model->setLastVisibleFrame(lastVisibleFrame);
}

void KisAnimTimelineFramesView::slotDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (m_d->model->isPlaybackActive()) return;

    int selectedColumn = -1;

    for (int j = topLeft.column(); j <= bottomRight.column(); j++) {
        QVariant value = m_d->model->data(
                    m_d->model->index(topLeft.row(), j),
                    KisAnimTimelineFramesModel::ActiveFrameRole);

        if (value.isValid() && value.toBool()) {
            selectedColumn = j;
            break;
        }
    }

    QModelIndex index = currentIndex();

    if (!index.isValid() && selectedColumn < 0) {
        return;
    }

    if (selectionModel()->selectedIndexes().count() > 1) return;

    if (selectedColumn == -1) {
        selectedColumn = index.column();
    }

    if (selectedColumn != index.column() && !m_d->dragInProgress) {
        int row= index.isValid() ? index.row() : 0;
        selectionModel()->setCurrentIndex(m_d->model->index(row, selectedColumn), QItemSelectionModel::ClearAndSelect);
    }
}

void KisAnimTimelineFramesView::slotHeaderDataChanged(Qt::Orientation orientation, int first, int last)
{
    Q_UNUSED(first);
    Q_UNUSED(last);

    if (orientation == Qt::Horizontal) {
        const int newFps = m_d->model->headerData(0, Qt::Horizontal, KisAnimTimelineFramesModel::FramesPerSecondRole).toInt();

        if (newFps != m_d->fps) {
            setFramesPerSecond(newFps);
        }
    }
}

void KisAnimTimelineFramesView::slotColorLabelChanged(int label)
{
    Q_FOREACH(QModelIndex index, selectedIndexes()) {
        m_d->model->setData(index, label, KisAnimTimelineFramesModel::FrameColorLabelIndexRole);
    }
    KisImageConfig(false).setDefaultFrameColorLabel(label);
}

void KisAnimTimelineFramesView::slotAddNewLayer()
{
    QModelIndex index = currentIndex();
    const int newRow = index.isValid() ? index.row() : 0;
    model()->insertRow(newRow);
}

void KisAnimTimelineFramesView::slotAddExistingLayer(QAction *action)
{
    QVariant value = action->data();

    if (value.isValid()) {
        QModelIndex index = currentIndex();
        const int newRow = index.isValid() ? index.row() + 1 : 0;

        m_d->model->insertOtherLayer(value.toInt(), newRow);
    }
}

void KisAnimTimelineFramesView::slotRemoveLayer()
{
    QModelIndex index = currentIndex();
    if (!index.isValid()) return;

    model()->removeRow(index.row());
}

void KisAnimTimelineFramesView::slotLayerContextMenuRequested(const QPoint &globalPos)
{
    m_d->layerEditingMenu->exec(globalPos);
}

void KisAnimTimelineFramesView::slotAddBlankFrame()
{
    QModelIndex index = currentIndex();
    if (!index.isValid() ||
        !m_d->model->data(index, KisAnimTimelineFramesModel::FrameEditableRole).toBool()) {

        return;
    }

    m_d->model->createFrame(index);
}

void KisAnimTimelineFramesView::slotAddDuplicateFrame()
{
    QModelIndex index = currentIndex();
    if (!index.isValid() ||
        !m_d->model->data(index, KisAnimTimelineFramesModel::FrameEditableRole).toBool()) {

        return;
    }

    m_d->model->copyFrame(index);
}

void KisAnimTimelineFramesView::slotRemoveSelectedFrames(bool entireColumn, bool pull)
{
    const QModelIndexList selectedIndices = calculateSelectionSpan(entireColumn);

    if (!selectedIndices.isEmpty()) {
        if (pull) {
            m_d->model->removeFramesAndOffset(selectedIndices);
        } else {
            m_d->model->removeFrames(selectedIndices);
        }
    }
}

void KisAnimTimelineFramesView::slotMirrorFrames(bool entireColumn)
{
    const QModelIndexList indexes = calculateSelectionSpan(entireColumn);

    if (!indexes.isEmpty()) {
        m_d->model->mirrorFrames(indexes);
    }
}

void KisAnimTimelineFramesView::slotPasteFrames(bool entireColumn)
{
    const QModelIndex currentIndex =
        !entireColumn ? this->currentIndex() : m_d->model->index(0, this->currentIndex().column());

    if (!currentIndex.isValid()) return;

    QClipboard *cb = QApplication::clipboard();
    const QMimeData *data = cb->mimeData();

    if (data && data->hasFormat("application/x-krita-frame")) {

        bool dataMoved = false;
        bool result = m_d->model->dropMimeDataExtended(data, Qt::MoveAction, currentIndex, &dataMoved);

        if (result && dataMoved) {
            cb->clear();
        }
    }
}

void KisAnimTimelineFramesView::slotMakeClonesUnique()
{
    if (!m_d->model) return;

    const QModelIndexList indices = calculateSelectionSpan(false);
    m_d->model->makeClonesUnique(indices);
}

void KisAnimTimelineFramesView::slotSelectAudioChannelFile()
{
    if (!m_d->model) return;

    QString defaultDir = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);

    const QString currentFile = m_d->model->audioChannelFileName();
    QDir baseDir = QFileInfo(currentFile).absoluteDir();
    if (baseDir.exists()) {
        defaultDir = baseDir.absolutePath();
    }

    const QString result = KisImportExportManager::askForAudioFileName(defaultDir, this);
    const QFileInfo info(result);

    if (info.exists()) {
        m_d->model->setAudioChannelFileName(info.absoluteFilePath());
    }
}

void KisAnimTimelineFramesView::slotAudioChannelMute(bool value)
{
    if (!m_d->model) return;

    if (value != m_d->model->isAudioMuted()) {
        m_d->model->setAudioMuted(value);
    }
}

void KisAnimTimelineFramesView::slotAudioChannelRemove()
{
    if (!m_d->model) return;
    m_d->model->setAudioChannelFileName(QString());
}

void KisAnimTimelineFramesView::slotUpdateAudioActions()
{
    if (!m_d->model) return;

    const QString currentFile = m_d->model->audioChannelFileName();

    if (currentFile.isEmpty()) {
        m_d->openAudioAction->setText(i18nc("@item:inmenu", "Open audio..."));
    } else {
        QFileInfo info(currentFile);
        m_d->openAudioAction->setText(i18nc("@item:inmenu", "Change audio (%1)...", info.fileName()));
    }

    m_d->audioMuteAction->setChecked(m_d->model->isAudioMuted());

    QIcon audioIcon;
    if (currentFile.isEmpty()) {
        audioIcon = KisIconUtils::loadIcon("audio-none");
    } else {
        if (m_d->model->isAudioMuted()) {
            audioIcon = KisIconUtils::loadIcon("audio-volume-mute");
        } else {
            audioIcon = KisIconUtils::loadIcon("audio-volume-high");
        }
    }

    m_d->audioOptionsButton->setIcon(audioIcon);

    m_d->volumeSlider->setEnabled(!m_d->model->isAudioMuted());

    KisSignalsBlocker b(m_d->volumeSlider);
    m_d->volumeSlider->setValue(qRound(m_d->model->audioVolume() * 100.0));
}

void KisAnimTimelineFramesView::slotAudioVolumeChanged(int value)
{
    m_d->model->setAudioVolume(qreal(value) / 100.0);
}

void KisAnimTimelineFramesView::slotScrollerStateChanged( QScroller::State state ) {

    if (state == QScroller::Dragging || state == QScroller::Scrolling ) {
        m_d->kineticScrollInfiniteFrameUpdater.start(16);
    } else {
        m_d->kineticScrollInfiniteFrameUpdater.stop();
    }

    KisKineticScroller::updateCursor(this, state);
}

void KisAnimTimelineFramesView::slotZoom(qreal zoom)
{
    const int originalFirstColumn = m_d->horizontalRuler->estimateFirstVisibleColumn();
    if (m_d->horizontalRuler->setZoom(m_d->horizontalRuler->zoom() + zoom)) {
        const int newLastColumn = m_d->horizontalRuler->estimateFirstVisibleColumn();
        if (newLastColumn >= m_d->model->columnCount()) {
            slotUpdateInfiniteFramesCount();
        }
        viewport()->update();
        horizontalScrollBar()->setValue(scrollPositionFromColumn(originalFirstColumn));
    }
}

void KisAnimTimelineFramesView::slotUpdateDragInfiniteFramesCount() {
    if(m_d->dragInProgress ||
      (m_d->model->isScrubbing() && horizontalScrollBar()->sliderPosition() == horizontalScrollBar()->maximum()) ) {
        slotUpdateInfiniteFramesCount();
    }
}

void KisAnimTimelineFramesView::slotRealignScrollBars() {
    QScrollBar* hBar = horizontalScrollBar();
    QScrollBar* vBar = verticalScrollBar();

    QSize desiredScrollArea = QSize(width() - verticalHeader()->width(), height() - horizontalHeader()->height());

    // Compensate for corner gap...
    if (hBar->isVisible() && vBar->isVisible()) {
        desiredScrollArea -= QSize(vBar->width(), hBar->height());
    }

    hBar->parentWidget()->layout()->setAlignment(Qt::AlignRight);
    hBar->setMaximumWidth(desiredScrollArea.width());
    hBar->setMinimumWidth(desiredScrollArea.width());


    vBar->parentWidget()->layout()->setAlignment(Qt::AlignBottom);
    vBar->setMaximumHeight(desiredScrollArea.height());
    vBar->setMinimumHeight(desiredScrollArea.height());
}

void KisAnimTimelineFramesView::slotEnsureRowVisible(int row)
{
    QModelIndex index = currentIndex();
    if (!index.isValid() || row < 0) return;

    index = m_d->model->index(row, index.column());
    scrollTo(index);
}

bool KisAnimTimelineFramesView::viewportEvent(QEvent *event)
{
    if (event->type() == QEvent::ToolTip && model()) {
        QHelpEvent *he = static_cast<QHelpEvent *>(event);
        QModelIndex index = model()->buddy(indexAt(he->pos()));
        if (index.isValid()) {
            QStyleOptionViewItem option = viewOptions();
            option.rect = visualRect(index);
            // The offset of the headers is needed to get the correct position inside the view.
            m_d->tip.showTip(this, he->pos() + QPoint(verticalHeader()->width(), horizontalHeader()->height()), option, index);
            return true;
        }
    }

    return QTableView::viewportEvent(event);
}

void KisAnimTimelineFramesView::mousePressEvent(QMouseEvent *event)
{
    QPersistentModelIndex index = indexAt(event->pos());

    if (m_d->modifiersCatcher->modifierPressed("pan-zoom")) {

        if (event->button() == Qt::RightButton) {
            // TODO: try calculate index under mouse cursor even when
            //       it is outside any visible row
            qreal staticPoint = index.isValid() ? index.column() : currentIndex().column();
//            m_d->zoomDragButton->beginZoom(event->pos(), staticPoint);
        } else if (event->button() == Qt::LeftButton) {
            m_d->initialDragPanPos = event->pos();
            m_d->initialDragPanValue =
                    QPoint(horizontalScrollBar()->value(),
                           verticalScrollBar()->value());
        }
        event->accept();

    } else if (event->button() == Qt::RightButton) {

        int numSelectedItems = selectionModel()->selectedIndexes().size();

        if (index.isValid() &&
                numSelectedItems <= 1 &&
                m_d->model->data(index, KisAnimTimelineFramesModel::FrameEditableRole).toBool()) {

            model()->setData(index, true, KisAnimTimelineFramesModel::ActiveLayerRole);
            model()->setData(index, true, KisAnimTimelineFramesModel::ActiveFrameRole);
            setCurrentIndex(index);

            if (model()->data(index, KisAnimTimelineFramesModel::FrameExistsRole).toBool() ||
                    model()->data(index, KisAnimTimelineFramesModel::SpecialKeyframeExists).toBool()) {

                {
                    KisSignalsBlocker b(m_d->colorSelector);
                    QVariant colorLabel = index.data(KisAnimTimelineFramesModel::FrameColorLabelIndexRole);
                    int labelIndex = colorLabel.isValid() ? colorLabel.toInt() : 0;
                    m_d->colorSelector->setCurrentIndex(labelIndex);
                }

                const bool hasClones = model()->data(index, KisAnimTimelineFramesModel::CloneCount).toInt() > 0;

                QMenu menu;
                createFrameEditingMenuActions(&menu, false, hasClones);
                menu.addSeparator();
                menu.addAction(m_d->colorSelectorAction);
                menu.exec(event->globalPos());

            } else {
                {
                    KisSignalsBlocker b(m_d->colorSelector);
                    const int labelIndex = KisImageConfig(true).defaultFrameColorLabel();
                    m_d->colorSelector->setCurrentIndex(labelIndex);
                }

                QMenu menu;
                createFrameEditingMenuActions(&menu, true, false);
                menu.addSeparator();
                menu.addAction(m_d->colorSelectorAction);
                menu.exec(event->globalPos());
            }
        } else if (numSelectedItems > 1) {
            int labelIndex = -1;
            bool firstKeyframe = true;
            bool hasKeyframes = false;
            bool containsClones = false;
            Q_FOREACH(QModelIndex index, selectedIndexes()) {
                hasKeyframes |= index.data(KisAnimTimelineFramesModel::FrameExistsRole).toBool();
                containsClones |= (index.data(KisAnimTimelineFramesModel::CloneCount).toInt() > 0);

                QVariant colorLabel = index.data(KisAnimTimelineFramesModel::FrameColorLabelIndexRole);
                if (colorLabel.isValid()) {
                    if (firstKeyframe) {
                        labelIndex = colorLabel.toInt();
                    } else if (labelIndex != colorLabel.toInt()) {
                        // Mixed colors in selection
                        labelIndex = -1;
                    }

                    firstKeyframe = false;
                }

                if (!firstKeyframe
                    && hasKeyframes
                    && containsClones
                    && labelIndex == -1) {
                    break; // Break out early if we find all of the above.
                }
            }

            if (hasKeyframes) {
                KisSignalsBlocker b(m_d->multiframeColorSelector);
                m_d->multiframeColorSelector->setCurrentIndex(labelIndex);
            }

            QMenu menu;
            createFrameEditingMenuActions(&menu, false, containsClones);
            menu.addSeparator();
            KisActionManager::safePopulateMenu(&menu, "mirror_frames", m_d->actionMan);
            menu.addSeparator();
            menu.addAction(m_d->multiframeColorSelectorAction);
            menu.exec(event->globalPos());
        }
    } else if (event->button() == Qt::MidButton) {
        QModelIndex index = model()->buddy(indexAt(event->pos()));
        if (index.isValid()) {
            QStyleOptionViewItem option = viewOptions();
            option.rect = visualRect(index);
            // The offset of the headers is needed to get the correct position inside the view.
            m_d->tip.showTip(this, event->pos() + QPoint(verticalHeader()->width(), horizontalHeader()->height()), option, index);
        }
        event->accept();
    } else {
        if (index.isValid()) {
            m_d->model->setLastClickedIndex(index);
        }

        m_d->lastPressedPosition =
                QPoint(horizontalOffset(), verticalOffset()) + event->pos();
        m_d->lastPressedModifier = event->modifiers();

        m_d->initialDragPanPos = event->pos();

        QAbstractItemView::mousePressEvent(event);
    }
}

void KisAnimTimelineFramesView::mouseMoveEvent(QMouseEvent *e)
{
    // Custom keyframe dragging distance based on zoom level.
    if (state() == DraggingState &&
        (horizontalHeader()->defaultSectionSize() / 2) < QApplication::startDragDistance() ) {

        const QPoint dragVector = e->pos() - m_d->initialDragPanPos;
        if (dragVector.manhattanLength() >= (horizontalHeader()->defaultSectionSize() / 2)) {
            startDrag(model()->supportedDragActions());
            setState(NoState);
            stopAutoScroll();
        }
    }

    if (m_d->modifiersCatcher->modifierPressed("pan-zoom")) {

        if (e->buttons() & Qt::RightButton) {

//            m_d->zoomDragButton->continueZoom(e->pos());
        } else if (e->buttons() & Qt::LeftButton) {

            QPoint diff = e->pos() - m_d->initialDragPanPos;
            QPoint offset = QPoint(m_d->initialDragPanValue.x() - diff.x(),
                                   m_d->initialDragPanValue.y() - diff.y());

            const int height = m_d->layersHeader->defaultSectionSize();

            if (m_d->initialDragPanValue.x() - diff.x() > horizontalScrollBar()->maximum() || m_d->initialDragPanValue.x() - diff.x() > horizontalScrollBar()->minimum() ){
                KisZoomableScrollBar* zoombar = static_cast<KisZoomableScrollBar*>(horizontalScrollBar());
                zoombar->overscroll(-diff.x());
            }

            horizontalScrollBar()->setValue(offset.x());
            verticalScrollBar()->setValue(offset.y() / height);
        }
        e->accept();
    } else if (e->buttons() == Qt::MidButton) {
        QModelIndex index = model()->buddy(indexAt(e->pos()));
        if (index.isValid()) {
            QStyleOptionViewItem option = viewOptions();
            option.rect = visualRect(index);
            // The offset of the headers is needed to get the correct position inside the view.
            m_d->tip.showTip(this, e->pos() + QPoint(verticalHeader()->width(), horizontalHeader()->height()), option, index);
        }
        e->accept();
    } else {
        m_d->model->setScrubState(true);
        QTableView::mouseMoveEvent(e);
    }
}

void KisAnimTimelineFramesView::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_d->modifiersCatcher->modifierPressed("pan-zoom")) {
        e->accept();
    } else {
        m_d->model->setScrubState(false);
        QTableView::mouseReleaseEvent(e);
    }
}

void KisAnimTimelineFramesView::startDrag(Qt::DropActions supportedActions)
{
    QModelIndexList indexes = selectionModel()->selectedIndexes();

    if (!indexes.isEmpty() && m_d->modifiersCatcher->modifierPressed("offset-frame")) {
        QVector<int> rows;
        int leftmostColumn = std::numeric_limits<int>::max();

        Q_FOREACH (const QModelIndex &index, indexes) {
            leftmostColumn = qMin(leftmostColumn, index.column());
            if (!rows.contains(index.row())) {
                rows.append(index.row());
            }
        }

        const int lastColumn = m_d->model->columnCount() - 1;

        selectionModel()->clear();
        Q_FOREACH (const int row, rows) {
            QItemSelection sel(m_d->model->index(row, leftmostColumn), m_d->model->index(row, lastColumn));
            selectionModel()->select(sel, QItemSelectionModel::Select);
        }

        supportedActions = Qt::MoveAction;

        {
            QModelIndexList indexes = selectedIndexes();
            for(int i = indexes.count() - 1 ; i >= 0; --i) {
                if (!isIndexDragEnabled(m_d->model, indexes.at(i)))
                    indexes.removeAt(i);
            }

            selectionModel()->clear();

            if (indexes.count() > 0) {
                QMimeData *data = m_d->model->mimeData(indexes);
                if (!data)
                    return;
                QRect rect;
                QPixmap pixmap = m_d->renderToPixmap(indexes, &rect);
                rect.adjust(horizontalOffset(), verticalOffset(), 0, 0);
                QDrag *drag = new QDrag(this);
                drag->setPixmap(pixmap);
                drag->setMimeData(data);
                drag->setHotSpot(m_d->lastPressedPosition - rect.topLeft());
                drag->exec(supportedActions, Qt::MoveAction);
                setCurrentIndex(currentIndex());
            }
        }
    } else {

        /**
         * Workaround for Qt5's bug: if we start a dragging action right during
         * Shift-selection, Qt will get crazy. We cannot workaround it easily,
         * because we would need to fork mouseMoveEvent() for that (where the
         * decision about drag state is done). So we just abort dragging in that
         * case.
         *
         * BUG:373067
         */
        if (m_d->lastPressedModifier & Qt::ShiftModifier) {
            return;
        }

        /**
         * Workaround for Qt5's bugs:
         *
         * 1) Qt doesn't treat selection the selection on D&D
         *    correctly, so we save it in advance and restore
         *    afterwards.
         *
         * 2) There is a private variable in QAbstractItemView:
         *    QAbstractItemView::Private::currentSelectionStartIndex.
         *    It is initialized *only* when the setCurrentIndex() is called
         *    explicitly on the view object, not on the selection model.
         *    Therefore we should explicitly call setCurrentIndex() after
         *    D&D, even if it already has *correct* value!
         *
         * 2) We should also call selectionModel()->select()
         *    explicitly.  There are two reasons for it: 1) Qt doesn't
         *    maintain selection over D&D; 2) when reselecting single
         *    element after D&D, Qt goes crazy, because it tries to
         *    read *global* keyboard modifiers. Therefore if we are
         *    dragging with Shift or Ctrl pressed it'll get crazy. So
         *    just reset it explicitly.
         */

        QModelIndexList selectionBefore = selectionModel()->selectedIndexes();
        QModelIndex currentBefore = selectionModel()->currentIndex();

        // initialize a global status variable
        m_d->dragWasSuccessful = false;
        QAbstractItemView::startDrag(supportedActions);

        QModelIndex newCurrent;
        QPoint selectionOffset;

        if (m_d->dragWasSuccessful) {
            newCurrent = currentIndex();
            selectionOffset = QPoint(newCurrent.column() - currentBefore.column(),
                                     newCurrent.row() - currentBefore.row());
        } else {
            newCurrent = currentBefore;
            selectionOffset = QPoint();
        }

        setCurrentIndex(newCurrent);
        selectionModel()->clearSelection();
        Q_FOREACH (const QModelIndex &idx, selectionBefore) {
            QModelIndex newIndex =
                    model()->index(idx.row() + selectionOffset.y(),
                                   idx.column() + selectionOffset.x());
            selectionModel()->select(newIndex, QItemSelectionModel::Select);
        }
    }
}

void KisAnimTimelineFramesView::dragEnterEvent(QDragEnterEvent *event)
{
    m_d->dragInProgress = true;
    m_d->model->setScrubState(true);

    QTableView::dragEnterEvent(event);
}

void KisAnimTimelineFramesView::dragMoveEvent(QDragMoveEvent *event)
{
    m_d->dragInProgress = true;
    m_d->model->setScrubState(true);

    QAbstractItemView::dragMoveEvent(event);

    // Let's check for moving within a selection --
    // We want to override the built in qt behavior that
    // denies drag events when dragging within a selection...
    if (!event->isAccepted() && selectionModel()->isSelected(indexAt(event->pos()))) {
        event->setAccepted(true);
    }

    if (event->isAccepted()) {
        QModelIndex index = indexAt(event->pos());

        if (!m_d->model->canDropFrameData(event->mimeData(), index)) {
            event->ignore();
        } else {
            selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
        }
    }
}

void KisAnimTimelineFramesView::dragLeaveEvent(QDragLeaveEvent *event)
{
    m_d->dragInProgress = false;
    m_d->model->setScrubState(false);

    QAbstractItemView::dragLeaveEvent(event);
}

void KisAnimTimelineFramesView::dropEvent(QDropEvent *event)
{
    m_d->dragInProgress = false;
    m_d->model->setScrubState(false);

    if (event->keyboardModifiers() & Qt::ControlModifier) {
        event->setDropAction(Qt::CopyAction);
    } else if (event->keyboardModifiers() & Qt::AltModifier) {
        event->setDropAction(Qt::LinkAction);
    }

    QAbstractItemView::dropEvent(event);

    // Override drop event to accept drops within selected range.0
    QModelIndex index = indexAt(event->pos());
    if (!event->isAccepted() &&  selectionModel()->isSelected(index)) {
        event->setAccepted(true);
        const Qt::DropAction action = event->dropAction();
        const int row = event->pos().y();
        const int column = event->pos().x();
        if (m_d->model->dropMimeData(event->mimeData(), action, row, column, index)) {
            event->acceptProposedAction();
        }
    }

    m_d->dragWasSuccessful = event->isAccepted();
}

void KisAnimTimelineFramesView::wheelEvent(QWheelEvent *e)
{
    QModelIndex index = currentIndex();
    int column= -1;

    if (verticalHeader()->rect().contains(verticalHeader()->mapFromGlobal(e->globalPos()))) {
        QTableView::wheelEvent(e);
        return;
    }

    if (index.isValid()) {
        column= index.column() + ((e->delta() > 0) ? 1 : -1);
    }

    if (column >= 0 && !m_d->dragInProgress) {
        setCurrentIndex(m_d->model->index(index.row(), column));
    }
}

void KisAnimTimelineFramesView::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    updateGeometries();
    slotUpdateInfiniteFramesCount();
}

void KisAnimTimelineFramesView::rowsInserted(const QModelIndex& parent, int start, int end)
{
    QTableView::rowsInserted(parent, start, end);
}

void KisAnimTimelineFramesView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QTableView::currentChanged(current, previous);

    if (previous.column() != current.column()) {
        m_d->model->setData(previous, false, KisAnimTimelineFramesModel::ActiveFrameRole);
        m_d->model->setData(current, true, KisAnimTimelineFramesModel::ActiveFrameRole);
    }
}

QItemSelectionModel::SelectionFlags KisAnimTimelineFramesView::selectionCommand(const QModelIndex &index,
                                                                         const QEvent *event) const
{
    // WARNING: Copy-pasted from KisNodeView! Please keep in sync!

    /**
     * Qt has a bug: when we Ctrl+click on an item, the item's
     * selections gets toggled on mouse *press*, whereas usually it is
     * done on mouse *release*.  Therefore the user cannot do a
     * Ctrl+D&D with the default configuration. This code fixes the
     * problem by manually returning QItemSelectionModel::NoUpdate
     * flag when the user clicks on an item and returning
     * QItemSelectionModel::Toggle on release.
     */

    if (event &&
            (event->type() == QEvent::MouseButtonPress ||
             event->type() == QEvent::MouseButtonRelease) &&
            index.isValid()) {

        const QMouseEvent *mevent = static_cast<const QMouseEvent*>(event);

        if (mevent->button() == Qt::RightButton &&
                selectionModel()->selectedIndexes().contains(index)) {

            // Allow calling context menu for multiple layers
            return QItemSelectionModel::NoUpdate;
        }

        if (event->type() == QEvent::MouseButtonPress &&
                (mevent->modifiers() & Qt::ControlModifier)) {

            return QItemSelectionModel::NoUpdate;
        }

        if (event->type() == QEvent::MouseButtonRelease &&
                (mevent->modifiers() & Qt::ControlModifier)) {

            return QItemSelectionModel::Toggle;
        }
    }

    return QAbstractItemView::selectionCommand(index, event);
}

void KisAnimTimelineFramesView::setFramesPerSecond(int fps)
{
    m_d->fps = fps;
    m_d->horizontalRuler->setFramePerSecond(fps);
}

QModelIndexList KisAnimTimelineFramesView::calculateSelectionSpan(bool entireColumn, bool editableOnly) const
{
    QModelIndexList indexes;

    if (entireColumn) {
        QSet<int> rows;
        int minColumn = 0;
        int maxColumn = 0;

        calculateSelectionMetrics(minColumn, maxColumn, rows);

        rows.clear();
        for (int i = 0; i < m_d->model->rowCount(); i++) {
            if (editableOnly &&
                !m_d->model->data(m_d->model->index(i, minColumn), KisAnimTimelineFramesModel::FrameEditableRole).toBool()) continue;

            for (int column = minColumn; column <= maxColumn; column++) {
                indexes << m_d->model->index(i, column);
            }
        }
    } else {
        Q_FOREACH (const QModelIndex &index, selectionModel()->selectedIndexes()) {
            if (!editableOnly || m_d->model->data(index, KisAnimTimelineFramesModel::FrameEditableRole).toBool()) {
                indexes << index;
            }
        }
    }

    return indexes;
}

void KisAnimTimelineFramesView::calculateSelectionMetrics(int &minColumn, int &maxColumn, QSet<int> &rows) const
{
    minColumn = std::numeric_limits<int>::max();
    maxColumn = std::numeric_limits<int>::min();

    Q_FOREACH (const QModelIndex &index, selectionModel()->selectedIndexes()) {
        if (!m_d->model->data(index, KisAnimTimelineFramesModel::FrameEditableRole).toBool()) continue;

        rows.insert(index.row());
        minColumn = qMin(minColumn, index.column());
        maxColumn = qMax(maxColumn, index.column());
    }
}

void KisAnimTimelineFramesView::insertKeyframes(int count, int timing, TimelineDirection direction, bool entireColumn)
{
    QSet<int> rows;
    int minColumn = 0, maxColumn = 0;

    calculateSelectionMetrics(minColumn, maxColumn, rows);

    if (count <= 0) { //Negative count? Use number of selected frames.
        count = qMax(1, maxColumn - minColumn + 1);
    }

    const int insertionColumn =
        direction == TimelineDirection::RIGHT ?
        maxColumn + 1 : minColumn;

    if (entireColumn) {
        rows.clear();
        for (int i = 0; i < m_d->model->rowCount(); i++) {
            if (!m_d->model->data(m_d->model->index(i, insertionColumn), KisAnimTimelineFramesModel::FrameEditableRole).toBool()) continue;
            rows.insert(i);
        }
    }

    if (!rows.isEmpty()) {
#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
        m_d->model->insertFrames(insertionColumn, QList<int>(rows.begin(), rows.end()), count, timing);
#else
        m_d->model->insertFrames(insertionColumn, QList<int>::fromSet(rows), count, timing);
#endif
    }
}

void KisAnimTimelineFramesView::insertMultipleKeyframes(bool entireColumn)
{
    int count, timing;
    TimelineDirection direction;

    if (m_d->insertKeyframeDialog->promptUserSettings(count, timing, direction)) {
        insertKeyframes(count, timing, direction, entireColumn);
    }
}

void KisAnimTimelineFramesView::insertOrRemoveHoldFrames(int count, bool entireColumn)
{
    QModelIndexList indexes;

    if (!entireColumn) {
        Q_FOREACH (const QModelIndex &index, selectionModel()->selectedIndexes()) {
            if (m_d->model->data(index, KisAnimTimelineFramesModel::FrameEditableRole).toBool()) {
                indexes << index;
            }
        }
    } else {
        const int column = selectionModel()->currentIndex().column();

        for (int i = 0; i < m_d->model->rowCount(); i++) {
            const QModelIndex index = m_d->model->index(i, column);
            if (m_d->model->data(index, KisAnimTimelineFramesModel::FrameEditableRole).toBool()) {
                indexes << index;
            }
        }
    }

    if (!indexes.isEmpty()) {
        // add extra columns to the end of the timeline if we are adding hold frames
        // they will be truncated if we don't do this
        if (count > 0) {
            // Scan all the layers and find out what layer has the most keyframes
            // only keep a reference of layer that has the most keyframes
            int keyframesInLayerNode = 0;
            Q_FOREACH (const QModelIndex &index, indexes) {
                KisNodeSP layerNode = m_d->model->nodeAt(index);

                KisKeyframeChannel *channel = layerNode->getKeyframeChannel(KisKeyframeChannel::Raster.id());
                if (!channel) continue;

                if (keyframesInLayerNode < channel->allKeyframeTimes().count()) {
                   keyframesInLayerNode = channel->allKeyframeTimes().count();
                }
            }
            m_d->model->setLastVisibleFrame(m_d->model->columnCount() + count*keyframesInLayerNode);
        }


        m_d->model->insertHoldFrames(indexes, count);

        // Fan selection based on insertion or deletion.
        // This should allow better UI/UX for insertion of keyframes or hold frames.
        fanSelectedFrames(indexes, count);

        // bulk adding frames can add too many
        // trim timeline to clean up extra frames that might have been added
        slotUpdateInfiniteFramesCount();
    }
}

void KisAnimTimelineFramesView::insertOrRemoveMultipleHoldFrames(bool insertion, bool entireColumn)
{
    bool ok = false;
    const int count = QInputDialog::getInt(this,
                                           i18nc("@title:window", "Insert or Remove Hold Frames"),
                                           i18nc("@label:spinbox", "Enter number of frames"),
                                           insertion ?
                                               m_d->insertKeyframeDialog->defaultTimingOfAddedFrames() :
                                               m_d->insertKeyframeDialog->defaultNumberOfHoldFramesToRemove(),
                                           1, 10000, 1, &ok);

    if (ok) {
        if (insertion) {
            m_d->insertKeyframeDialog->setDefaultTimingOfAddedFrames(count);
            insertOrRemoveHoldFrames(count, entireColumn);
        } else {
            m_d->insertKeyframeDialog->setDefaultNumberOfHoldFramesToRemove(count);
            insertOrRemoveHoldFrames(-count, entireColumn);
        }

    }
}

void KisAnimTimelineFramesView::fanSelectedFrames(const QModelIndexList &selection, int count, bool ignoreKeyless) {
    QMap<int, QList<int>> indexMap;

    QList<QModelIndex> selectedIndices = selection;

    foreach (const QModelIndex &index, selectedIndices) {
        if (!indexMap.contains(index.row())) {
            indexMap.insert(index.row(), QList<int>());
        }

        if (m_d->model->data(index, KisAnimTimelineFramesModel::FrameExistsRole).value<bool>() || !ignoreKeyless) {
            indexMap[index.row()] << index.column();
        }
    }

    selectionModel()->clearSelection();
    KisSignalsBlocker blockSig(selectionModel());
    foreach (const int &layer, indexMap.keys()) {
        QList<int>::const_iterator it;
        int progressIndex = 0;

        std::sort(indexMap[layer].begin(), indexMap[layer].end());
        for (it = indexMap[layer].constBegin(); it != indexMap[layer].constEnd(); it++) {
            const int offsetColumn = *it + (progressIndex * count);
            selectionModel()->select(model()->index(layer, offsetColumn), QItemSelectionModel::Select);
            progressIndex++;
        }
    }
}

void KisAnimTimelineFramesView::cutCopyImpl(bool entireColumn, bool copy)
{
    const QModelIndexList selectedIndices = calculateSelectionSpan(entireColumn, !copy);
    if (selectedIndices.isEmpty()) return;

    int minColumn = std::numeric_limits<int>::max();
    int minRow = std::numeric_limits<int>::max();
    Q_FOREACH (const QModelIndex &index, selectedIndices) {
        minRow = qMin(minRow, index.row());
        minColumn = qMin(minColumn, index.column());
    }

    const QModelIndex baseIndex = m_d->model->index(minRow, minColumn);
    QMimeData *data = m_d->model->mimeDataExtended(selectedIndices,
                                                   baseIndex,
                                                   copy ?
                                                       KisAnimTimelineFramesModel::CopyFramesPolicy :
                                                       KisAnimTimelineFramesModel::MoveFramesPolicy);

    if (data) {
        QClipboard *cb = QApplication::clipboard();
        cb->setMimeData(data);
    }
}

void KisAnimTimelineFramesView::clone(bool entireColumn)
{
    const QModelIndexList selectedIndices = calculateSelectionSpan(entireColumn, false);
    if (selectedIndices.isEmpty()) return;

    int minColumn = std::numeric_limits<int>::max();
    int minRow = std::numeric_limits<int>::max();
    Q_FOREACH (const QModelIndex &index, selectedIndices) {
        minRow = qMin(minRow, index.row());
        minColumn = qMin(minColumn, index.column());
    }

    const QModelIndex baseIndex = m_d->model->index(minRow, minColumn);
    QMimeData *data = m_d->model->mimeDataExtended(selectedIndices,
                                                   baseIndex,
                                                   KisAnimTimelineFramesModel::CloneFramesPolicy);

    if (data) {
        QClipboard *cb = QApplication::clipboard();
        cb->setMimeData(data);
    }
}

void KisAnimTimelineFramesView::createFrameEditingMenuActions(QMenu *menu, bool emptyFrame, bool cloneFrameSelected)
{
    slotUpdateFrameActions();

    // calculate if selection range is set. This will determine if the update playback range is available
    QSet<int> rows;
    int minColumn = 0;
    int maxColumn = 0;
    calculateSelectionMetrics(minColumn, maxColumn, rows);
    bool selectionExists = minColumn != maxColumn;

    menu->addSection(i18n("Edit Frames:"));
    menu->addSeparator();

    if (selectionExists) {
        KisActionManager::safePopulateMenu(menu, "update_playback_range", m_d->actionMan);
    } else {
        KisActionManager::safePopulateMenu(menu, "set_start_time", m_d->actionMan);
        KisActionManager::safePopulateMenu(menu, "set_end_time", m_d->actionMan);
    }

    menu->addSeparator();

    if (!emptyFrame) {
        KisActionManager::safePopulateMenu(menu, "cut_frames", m_d->actionMan);
        KisActionManager::safePopulateMenu(menu, "copy_frames", m_d->actionMan);
        KisActionManager::safePopulateMenu(menu, "copy_frames_as_clones", m_d->actionMan);
    }

    KisActionManager::safePopulateMenu(menu, "paste_frames", m_d->actionMan);

    if (!emptyFrame && cloneFrameSelected) {
        KisActionManager::safePopulateMenu(menu, "make_clones_unique", m_d->actionMan);
    }

    menu->addSeparator();

    {   //Frames submenu.
        QMenu *frames = menu->addMenu(i18nc("@item:inmenu", "Keyframes"));
        KisActionManager::safePopulateMenu(frames, "insert_keyframe_left", m_d->actionMan);
        KisActionManager::safePopulateMenu(frames, "insert_keyframe_right", m_d->actionMan);
        frames->addSeparator();
        KisActionManager::safePopulateMenu(frames, "insert_multiple_keyframes", m_d->actionMan);
    }

    {   //Holds submenu.
        QMenu *hold = menu->addMenu(i18nc("@item:inmenu", "Hold Frames"));
        KisActionManager::safePopulateMenu(hold, "insert_hold_frame", m_d->actionMan);
        KisActionManager::safePopulateMenu(hold, "remove_hold_frame", m_d->actionMan);
        hold->addSeparator();
        KisActionManager::safePopulateMenu(hold, "insert_multiple_hold_frames", m_d->actionMan);
        KisActionManager::safePopulateMenu(hold, "remove_multiple_hold_frames", m_d->actionMan);
    }

    menu->addSeparator();

    if (!emptyFrame) {
        KisActionManager::safePopulateMenu(menu, "remove_frames", m_d->actionMan);
    }
    KisActionManager::safePopulateMenu(menu, "remove_frames_and_pull", m_d->actionMan);

    menu->addSeparator();

    if (emptyFrame) {
        KisActionManager::safePopulateMenu(menu, "add_blank_frame", m_d->actionMan);
        KisActionManager::safePopulateMenu(menu, "add_duplicate_frame", m_d->actionMan);
        menu->addSeparator();
    }
}

int KisAnimTimelineFramesView::scrollPositionFromColumn(int column) {
    const int sectionWidth = m_d->horizontalRuler->defaultSectionSize();
    return sectionWidth * column;
}

QStyleOptionViewItem KisAnimTimelineFramesView::Private::viewOptionsV4() const
{
    QStyleOptionViewItem option = q->viewOptions();
    option.locale = q->locale();
    option.locale.setNumberOptions(QLocale::OmitGroupSeparator);
    option.widget = q;
    return option;
}

QItemViewPaintPairs KisAnimTimelineFramesView::Private::draggablePaintPairs(const QModelIndexList &indexes, QRect *r) const
{
    Q_ASSERT(r);
    QRect &rect = *r;
    const QRect viewportRect = q->viewport()->rect();
    QItemViewPaintPairs ret;
    for (int i = 0; i < indexes.count(); ++i) {
        const QModelIndex &index = indexes.at(i);
        const QRect current = q->visualRect(index);
        if (current.intersects(viewportRect)) {
            ret += qMakePair(current, index);
            rect |= current;
        }
    }
    rect &= viewportRect;
    return ret;
}

QPixmap KisAnimTimelineFramesView::Private::renderToPixmap(const QModelIndexList &indexes, QRect *r) const
{
    Q_ASSERT(r);
    QItemViewPaintPairs paintPairs = draggablePaintPairs(indexes, r);
    if (paintPairs.isEmpty())
        return QPixmap();
    QPixmap pixmap(r->size());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option = viewOptionsV4();
    option.state |= QStyle::State_Selected;
    for (int j = 0; j < paintPairs.count(); ++j) {
        option.rect = paintPairs.at(j).first.translated(-r->topLeft());
        const QModelIndex &current = paintPairs.at(j).second;
        //adjustViewOptionsForIndex(&option, current);

        q->itemDelegate(current)->paint(&painter, option, current);
    }
    return pixmap;
}

void resizeToMinimalSize(QAbstractButton *w, int minimalSize)
{
    QSize buttonSize = w->sizeHint();
    if (buttonSize.height() > minimalSize) {
        buttonSize = QSize(minimalSize, minimalSize);
    }
    w->resize(buttonSize);
}

inline bool isIndexDragEnabled(QAbstractItemModel *model, const QModelIndex &index)
{
    return (model->flags(index) & Qt::ItemIsDragEnabled);
}
