/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "timeline_frames_view.h"

#include "timeline_frames_model.h"
#include "timeline_ruler_header.h"
#include "timeline_layers_header.h"
#include "timeline_insert_keyframe_dialog.h"
#include "timeline_frames_item_delegate.h"

#include <QPainter>
#include <QApplication>
#include <QDropEvent>
#include <QMenu>
#include <QScrollBar>
#include <QScroller>
#include <QDrag>
#include <QInputDialog>
#include <QClipboard>
#include <QMimeData>
#include "config-qtmultimedia.h"

#include "KSharedConfig"
#include "KisKineticScroller.h"

#include "kis_zoom_button.h"
#include "kis_icon_utils.h"
#include "kis_animation_utils.h"
#include "kis_custom_modifiers_catcher.h"
#include "kis_action.h"
#include "kis_signal_compressor.h"
#include "kis_time_range.h"
#include "kis_color_label_selector_widget.h"
#include "kis_keyframe_channel.h"
#include "kis_slider_spin_box.h"
#include <KisImportExportManager.h>
#include <kis_signals_blocker.h>
#include <kis_image_config.h>

#include <KoFileDialog.h>
#include <KoIconToolTip.h>

typedef QPair<QRect, QModelIndex> QItemViewPaintPair;
typedef QList<QItemViewPaintPair> QItemViewPaintPairs;

struct TimelineFramesView::Private
{
    Private(TimelineFramesView *_q)
        : q(_q),
          fps(1),
          zoomStillPointIndex(-1),
          zoomStillPointOriginalOffset(0),
          dragInProgress(false),
          dragWasSuccessful(false),
          modifiersCatcher(0),
          selectionChangedCompressor(300, KisSignalCompressor::FIRST_INACTIVE)
    {}

    TimelineFramesView *q;

    TimelineFramesModel *model;
    TimelineRulerHeader *horizontalRuler;
    TimelineLayersHeader *layersHeader;
    int fps;
    int zoomStillPointIndex;
    int zoomStillPointOriginalOffset;
    QPoint initialDragPanValue;
    QPoint initialDragPanPos;

    QToolButton *addLayersButton;
    KisAction *showHideLayerAction;

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
    KisSignalCompressor selectionChangedCompressor;

    QStyleOptionViewItem viewOptionsV4() const;
    QItemViewPaintPairs draggablePaintPairs(const QModelIndexList &indexes, QRect *r) const;
    QPixmap renderToPixmap(const QModelIndexList &indexes, QRect *r) const;

    KoIconToolTip tip;

    KisActionManager *actionMan = 0;
};

