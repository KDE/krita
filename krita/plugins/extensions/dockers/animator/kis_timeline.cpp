/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License, or (at your option)
 *  any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_timeline.h"

#include <KisPart.h>

#include <kactioncollection.h>

#include <QToolButton>
#include <QToolBar>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QCheckBox>
#include <QSplitter>
#include <KoIcon.h>
#include <QScrollArea>
#include <QMenu>
#include <QScrollBar>
#include <KoIcon.h>

#include<kis_canvas2.h>
#include<KisViewManager.h>
#include<KisDocument.h>
#include<kis_animation_doc.h>
#include<kis_image.h>
#include<kis_debug.h>
#include<kis_animation.h>
#include<kis_action_manager.h>
#include<kis_action.h>
#include <kis_animation_model.h>

#include "TimelineView.h"
#include "kis_frame_box.h"
#include "kis_animation_layerbox.h"
#include "kis_animation_frame_widget.h"
#include "animator_settings_dialog.h"


KisTimelineWidget::KisTimelineWidget(QWidget *parent)
    : QWidget(parent)
    , m_animation(0)
    , m_timeline(0)
    , m_animationLayerBox(0)
{
    m_settingsDialog = new AnimatorSettingsDialog(this);
    m_playbackDialog = new AnimatorPlaybackDialog(this);

}