TimelineFramesView::TimelineFramesView(QWidget *parent)
    : QTableView(parent),
      m_d(new Private(this))
{
    m_d->modifiersCatcher = new KisCustomModifiersCatcher(this);
    m_d->modifiersCatcher->addModifier("pan-zoom", Qt::Key_Space);
    m_d->modifiersCatcher->addModifier("offset-frame", Qt::Key_Alt);

    setCornerButtonEnabled(false);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    setItemDelegate(new TimelineFramesItemDelegate(this));

    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDefaultDropAction(Qt::MoveAction);

    m_d->horizontalRuler = new TimelineRulerHeader(this);
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

    m_d->layersHeader = new TimelineLayersHeader(this);

    m_d->layersHeader->setSectionResizeMode(QHeaderView::Fixed);

    m_d->layersHeader->setDefaultSectionSize(24);
    m_d->layersHeader->setMinimumWidth(60);
    m_d->layersHeader->setHighlightSections(true);

    this->setVerticalHeader(m_d->layersHeader);

    connect(horizontalScrollBar(), SIGNAL(valueChanged(int)), SLOT(slotUpdateInfiniteFramesCount()));
    connect(horizontalScrollBar(), SIGNAL(sliderReleased()), SLOT(slotUpdateInfiniteFramesCount()));

    /********** New Layer Menu ***********************************************************/

    m_d->addLayersButton = new QToolButton(this);
    m_d->addLayersButton->setAutoRaise(true);
    m_d->addLayersButton->setIcon(KisIconUtils::loadIcon("addlayer"));
    m_d->addLayersButton->setIconSize(QSize(20, 20));
    m_d->addLayersButton->setPopupMode(QToolButton::InstantPopup);

    m_d->layerEditingMenu = new QMenu(this);
    m_d->layerEditingMenu->addAction(KisAnimationUtils::newLayerActionName, this, SLOT(slotAddNewLayer()));
    m_d->existingLayersMenu = m_d->layerEditingMenu->addMenu(KisAnimationUtils::addExistingLayerActionName);
    m_d->layerEditingMenu->addSeparator();

    m_d->layerEditingMenu->addAction(KisAnimationUtils::removeLayerActionName, this, SLOT(slotRemoveLayer()));

    connect(m_d->existingLayersMenu, SIGNAL(aboutToShow()), SLOT(slotUpdateLayersMenu()));
    connect(m_d->existingLayersMenu, SIGNAL(triggered(QAction*)), SLOT(slotAddExistingLayer(QAction*)));

    connect(m_d->layersHeader, SIGNAL(sigRequestContextMenu(QPoint)), SLOT(slotLayerContextMenuRequested(QPoint)));

    m_d->addLayersButton->setMenu(m_d->layerEditingMenu);

    /********** Audio Channel Menu *******************************************************/

    m_d->audioOptionsButton = new QToolButton(this);
    m_d->audioOptionsButton->setAutoRaise(true);
    m_d->audioOptionsButton->setIcon(KisIconUtils::loadIcon("audio-none"));
    m_d->audioOptionsButton->setIconSize(QSize(20, 20)); // very small on windows if not explicitly set
    m_d->audioOptionsButton->setPopupMode(QToolButton::InstantPopup);

    m_d->audioOptionsMenu = new QMenu(this);

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
    m_d->colorSelectorAction = new QWidgetAction(this);
    m_d->colorSelectorAction->setDefaultWidget(m_d->colorSelector);
    connect(m_d->colorSelector, &KisColorLabelSelectorWidget::currentIndexChanged, this, &TimelineFramesView::slotColorLabelChanged);

    m_d->multiframeColorSelector = new KisColorLabelSelectorWidget(this);
    m_d->multiframeColorSelectorAction = new QWidgetAction(this);
    m_d->multiframeColorSelectorAction->setDefaultWidget(m_d->multiframeColorSelector);
    connect(m_d->multiframeColorSelector, &KisColorLabelSelectorWidget::currentIndexChanged, this, &TimelineFramesView::slotColorLabelChanged);

    /********** Insert Keyframes Dialog **************************************************/

    m_d->insertKeyframeDialog = new TimelineInsertKeyframeDialog(this);

    /********** Zoom Button **************************************************************/

    m_d->zoomDragButton = new KisZoomButton(this);
    m_d->zoomDragButton->setAutoRaise(true);
    m_d->zoomDragButton->setIcon(KisIconUtils::loadIcon("zoom-horizontal"));
    m_d->zoomDragButton->setIconSize(QSize(20, 20)); // this icon is very small on windows if no explicitly set

    m_d->zoomDragButton->setToolTip(i18nc("@info:tooltip", "Zoom Timeline. Hold down and drag left or right."));
    m_d->zoomDragButton->setPopupMode(QToolButton::InstantPopup);
    connect(m_d->zoomDragButton, SIGNAL(zoomLevelChanged(qreal)), SLOT(slotZoomButtonChanged(qreal)));
    connect(m_d->zoomDragButton, SIGNAL(zoomStarted(qreal)), SLOT(slotZoomButtonPressed(qreal)));

    setFramesPerSecond(12);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    {
        QScroller *scroller = KisKineticScroller::createPreconfiguredScroller(this);
        if( scroller ) {
            connect(scroller, SIGNAL(stateChanged(QScroller::State)),
                    this, SLOT(slotScrollerStateChanged(QScroller::State)));
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
}

TimelineFramesView::~TimelineFramesView()
{
}

void TimelineFramesView::setShowInTimeline(KisAction *action)
{
    m_d->showHideLayerAction = action;
    m_d->layerEditingMenu->addAction(m_d->showHideLayerAction);
}

void TimelineFramesView::setActionManager(KisActionManager *actionManager)
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

        action = m_d->actionMan->createAction("copy_frames_to_clipboard");
        connect(action, SIGNAL(triggered()), SLOT(slotCopyFrames()));

        action = m_d->actionMan->createAction("cut_frames_to_clipboard");
        connect(action, SIGNAL(triggered()), SLOT(slotCutFrames()));

        action = m_d->actionMan->createAction("paste_frames_from_clipboard");
        connect(action, SIGNAL(triggered()), SLOT(slotPasteFrames()));

        action = m_d->actionMan->createAction("set_start_time");
        connect(action, SIGNAL(triggered()), SLOT(slotSetStartTimeToCurrentPosition()));

        action = m_d->actionMan->createAction("set_end_time");
        connect(action, SIGNAL(triggered()), SLOT(slotSetEndTimeToCurrentPosition()));

        action = m_d->actionMan->createAction("update_playback_range");
        connect(action, SIGNAL(triggered()), SLOT(slotUpdatePlackbackRange()));
    }
}

void resizeToMinimalSize(QAbstractButton *w, int minimalSize) {
    QSize buttonSize = w->sizeHint();
    if (buttonSize.height() > minimalSize) {
        buttonSize = QSize(minimalSize, minimalSize);
    }
    w->resize(buttonSize);
}

void TimelineFramesView::updateGeometries()
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

void TimelineFramesView::setModel(QAbstractItemModel *model)
{
    TimelineFramesModel *framesModel = qobject_cast<TimelineFramesModel*>(model);
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

void TimelineFramesView::setFramesPerSecond(int fps)
{
    m_d->fps = fps;
    m_d->horizontalRuler->setFramePerSecond(fps);

    // For some reason simple update sometimes doesn't work here, so
    // reset the whole header
    //
    // m_d->horizontalRuler->reset();
}

void TimelineFramesView::slotZoomButtonPressed(qreal staticPoint)
{
    m_d->zoomStillPointIndex =
            qIsNaN(staticPoint) ? currentIndex().column() : staticPoint;

    const int w = m_d->horizontalRuler->defaultSectionSize();

    m_d->zoomStillPointOriginalOffset =
            w * m_d->zoomStillPointIndex -
            horizontalScrollBar()->value();
}

void TimelineFramesView::slotZoomButtonChanged(qreal zoomLevel)
{
    if (m_d->horizontalRuler->setZoom(zoomLevel)) {
        slotUpdateInfiniteFramesCount();

        const int w = m_d->horizontalRuler->defaultSectionSize();
        horizontalScrollBar()->setValue(w * m_d->zoomStillPointIndex - m_d->zoomStillPointOriginalOffset);

        viewport()->update();
    }
}

void TimelineFramesView::slotColorLabelChanged(int label)
{
    Q_FOREACH(QModelIndex index, selectedIndexes()) {
        m_d->model->setData(index, label, TimelineFramesModel::FrameColorLabelIndexRole);
    }
    KisImageConfig(false).setDefaultFrameColorLabel(label);
}

void TimelineFramesView::slotSelectAudioChannelFile()
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

void TimelineFramesView::slotAudioChannelMute(bool value)
{
    if (!m_d->model) return;

    if (value != m_d->model->isAudioMuted()) {
        m_d->model->setAudioMuted(value);
    }
}

void TimelineFramesView::slotUpdateIcons()
{
    m_d->addLayersButton->setIcon(KisIconUtils::loadIcon("addlayer"));
    m_d->audioOptionsButton->setIcon(KisIconUtils::loadIcon("audio-none"));
    m_d->zoomDragButton->setIcon(KisIconUtils::loadIcon("zoom-horizontal"));
}

void TimelineFramesView::slotAudioChannelRemove()
{
    if (!m_d->model) return;
    m_d->model->setAudioChannelFileName(QString());
}

void TimelineFramesView::slotUpdateAudioActions()
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

void TimelineFramesView::slotAudioVolumeChanged(int value)
{
    m_d->model->setAudioVolume(qreal(value) / 100.0);
}

void TimelineFramesView::slotUpdateInfiniteFramesCount()
{
    if (horizontalScrollBar()->isSliderDown()) return;

    const int sectionWidth = m_d->horizontalRuler->defaultSectionSize();
    const int calculatedIndex =
            (horizontalScrollBar()->value() +
             m_d->horizontalRuler->width() - 1) / sectionWidth;

    m_d->model->setLastVisibleFrame(calculatedIndex);
}

void TimelineFramesView::slotScrollerStateChanged( QScroller::State state ) {
    KisKineticScroller::updateCursor(this, state);
}

void TimelineFramesView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QTableView::currentChanged(current, previous);

    if (previous.column() != current.column()) {
        m_d->model->setData(previous, false, TimelineFramesModel::ActiveFrameRole);
        m_d->model->setData(current, true, TimelineFramesModel::ActiveFrameRole);
    }
}