void KisTimelineWidget::init()
{
    KActionCollection* actionCollection = m_canvas->viewManager()->actionCollection();
    KisActionManager* actionManager = m_canvas->viewManager()->actionManager();

    m_lastBrokenFrame = QRect();
    m_frameBreakState = false;

    QWidget* leftWidget = new QWidget();
    leftWidget->setMinimumWidth(120);
    QWidget* rightWidget = new QWidget();

    QWidget* leftToolBar = new QWidget();
    leftToolBar->setFixedHeight(31);
    QWidget* rightToolBar = new QWidget();
    rightToolBar->setFixedHeight(31);

    QToolBar* layerButtons = new QToolBar(this);

    QToolButton* addLayerButton = new QToolButton(this);
    addLayerButton->setIcon(themedIcon("addlayer"));
    addLayerButton->setToolTip(i18n("Add Animation Layer"));

    KisAction* addPaintLayerAction = new KisAction(koIcon("document-new"), i18n("Add Animation Paint Layer"), this);
    actionManager->addAction("add_animation_paint_layer", addPaintLayerAction);
    connect(addPaintLayerAction, SIGNAL(triggered()), this, SLOT(paintLayerPressed()));

    KisAction* addVectorLayerAction = new KisAction(koIcon("bookmark-new"), i18n("Add Animation Vector Layer"), this);
    actionManager->addAction("add_animation_vector_layer", addVectorLayerAction);
    connect(addVectorLayerAction, SIGNAL(triggered()), this, SLOT(vectorLayerPressed()));

    QMenu* layerMenu = new QMenu(i18n("Add Animation Layer"), this);
    layerMenu->addAction(addPaintLayerAction);
    layerMenu->addAction(addVectorLayerAction);
    addLayerButton->setMenu(layerMenu);
    addLayerButton->setPopupMode(QToolButton::InstantPopup);

    KisAction* removeLayerAction = new KisAction(themedIcon("deletelayer"), i18n("Remove Animation Layer"), this);
    actionManager->addAction("remove_animation_layer", removeLayerAction);
    connect(removeLayerAction, SIGNAL(triggered()), this, SLOT(removeLayerPressed()));

    KisAction* layerUpAction = new KisAction(themedIcon("arrowupblr"), i18n("Move animation layer up"), this);
    actionManager->addAction("move_animation_layer_up", layerUpAction);
    connect(layerUpAction, SIGNAL(triggered()), this, SLOT(layerUpPressed()));

    KisAction* layerDownAction = new KisAction(themedIcon("arrowdown"), i18n("Move animation layer down"), this);
    actionManager->addAction("move_animation_layer_down", layerDownAction);
    connect(layerDownAction, SIGNAL(triggered()), this, SLOT(layerDownPressed()));

    layerButtons->addWidget(addLayerButton);
    layerButtons->addAction(removeLayerAction);
    layerButtons->addAction(layerUpAction);
    layerButtons->addAction(layerDownAction);

    QHBoxLayout* leftToolBarLayout = new QHBoxLayout();
    leftToolBarLayout->setAlignment(Qt::AlignLeft);
    leftToolBarLayout->setMargin(0);
    leftToolBarLayout->addWidget(layerButtons);
    leftToolBar->setLayout(leftToolBarLayout);

    QScrollArea* leftScrollArea = new QScrollArea(this);
    leftScrollArea->setBackgroundRole(QPalette::Dark);
    m_animationLayerBox = new KisAnimationLayerBox(this);
    leftScrollArea->setWidget(m_animationLayerBox);
    m_animationLayerBox->setFixedHeight(45);
    m_animationLayerBox->setFixedWidth(200);
    leftScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    leftScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QGridLayout* leftLayout = new QGridLayout();
    leftLayout->addWidget(leftToolBar, 1, 0);
    leftLayout->addWidget(leftScrollArea, 0, 0);
    leftLayout->setMargin(0);
    leftLayout->setSpacing(0);
    leftWidget->setLayout(leftLayout);

    QToolBar* frameButtons = new QToolBar(this);

    KisAction* addKeyFrameAction = new KisAction(koIcon("list-add"), i18n("Insert Keyframe"), this);
    actionManager->addAction("insert_key_frame", addKeyFrameAction);
    connect(addKeyFrameAction, SIGNAL(triggered()), this, SLOT(keyFramePressed()));

    KisAction* addBlankFrameAction = new KisAction(koIcon("list-add"), i18n("Insert Blank Frame"), this);
    actionManager->addAction("insert_blank_frame", addBlankFrameAction);
    connect(addBlankFrameAction, SIGNAL(triggered()), this, SLOT(blankFramePressed()));

    KisAction* removeFrameAction = new KisAction(koIcon("list-remove"), i18n("Remove Frame"), this);
    actionManager->addAction("remove_frame", removeFrameAction);
    connect(removeFrameAction, SIGNAL(triggered()), this, SLOT(removeFramePressed()));

    frameButtons->addAction(addKeyFrameAction);
    frameButtons->addAction(addBlankFrameAction);
    frameButtons->addAction(removeFrameAction);

    QToolBar* navToolBar = new QToolBar(this);

    KisAction* nextFrameAction = new KisAction(koIcon("go-next-view"), i18n("Next Frame"), this);
    actionManager->addAction("next_frame", nextFrameAction);
    connect(nextFrameAction, SIGNAL(triggered()), this, SLOT(nextFramePressed()));

    KisAction* prevFrameAction = new KisAction(koIcon("go-previous-view"), i18n("Previous Frame"), this);
    actionManager->addAction("previous_frame", prevFrameAction);
    connect(prevFrameAction, SIGNAL(triggered()), this, SLOT(prevFramePressed()));

    KisAction* prevKeyFrameAction = new KisAction(koIcon("go-previous-content"), i18n("Previous Keyframe"), this);
    actionManager->addAction("previous_key_frame", prevKeyFrameAction);
    connect(prevKeyFrameAction, SIGNAL(triggered()), this, SLOT(prevKeyFramePressed()));

    KisAction* nextKeyFrameAction = new KisAction(koIcon("go-next-content"), i18n("Next Keyframe"), this);
    actionManager->addAction("next_key_frame", nextKeyFrameAction);
    connect(nextKeyFrameAction, SIGNAL(triggered()), this, SLOT(nextKeyFramePressed()));

    navToolBar->addAction(prevKeyFrameAction);
    navToolBar->addAction(prevFrameAction);
    navToolBar->addAction(nextFrameAction);
    navToolBar->addAction(nextKeyFrameAction);

    QToolBar* playerButtons = new QToolBar(this);

    KisAction* playAction = new KisAction((KIcon)style()->standardIcon(QStyle::SP_MediaPlay), i18n("Play Animation"), this);
    actionManager->addAction("play_animation", playAction);
    connect(playAction, SIGNAL(triggered()), this, SLOT(playAnimation()));

    KisAction* pauseAction = new KisAction((KIcon)style()->standardIcon(QStyle::SP_MediaPause), i18n("Pause Animation"), this);
    actionManager->addAction("pause_animation", pauseAction);
    connect(pauseAction, SIGNAL(triggered()), this, SLOT(pauseAnimation()));

    KisAction* stopAction = new KisAction((KIcon)style()->standardIcon(QStyle::SP_MediaStop), i18n("Stop Animation"), this);
    actionManager->addAction("stop_animation", stopAction);
    connect(stopAction, SIGNAL(triggered()), this, SLOT(stopAnimation()));

    QToolButton* playbackOptions = new QToolButton(this);
    playbackOptions->setIcon(koIcon("configure"));
    playbackOptions->setToolTip(i18n("Settings"));
    connect(playbackOptions, SIGNAL(clicked()), this, SLOT(playbackOptionsPressed()));

    playerButtons->addAction(playAction);
    playerButtons->addAction(pauseAction);
    playerButtons->addAction(stopAction);
    playerButtons->addWidget(playbackOptions);

    QToolBar* settingsToolBar = new QToolBar(this);

    QCheckBox* frameBreakState = new QCheckBox(this);
    frameBreakState->setText(i18n("Add blank frame "));

    QToolButton* settingsButton = new QToolButton(this);
    settingsButton->setIcon(koIcon("configure"));
    settingsButton->setToolTip(i18n("Settings"));

    settingsToolBar->addWidget(frameBreakState);
    settingsToolBar->addWidget(settingsButton);

    connect(frameBreakState, SIGNAL(clicked(bool)), this, SLOT(frameBreakStateChanged(bool)));
    connect(settingsButton, SIGNAL(clicked()), this, SLOT(settingsButtonPressed()));

    QHBoxLayout* rightToolBarLayout = new QHBoxLayout();
    rightToolBarLayout->addWidget(frameButtons);
    rightToolBarLayout->addWidget(navToolBar);
    rightToolBarLayout->addWidget(playerButtons);
    rightToolBarLayout->addWidget(settingsToolBar);
    rightToolBar->setLayout(rightToolBarLayout);

    m_timeline = new TimelineView(this);
    // connect(m_cells, SIGNAL(frameSelectionChanged(QRect)), this, SLOT(frameSelectionChanged(QRect)));

    QGridLayout* rightLayout = new QGridLayout();
    rightLayout->addWidget(rightToolBar, 1, 0);
    rightLayout->addWidget(m_timeline, 0, 0);
    rightLayout->setMargin(0);
    rightLayout->setSpacing(0);
    rightWidget->setLayout(rightLayout);

    QSplitter* splitter = new QSplitter(this);
    splitter->addWidget(leftWidget);
    splitter->addWidget(rightWidget);
    splitter->setSizes(QList<int>() << 140 << 600);

    QGridLayout* lay = new QGridLayout();

    lay->addWidget(splitter, 0, 0);
    lay->setMargin(0);
    lay->setSpacing(0);
    setLayout(lay);

    connect(m_settingsDialog, SIGNAL(sigTimelineWithChanged(int)), this, SLOT(timelineWidthChanged(int)));

    m_imported = false;
}

void KisTimelineWidget::frameSelectionChanged(QRect frame)
{
    dynamic_cast<KisAnimationDoc*>(m_canvas->viewManager()->document())->frameSelectionChanged(frame);
}

void KisTimelineWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
}

void KisTimelineWidget::setCanvas(KisCanvas2 *canvas)
{
    Q_ASSERT(canvas);
    m_canvas = canvas;

    if (!m_timeline) {
        init();
    }
    KisAnimationDoc *doc = qobject_cast<KisAnimationDoc*>(canvas->viewManager()->document());
    // TODO: workaround to not crash when loading normal images
    if (!doc) {
        return;
    }
    Q_ASSERT(doc);
    m_timeline->setModel(new KisAnimationModel(doc));

    // Connect all the document signals here
    connect(doc, SIGNAL(sigFrameModified()), this, SLOT(documentModified()));
    connect(doc, SIGNAL(sigImportFinished(QHash<int, QList<QRect> >)), this, SLOT(importUI(QHash<int, QList<QRect> >)));
    connect(m_playbackDialog, SIGNAL(playbackStateChanged()), doc, SLOT(playbackStateChanged()));
}

void KisTimelineWidget::unsetCanvas()
{

}

void KisTimelineWidget::setAnimation(KisAnimation *animation)
{
    m_animation = animation;
    m_settingsDialog->setModel(animation);
    m_playbackDialog->setModel(animation);
}