QItemSelectionModel::SelectionFlags TimelineFramesView::selectionCommand(const QModelIndex &index,
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

void TimelineFramesView::slotSelectionChanged()
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

    KisTimeRange range;
    if (maxColumn > minColumn) {
        range = KisTimeRange(minColumn, maxColumn - minColumn + 1);
    }
    m_d->model->setPlaybackRange(range);
}

void TimelineFramesView::slotReselectCurrentIndex()
{
    QModelIndex index = currentIndex();
    currentChanged(index, index);
}

void TimelineFramesView::slotEnsureRowVisible(int row)
{
    QModelIndex index = currentIndex();
    if (!index.isValid() || row < 0) return;

    index = m_d->model->index(row, index.column());
    scrollTo(index);
}

void TimelineFramesView::slotDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (m_d->model->isPlaybackActive()) return;

    int selectedColumn = -1;

    for (int j = topLeft.column(); j <= bottomRight.column(); j++) {
        QVariant value = m_d->model->data(
                    m_d->model->index(topLeft.row(), j),
                    TimelineFramesModel::ActiveFrameRole);

        if (value.isValid() && value.toBool()) {
            selectedColumn = j;
            break;
        }
    }

    QModelIndex index = currentIndex();

    if (!index.isValid() && selectedColumn < 0) {
        return;
    }


    if (selectedColumn == -1) {
        selectedColumn = index.column();
    }

    if (selectedColumn != index.column() && !m_d->dragInProgress) {
        int row= index.isValid() ? index.row() : 0;
        selectionModel()->setCurrentIndex(m_d->model->index(row, selectedColumn), QItemSelectionModel::ClearAndSelect);
    }
}

void TimelineFramesView::slotHeaderDataChanged(Qt::Orientation orientation, int first, int last)
{
    Q_UNUSED(first);
    Q_UNUSED(last);

    if (orientation == Qt::Horizontal) {
        const int newFps = m_d->model->headerData(0, Qt::Horizontal, TimelineFramesModel::FramesPerSecondRole).toInt();

        if (newFps != m_d->fps) {
            setFramesPerSecond(newFps);
        }
    }
}

void TimelineFramesView::rowsInserted(const QModelIndex& parent, int start, int end)
{
    QTableView::rowsInserted(parent, start, end);
}

inline bool isIndexDragEnabled(QAbstractItemModel *model, const QModelIndex &index) {
    return (model->flags(index) & Qt::ItemIsDragEnabled);
}

QStyleOptionViewItem TimelineFramesView::Private::viewOptionsV4() const
{
    QStyleOptionViewItem option = q->viewOptions();
    option.locale = q->locale();
    option.locale.setNumberOptions(QLocale::OmitGroupSeparator);
    option.widget = q;
    return option;
}

QItemViewPaintPairs TimelineFramesView::Private::draggablePaintPairs(const QModelIndexList &indexes, QRect *r) const
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

QPixmap TimelineFramesView::Private::renderToPixmap(const QModelIndexList &indexes, QRect *r) const
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

void TimelineFramesView::startDrag(Qt::DropActions supportedActions)
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

void TimelineFramesView::dragEnterEvent(QDragEnterEvent *event)
{
    m_d->dragInProgress = true;
    m_d->model->setScrubState(true);

    QTableView::dragEnterEvent(event);
}

void TimelineFramesView::dragMoveEvent(QDragMoveEvent *event)
{
    m_d->dragInProgress = true;
    m_d->model->setScrubState(true);

    QTableView::dragMoveEvent(event);

    if (event->isAccepted()) {
        QModelIndex index = indexAt(event->pos());
        if (!m_d->model->canDropFrameData(event->mimeData(), index)) {
            event->ignore();
        } else {
            selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
        }
    }
}

void TimelineFramesView::dropEvent(QDropEvent *event)
{
    m_d->dragInProgress = false;
    m_d->model->setScrubState(false);

    if (event->keyboardModifiers() & Qt::ControlModifier) {
        event->setDropAction(Qt::CopyAction);
    }
    QAbstractItemView::dropEvent(event);
    m_d->dragWasSuccessful = event->isAccepted();
}

void TimelineFramesView::dragLeaveEvent(QDragLeaveEvent *event)
{
    m_d->dragInProgress = false;
    m_d->model->setScrubState(false);

    QAbstractItemView::dragLeaveEvent(event);
}