KisCanvas2* KisTimelineWidget::getCanvas()
{
    return m_canvas;
}

void KisTimelineWidget::addLayerUiUpdate()
{
    m_animationLayerBox->addLayerUiUpdate();
    //m_cells->addLayerUiUpdate();
}

void KisTimelineWidget::removeLayerUiUpdate(int layer)
{
    m_animationLayerBox->removeLayerUiUpdate(layer);
    //m_cells->removeLayerUiUpdate(layer);
}

void KisTimelineWidget::moveLayerDownUiUpdate(int layer)
{
    m_animationLayerBox->moveLayerDownUiUpdate(layer);
    //m_cells->moveLayerDownUiUpdate(layer);
}

void KisTimelineWidget::moveLayerUpUiUpdate(int layer)
{
    m_animationLayerBox->moveLayerUpUiUpdate(layer);
    //m_cells->moveLayerUpUiUpdate(layer);
}

void KisTimelineWidget::paintLayerPressed()
{
    addLayerUiUpdate();
    dynamic_cast<KisAnimationDoc*>(getCanvas()->viewManager()->document())->addPaintLayer();
}

void KisTimelineWidget::vectorLayerPressed()
{
    addLayerUiUpdate();
    dynamic_cast<KisAnimationDoc*>(getCanvas()->viewManager()->document())->addVectorLayer();
}

void KisTimelineWidget::removeLayerPressed()
{
    if (m_animationLayerBox->numberOfLayers() == 1) {
        return;
    }

//    if (m_cells->getSelectedFrame()) {
//        int layer = m_cells->getSelectedFrame()->getParent()->getLayerIndex();

//        // Refresh timeline
//        removeLayerUiUpdate(layer);

//        dynamic_cast<KisAnimationDoc*>(m_canvas->view()->document())->removeLayer(layer * 20);
//    }
}

void KisTimelineWidget::layerDownPressed()
{
//    if (m_cells->getSelectedFrame()) {
//        int layer = m_cells->getSelectedFrame()->getParent()->getLayerIndex();

//        // If it is the bottom most layer
//        if (layer == 0) {
//            return;
//        }

//        // Refresh the timeline
//        moveLayerDownUiUpdate(layer);

//        dynamic_cast<KisAnimationDoc*>(m_canvas->view()->document())->moveLayerDown(layer * 20);
//    }
}

void KisTimelineWidget::layerUpPressed()
{
//    if (m_cells->getSelectedFrame()) {
//        int layer = m_cells->getSelectedFrame()->getParent()->getLayerIndex();

//        // If it is the top most layer
//        if (layer == numberOfLayers() - 1) {
//            return;
//        }

//        // Refresh the timeline
//        moveLayerUpUiUpdate(layer);

//        dynamic_cast<KisAnimationDoc*>(m_canvas->view()->document())->moveLayerUp(layer * 20);
//    }
}

void KisTimelineWidget::blankFramePressed()
{
//    if (m_cells->getSelectedFrame()) {
//        QRect globalGeometry = m_cells->getSelectedFrame()->convertSelectionToFrame();

//        if (globalGeometry == QRect()) {
//            return;
//        }

//        m_cells->setSelectedFrame();
//        dynamic_cast<KisAnimationDoc*>(m_canvas->view()->document())->addBlankFrame(globalGeometry);
//    }
}

void KisTimelineWidget::keyFramePressed()
{
//    if (m_cells->getSelectedFrame()) {
//        QRect globalGeometry = m_cells->getSelectedFrame()->convertSelectionToFrame();

//        if (globalGeometry == QRect()) {
//            return;
//        }

//        m_cells->setSelectedFrame();
//        dynamic_cast<KisAnimationDoc*>(m_canvas->view()->document())->addKeyFrame(globalGeometry);
//    }
}

void KisTimelineWidget::removeFramePressed()
{
//    if (m_cells->getSelectedFrame()) {
//        QRect globalGeometry = m_cells->getSelectedFrame()->removeFrame();
//        m_cells->setSelectedFrame();
//        dynamic_cast<KisAnimationDoc*>(m_canvas->view()->document())->removeFrame(globalGeometry);
//    }
}

void KisTimelineWidget::frameBreakStateChanged(bool state)
{
    m_frameBreakState = state;
}