void TimelineFramesView::createFrameEditingMenuActions(QMenu *menu, bool addFrameCreationActions)
{
    slotUpdateFrameActions();

    // calculate if selection range is set. This will determine if the update playback range is available
    QSet<int> rows;
    int minColumn = 0;
    int maxColumn = 0;
    calculateSelectionMetrics(minColumn, maxColumn, rows);

    bool selectionExists = minColumn != maxColumn;


    if (selectionExists) {
        KisActionManager::safePopulateMenu(menu, "update_playback_range", m_d->actionMan);
    } else {
        KisActionManager::safePopulateMenu(menu, "set_start_time", m_d->actionMan);
        KisActionManager::safePopulateMenu(menu, "set_end_time", m_d->actionMan);
    }

    menu->addSeparator();

    KisActionManager::safePopulateMenu(menu, "cut_frames_to_clipboard", m_d->actionMan);
    KisActionManager::safePopulateMenu(menu, "copy_frames_to_clipboard", m_d->actionMan);
    KisActionManager::safePopulateMenu(menu, "paste_frames_from_clipboard", m_d->actionMan);

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

    KisActionManager::safePopulateMenu(menu, "remove_frames", m_d->actionMan);
    KisActionManager::safePopulateMenu(menu, "remove_frames_and_pull", m_d->actionMan);

    menu->addSeparator();

    if (addFrameCreationActions) {
        KisActionManager::safePopulateMenu(menu, "add_blank_frame", m_d->actionMan);
        KisActionManager::safePopulateMenu(menu, "add_duplicate_frame", m_d->actionMan);
        menu->addSeparator();
    }
}