void KisTimelineWidget::breakFrame(QRect position)
{
//    if (m_lastBrokenFrame.x() == position.x() && m_lastBrokenFrame.y() == position.y()) {
//        return;
//    }

//    QRect globalGeometry = m_cells->getSelectedFrame()->convertSelectionToFrame();
//    m_cells->setSelectedFrame();

//    m_lastBrokenFrame = position;
//    dynamic_cast<KisAnimationDoc*>(getCanvas()->view()->document())->breakFrame(globalGeometry, m_frameBreakState);
}

void KisTimelineWidget::nextFramePressed()
{
    //m_cells->setSelectedFrame(m_cells->getSelectedFrame()->geometry().x() + 10);
}

void KisTimelineWidget::prevFramePressed()
{
    //m_cells->setSelectedFrame(m_cells->getSelectedFrame()->geometry().x() - 10);
}

void KisTimelineWidget::nextKeyFramePressed()
{
//    KisAnimationFrameWidget* currSelection = m_cells->getSelectedFrame();

//    QRect nextKeyFrame = dynamic_cast<KisAnimationDoc*>(m_canvas->view()->document())->getNextKeyFramePosition(currSelection->x(),
//                         currSelection->getParent()->getLayerIndex() * 20);
//    m_cells->setSelectedFrame(nextKeyFrame.x());
}

void KisTimelineWidget::prevKeyFramePressed()
{
//    KisAnimationFrameWidget* currSelection = m_cells->getSelectedFrame();

//    QRect prevKeyFrame = dynamic_cast<KisAnimationDoc*>(m_canvas->view()->document())->getPreviousKeyFramePosition(currSelection->x(),
//                         currSelection->getParent()->getLayerIndex() * 20);

//    m_cells->setSelectedFrame(prevKeyFrame.x());
}

void KisTimelineWidget::settingsButtonPressed()
{
    m_settingsDialog->setVisible(true);
}

void KisTimelineWidget::playbackOptionsPressed()
{
    m_playbackDialog->setVisible(true);
}

void KisTimelineWidget::playAnimation()
{
    dynamic_cast<KisAnimationDoc*>(m_canvas->viewManager()->document())->play();
}

void KisTimelineWidget::pauseAnimation()
{
    dynamic_cast<KisAnimationDoc*>(m_canvas->viewManager()->document())->pause();
}

void KisTimelineWidget::stopAnimation()
{
    dynamic_cast<KisAnimationDoc*>(m_canvas->viewManager()->document())->stop();
}

void KisTimelineWidget::timelineWidthChanged(int width)
{
//    m_cells->setFixedWidth(width * 10);
}

void KisTimelineWidget::documentModified()
{
    emit canvasModified();
//    KisAnimationFrameWidget* selectedFrame = m_cells->getSelectedFrame();
//    if (selectedFrame) {
//        if (m_animation->frameBreakingEnabled()) {
//            breakFrame(QRect(selectedFrame->x(), selectedFrame->y(), 10, 20));
//        }
//    }
}

void KisTimelineWidget::importUI(QHash<int, QList<QRect> > timelineMap)
{
    if (m_imported) {
        return;
    }

    QList<int> layers = timelineMap.keys();

    qSort(layers);

    KisAnimationFrameWidget* oldSelection;

//    for (int i = 0 ; i < layers.size() ; i++) {
//        int layer = layers.at(i);

//        // No layer update UI since for layer 0, UI is already present
//        if (layer != 0) {
//            addLayerUiUpdate();
//        }

//        // Gets the first frame of the layer which is selected by default
//        oldSelection = m_cells->getSelectedFrame();

//        QList<QRect> frames = timelineMap[layer];

//        for (int j = 0 ; j < frames.size() ; j++) {
//            m_cells->m_selectedFrame->hide();

//            m_cells->m_selectedFrame = new KisAnimationFrameWidget(oldSelection->getParent(), KisAnimationFrameWidget::SELECTION, 10);
//            m_cells->m_selectedFrame->setGeometry(frames.at(j).x(), 0, 10, 20);

//            m_cells->getSelectedFrame()->show();
//            m_cells->getSelectedFrame()->convertSelectionToFrame();

//        }
//    }

    m_imported = true;
}

int KisTimelineWidget::numberOfLayers()
{
    return m_animationLayerBox->numberOfLayers();
}