void TimelineFramesView::mousePressEvent(QMouseEvent *event)
{
    QPersistentModelIndex index = indexAt(event->pos());

    if (m_d->modifiersCatcher->modifierPressed("pan-zoom")) {

        if (event->button() == Qt::RightButton) {
            // TODO: try calculate index under mouse cursor even when
            //       it is outside any visible row
            qreal staticPoint = index.isValid() ? index.column() : currentIndex().column();
            m_d->zoomDragButton->beginZoom(event->pos(), staticPoint);
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
                m_d->model->data(index, TimelineFramesModel::FrameEditableRole).toBool()) {

            model()->setData(index, true, TimelineFramesModel::ActiveLayerRole);
            model()->setData(index, true, TimelineFramesModel::ActiveFrameRole);
            setCurrentIndex(index);

            if (model()->data(index, TimelineFramesModel::FrameExistsRole).toBool() ||
                    model()->data(index, TimelineFramesModel::SpecialKeyframeExists).toBool()) {

                {
                    KisSignalsBlocker b(m_d->colorSelector);
                    QVariant colorLabel = index.data(TimelineFramesModel::FrameColorLabelIndexRole);
                    int labelIndex = colorLabel.isValid() ? colorLabel.toInt() : 0;
                    m_d->colorSelector->setCurrentIndex(labelIndex);
                }

                QMenu menu;
                createFrameEditingMenuActions(&menu, false);
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
                createFrameEditingMenuActions(&menu, true);

                menu.addSeparator();
                menu.addAction(m_d->colorSelectorAction);
                menu.exec(event->globalPos());
            }
        } else if (numSelectedItems > 1) {
            int labelIndex = -1;
            bool haveFrames = false;
            Q_FOREACH(QModelIndex index, selectedIndexes()) {
                haveFrames |= index.data(TimelineFramesModel::FrameExistsRole).toBool();
                QVariant colorLabel = index.data(TimelineFramesModel::FrameColorLabelIndexRole);
                if (colorLabel.isValid()) {
                    if (labelIndex == -1) {
                        // First label
                        labelIndex = colorLabel.toInt();
                    } else if (labelIndex != colorLabel.toInt()) {
                        // Mixed colors in selection
                        labelIndex = -1;
                        break;
                    }
                }
            }

            if (haveFrames) {
                KisSignalsBlocker b(m_d->multiframeColorSelector);
                m_d->multiframeColorSelector->setCurrentIndex(labelIndex);
            }

            QMenu menu;
            createFrameEditingMenuActions(&menu, false);
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

        QAbstractItemView::mousePressEvent(event);
    }
}

void TimelineFramesView::mouseMoveEvent(QMouseEvent *e)
{
    if (m_d->modifiersCatcher->modifierPressed("pan-zoom")) {

        if (e->buttons() & Qt::RightButton) {
            m_d->zoomDragButton->continueZoom(e->pos());
        } else if (e->buttons() & Qt::LeftButton) {
            QPoint diff = e->pos() - m_d->initialDragPanPos;
            QPoint offset = QPoint(m_d->initialDragPanValue.x() - diff.x(),
                                   m_d->initialDragPanValue.y() - diff.y());

            const int height = m_d->layersHeader->defaultSectionSize();

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

void TimelineFramesView::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_d->modifiersCatcher->modifierPressed("pan-zoom")) {
        e->accept();
    } else {
        m_d->model->setScrubState(false);
        QTableView::mouseReleaseEvent(e);
    }
}

void TimelineFramesView::wheelEvent(QWheelEvent *e)
{
    QModelIndex index = currentIndex();
    int column= -1;

    if (index.isValid()) {
        column= index.column() + ((e->delta() > 0) ? 1 : -1);
    }

    if (column >= 0 && !m_d->dragInProgress) {
        setCurrentIndex(m_d->model->index(index.row(), column));
    }
}

void TimelineFramesView::slotUpdateLayersMenu()
{
    QAction *action = 0;

    m_d->existingLayersMenu->clear();

    QVariant value = model()->headerData(0, Qt::Vertical, TimelineFramesModel::OtherLayersRole);
    if (value.isValid()) {
        TimelineFramesModel::OtherLayersList list = value.value<TimelineFramesModel::OtherLayersList>();

        int i = 0;
        Q_FOREACH (const TimelineFramesModel::OtherLayer &l, list) {
            action = m_d->existingLayersMenu->addAction(l.name);
            action->setData(i++);
        }
    }
}

void TimelineFramesView::slotUpdateFrameActions()
{
    if (!m_d->actionMan) return;

    const QModelIndexList editableIndexes = calculateSelectionSpan(false, true);
    const bool hasEditableFrames = !editableIndexes.isEmpty();

    bool hasExistingFrames = false;
    Q_FOREACH (const QModelIndex &index, editableIndexes) {
        if (model()->data(index, TimelineFramesModel::FrameExistsRole).toBool()) {
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

    enableAction("copy_frames_to_clipboard", true);
    enableAction("cut_frames_to_clipboard", hasEditableFrames);

    QClipboard *cp = QApplication::clipboard();
    const QMimeData *data = cp->mimeData();

    enableAction("paste_frames_from_clipboard", data && data->hasFormat("application/x-krita-frame"));

    //TODO: update column actions!
}

void TimelineFramesView::slotSetStartTimeToCurrentPosition()
{
     m_d->model->setFullClipRangeStart(this->currentIndex().column());
}

void TimelineFramesView::slotSetEndTimeToCurrentPosition()
{
    m_d->model->setFullClipRangeEnd(this->currentIndex().column());
}

void TimelineFramesView::slotUpdatePlackbackRange()
{
    QSet<int> rows;
    int minColumn = 0;
    int maxColumn = 0;

    calculateSelectionMetrics(minColumn, maxColumn, rows);

    m_d->model->setFullClipRangeStart(minColumn);
    m_d->model->setFullClipRangeEnd(maxColumn);
}

void TimelineFramesView::slotLayerContextMenuRequested(const QPoint &globalPos)
{
    m_d->layerEditingMenu->exec(globalPos);
}

void TimelineFramesView::slotAddNewLayer()
{
    QModelIndex index = currentIndex();
    const int newRow = index.isValid() ? index.row() : 0;
    model()->insertRow(newRow);
}

void TimelineFramesView::slotAddExistingLayer(QAction *action)
{
    QVariant value = action->data();

    if (value.isValid()) {
        QModelIndex index = currentIndex();
        const int newRow = index.isValid() ? index.row() + 1 : 0;

        m_d->model->insertOtherLayer(value.toInt(), newRow);
    }
}

void TimelineFramesView::slotRemoveLayer()
{
    QModelIndex index = currentIndex();
    if (!index.isValid()) return;

    model()->removeRow(index.row());
}

void TimelineFramesView::slotAddBlankFrame()
{
    QModelIndex index = currentIndex();
    if (!index.isValid() ||
        !m_d->model->data(index, TimelineFramesModel::FrameEditableRole).toBool()) {

        return;
    }

    m_d->model->createFrame(index);
}

void TimelineFramesView::slotAddDuplicateFrame()
{
    QModelIndex index = currentIndex();
    if (!index.isValid() ||
        !m_d->model->data(index, TimelineFramesModel::FrameEditableRole).toBool()) {

        return;
    }

    m_d->model->copyFrame(index);
}

void TimelineFramesView::calculateSelectionMetrics(int &minColumn, int &maxColumn, QSet<int> &rows) const
{
    minColumn = std::numeric_limits<int>::max();
    maxColumn = std::numeric_limits<int>::min();

    Q_FOREACH (const QModelIndex &index, selectionModel()->selectedIndexes()) {
        if (!m_d->model->data(index, TimelineFramesModel::FrameEditableRole).toBool()) continue;

        rows.insert(index.row());
        minColumn = qMin(minColumn, index.column());
        maxColumn = qMax(maxColumn, index.column());
    }
}

void TimelineFramesView::insertKeyframes(int count, int timing, TimelineDirection direction, bool entireColumn)
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
            if (!m_d->model->data(m_d->model->index(i, insertionColumn), TimelineFramesModel::FrameEditableRole).toBool()) continue;
            rows.insert(i);
        }
    }

    if (!rows.isEmpty()) {
        m_d->model->insertFrames(insertionColumn, rows.toList(), count, timing);
    }
}

void TimelineFramesView::insertMultipleKeyframes(bool entireColumn)
{
    int count, timing;
    TimelineDirection direction;

    if (m_d->insertKeyframeDialog->promptUserSettings(count, timing, direction)) {
        insertKeyframes(count, timing, direction, entireColumn);
    }
}

QModelIndexList TimelineFramesView::calculateSelectionSpan(bool entireColumn, bool editableOnly) const
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
                !m_d->model->data(m_d->model->index(i, minColumn), TimelineFramesModel::FrameEditableRole).toBool()) continue;

            for (int column = minColumn; column <= maxColumn; column++) {
                indexes << m_d->model->index(i, column);
            }
        }
    } else {
        Q_FOREACH (const QModelIndex &index, selectionModel()->selectedIndexes()) {
            if (!editableOnly || m_d->model->data(index, TimelineFramesModel::FrameEditableRole).toBool()) {
                indexes << index;
            }
        }
    }

    return indexes;
}

void TimelineFramesView::slotRemoveSelectedFrames(bool entireColumn, bool pull)
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

void TimelineFramesView::insertOrRemoveHoldFrames(int count, bool entireColumn)
{
    QModelIndexList indexes;

    if (!entireColumn) {
        Q_FOREACH (const QModelIndex &index, selectionModel()->selectedIndexes()) {
            if (m_d->model->data(index, TimelineFramesModel::FrameEditableRole).toBool()) {
                indexes << index;
            }
        }
    } else {
        const int column = selectionModel()->currentIndex().column();

        for (int i = 0; i < m_d->model->rowCount(); i++) {
            const QModelIndex index = m_d->model->index(i, column);
            if (m_d->model->data(index, TimelineFramesModel::FrameEditableRole).toBool()) {
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

                KisKeyframeChannel *channel = layerNode->getKeyframeChannel(KisKeyframeChannel::Content.id());
                if (!channel) continue;

                if (keyframesInLayerNode < channel->allKeyframeIds().count()) {
                   keyframesInLayerNode = channel->allKeyframeIds().count();
                }
            }
            m_d->model->setLastVisibleFrame(m_d->model->columnCount() + count*keyframesInLayerNode);
        }


        m_d->model->insertHoldFrames(indexes, count);

        // bulk adding frames can add too many
        // trim timeline to clean up extra frames that might have been added
        slotUpdateInfiniteFramesCount();
    }
}

void TimelineFramesView::insertOrRemoveMultipleHoldFrames(bool insertion, bool entireColumn)
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

void TimelineFramesView::slotMirrorFrames(bool entireColumn)
{
    const QModelIndexList indexes = calculateSelectionSpan(entireColumn);

    if (!indexes.isEmpty()) {
        m_d->model->mirrorFrames(indexes);
    }
}

void TimelineFramesView::cutCopyImpl(bool entireColumn, bool copy)
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
                                                       TimelineFramesModel::CopyFramesPolicy :
                                                       TimelineFramesModel::MoveFramesPolicy);

    if (data) {
        QClipboard *cb = QApplication::clipboard();
        cb->setMimeData(data);
    }
}

void TimelineFramesView::slotPasteFrames(bool entireColumn)
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

bool TimelineFramesView::viewportEvent(QEvent *event)
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
