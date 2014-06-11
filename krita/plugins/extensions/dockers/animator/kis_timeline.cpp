/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or(at you option)
 *  any later version..
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_timeline.h"
#include "kis_frame_box.h"
#include "kis_animation_layerbox.h"
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
#include "kis_animation_frame.h"
#include "animator_settings_dialog.h"
#include <kis_canvas2.h>
#include <kis_view2.h>
#include <kis_doc2.h>
#include <kis_animation_doc.h>
#include <kis_image.h>
#include <kis_debug.h>
#include <KoPart.h>
#include <kis_animation.h>
#include <KFileDialog>
#include <KActionCollection>
#include <kis_action_manager.h>
#include <kis_action.h>

KisTimeline::KisTimeline(QWidget *parent) : QWidget(parent)
{
    m_initialized = false;
    m_parent = parent;
}

void KisTimeline::init()
{
    m_list = new KisAnimationLayerBox(this);
    m_cells = new KisFrameBox(this);
    m_settingsDialog = new AnimatorSettingsDialog();

    KActionCollection* actionCollection = m_canvas->view()->actionCollection();
    KisActionManager* actionManager = m_canvas->view()->actionManager();

    this->m_numberOfLayers = 0;
    this->m_lastBrokenFrame = QRect();

    this->m_frameBreakState = false;

    QWidget* leftWidget = new QWidget();
    leftWidget->setMinimumWidth(120);
    QWidget* rightWidget = new QWidget();

    QWidget* leftToolBar = new QWidget();
    leftToolBar->setFixedHeight(31);
    QWidget* rightToolBar = new QWidget();
    rightToolBar->setFixedHeight(31);

    QToolBar* layerButtons = new QToolBar(this);

    QToolButton* addLayerButton = new QToolButton(this);
    addLayerButton->setIcon(koIcon("list-add"));
    addLayerButton->setToolTip("Add Animation Layer");

    KisAction* addPaintLayerAction = new KisAction(koIcon("list-add"), tr("Add Animation Paint Layer"), this);
    actionManager->addAction("add_animation_paint_layer", addPaintLayerAction, actionCollection);
    connect(addPaintLayerAction, SIGNAL(triggered()), this, SLOT(paintLayerPressed()));

    KisAction* addVectorLayerAction = new KisAction(koIcon("list-add"), tr("Add Animation Vector Layer"), this);
    actionManager->addAction("add_animation_vector_layer", addVectorLayerAction, actionCollection);
    connect(addVectorLayerAction, SIGNAL(triggered()), this, SLOT(vectorLayerPressed()));

    QMenu* layerMenu = new QMenu("Add Animation Layer", this);
    layerMenu->addAction(addPaintLayerAction);
    layerMenu->addAction(addVectorLayerAction);
    addLayerButton->setMenu(layerMenu);
    addLayerButton->setPopupMode(QToolButton::InstantPopup);

    KisAction* removeLayerAction = new KisAction(koIcon("list-remove"), tr("Remove Animation Layer"), this);
    actionManager->addAction("remove_animation_layer", removeLayerAction, actionCollection);

    layerButtons->addWidget(addLayerButton);
    layerButtons->addAction(removeLayerAction);

    QHBoxLayout* leftToolBarLayout = new QHBoxLayout();
    leftToolBarLayout->setAlignment(Qt::AlignLeft);
    leftToolBarLayout->setMargin(0);
    leftToolBarLayout->addWidget(layerButtons);
    leftToolBar->setLayout(leftToolBarLayout);

    QScrollArea* leftScrollArea = new QScrollArea(this);
    leftScrollArea->setBackgroundRole(QPalette::Dark);
    leftScrollArea->setWidget(m_list);
    m_list->setFixedHeight(45);
    m_list->setFixedWidth(2000);
    leftScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    leftScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QGridLayout* leftLayout = new QGridLayout();
    leftLayout->addWidget(leftToolBar, 1, 0);
    leftLayout->addWidget(leftScrollArea, 0, 0);
    leftLayout->setMargin(0);
    leftLayout->setSpacing(0);
    leftWidget->setLayout(leftLayout);

    QToolBar* frameButtons = new QToolBar(this);

    KisAction* addFrameAction = new KisAction(koIcon("list-add"), tr("Insert Frame"), this);
    actionManager->addAction("insert_frame", addFrameAction, actionCollection);
    connect(addFrameAction, SIGNAL(triggered()), this, SLOT(addframePressed()));

    KisAction* addKeyFrameAction = new KisAction(koIcon("list-add"), tr("Insert Keyframe"), this);
    actionManager->addAction("insert_key_frame", addKeyFrameAction, actionCollection);
    connect(addKeyFrameAction, SIGNAL(triggered()), this, SLOT(keyFramePressed()));

    KisAction* addBlankFrameAction = new KisAction(koIcon("list-add"), tr("Insert Blank Frame"), this);
    actionManager->addAction("insert_blank_frame", addBlankFrameAction, actionCollection);
    connect(addBlankFrameAction, SIGNAL(triggered()), this, SLOT(blankFramePressed()));

    KisAction* removeFrameAction = new KisAction(koIcon("list-remove"), tr("Remove Frame"), this);
    actionManager->addAction("remove_frame", removeFrameAction, actionCollection);

    frameButtons->addAction(addFrameAction);
    frameButtons->addAction(addKeyFrameAction);
    frameButtons->addAction(addBlankFrameAction);
    frameButtons->addAction(removeFrameAction);

    QToolBar* navToolBar = new QToolBar(this);

    KisAction* nextFrameAction = new KisAction(koIcon("go-next-view"), tr("Next Frame"), this);
    actionManager->addAction("next_frame", nextFrameAction, actionCollection);
    connect(nextFrameAction, SIGNAL(triggered()), this, SLOT(nextFramePressed()));

    KisAction* prevFrameAction = new KisAction(koIcon("go-previous-view"), tr("Previous Frame"), this);
    actionManager->addAction("previous_frame", prevFrameAction, actionCollection);
    connect(prevFrameAction, SIGNAL(triggered()), this, SLOT(prevFramePressed()));

    KisAction* prevKeyFrameAction = new KisAction(koIcon("go-previous-content"), tr("Previous Keyframe"), this);
    actionManager->addAction("previous_key_frame", prevKeyFrameAction, actionCollection);
    connect(prevKeyFrameAction, SIGNAL(triggered()), this, SLOT(prevKeyFramePressed()));

    KisAction* nextKeyFrameAction = new KisAction(koIcon("go-next-content"), tr("Next Keyframe"), this);
    actionManager->addAction("next_key_frame", nextKeyFrameAction, actionCollection);
    connect(nextKeyFrameAction, SIGNAL(triggered()), this, SLOT(nextKeyFramePressed()));

    navToolBar->addAction(prevKeyFrameAction);
    navToolBar->addAction(prevFrameAction);
    navToolBar->addAction(nextFrameAction);
    navToolBar->addAction(nextKeyFrameAction);

    QToolBar* playerButtons = new QToolBar(this);

    KisAction* playAction = new KisAction((KIcon)style()->standardIcon(QStyle::SP_MediaPlay), tr("Play Animation"), this);
    actionManager->addAction("play_animation", playAction, actionCollection);
    connect(playAction, SIGNAL(triggered()), this, SLOT(playAnimation()));

    KisAction* pauseAction = new KisAction((KIcon)style()->standardIcon(QStyle::SP_MediaPause), tr("Pause Animation"), this);
    actionManager->addAction("pause_animation", pauseAction, actionCollection);
    connect(pauseAction, SIGNAL(triggered()), this, SLOT(pauseAnimation()));

    KisAction* stopAction = new KisAction((KIcon)style()->standardIcon(QStyle::SP_MediaStop), tr("Stop Animation"), this);
    actionManager->addAction("stop_animation", stopAction, actionCollection);
    connect(stopAction, SIGNAL(triggered()), this, SLOT(stopAnimation()));

    playerButtons->addAction(playAction);
    playerButtons->addAction(pauseAction);
    playerButtons->addAction(stopAction);

    QToolBar* settingsToolBar = new QToolBar(this);

    QCheckBox* frameBreakState = new QCheckBox(this);
    frameBreakState->setText(tr("Add blank frame "));

    QToolButton* settingsButton = new QToolButton(this);
    settingsButton->setIcon(koIcon("configure"));
    settingsButton->setToolTip(tr("Settings"));

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

    QScrollArea* rightScrollArea = new QScrollArea(this);
    rightScrollArea->setBackgroundRole(QPalette::Dark);
    rightScrollArea->setWidget(m_cells);
    m_cells->setFixedWidth(4000);
    m_cells->setFixedHeight(45);
    rightScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    connect(rightScrollArea->verticalScrollBar(), SIGNAL(sliderMoved(int)), leftScrollArea->verticalScrollBar(), SLOT(setValue(int)));

    QGridLayout* rightLayout = new QGridLayout();
    rightLayout->addWidget(rightToolBar, 1, 0);
    rightLayout->addWidget(rightScrollArea, 0, 0);
    rightLayout->setMargin(0);
    rightLayout->setSpacing(0);
    rightWidget->setLayout(rightLayout);

    QSplitter* splitter = new QSplitter(m_parent);
    splitter->addWidget(leftWidget);
    splitter->addWidget(rightWidget);
    splitter->setSizes(QList<int>() << 100 << 600);

    QGridLayout* lay = new QGridLayout();

    lay->addWidget(splitter, 0, 0);
    lay->setMargin(0);
    lay->setSpacing(0);
    this->setLayout(lay);

    connect(this->m_cells, SIGNAL(frameSelectionChanged(QRect)), this, SLOT(frameSelectionChanged(QRect)));
    connect(this->m_settingsDialog, SIGNAL(sigTimelineWithChanged(int)), this, SLOT(timelineWidthChanged(int)));

    m_initialized = true;
}

void KisTimeline::frameSelectionChanged(QRect frame)
{
    dynamic_cast<KisAnimationDoc*>(this->m_canvas->view()->document())->frameSelectionChanged(frame);
}

void KisTimeline::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
}

void KisTimeline::setCanvas(KisCanvas2 *canvas)
{
    m_canvas = canvas;

    if(!m_initialized) {
        this->init();
    }

    // Connect all the document signals here
    connect(dynamic_cast<KisAnimationDoc*>(m_canvas->view()->document()), SIGNAL(sigFrameModified()), this, SLOT(documentModified()));
    connect(dynamic_cast<KisAnimationDoc*>(m_canvas->view()->document()), SIGNAL(sigImportFinished(QList<QRect>)), this, SLOT(importUI(QList<QRect>)));
}

void KisTimeline::setModel(KisAnimation *animation)
{
    this->m_animation = animation;
    this->m_settingsDialog->setModel(animation);
}

KisCanvas2* KisTimeline::getCanvas()
{
    return m_canvas;
}

KisAnimationLayerBox* KisTimeline::getLayerBox()
{
    return m_list;
}

KisFrameBox* KisTimeline::getFrameBox()
{
    return m_cells;
}

void KisTimeline::addLayerUiUpdate()
{
    m_list->addLayerUiUpdate();
    m_cells->addLayerUiUpdate();
}

void KisTimeline::paintLayerPressed()
{
    this->addLayerUiUpdate();
    dynamic_cast<KisAnimationDoc*>(this->getCanvas()->view()->document())->addPaintLayer();
}

void KisTimeline::vectorLayerPressed()
{
    this->addLayerUiUpdate();
    dynamic_cast<KisAnimationDoc*>(this->getCanvas()->view()->document())->addVectorLayer();
}

void KisTimeline::blankFramePressed()
{
    if(m_cells->getSelectedFrame()) {
        QRect globalGeometry = this->m_cells->getSelectedFrame()->convertSelectionToFrame(KisAnimationFrame::BLANKFRAME);
        dynamic_cast<KisAnimationDoc*>(this->m_canvas->view()->document())->addBlankFrame(globalGeometry);
    }
}

void KisTimeline::keyFramePressed()
{
    if(m_cells->getSelectedFrame()) {
        QRect globalGeometry = this->m_cells->getSelectedFrame()->convertSelectionToFrame(KisAnimationFrame::KEYFRAME);
        dynamic_cast<KisAnimationDoc*>(this->m_canvas->view()->document())->addKeyFrame(globalGeometry);
    }
}

void KisTimeline::addframePressed()
{
    if(m_cells->getSelectedFrame()) {
        this->m_cells->getSelectedFrame()->expandWidth();
        this->m_cells->setSelectedFrame();
    }
}

void KisTimeline::frameBreakStateChanged(bool state)
{
    this->m_frameBreakState = state;
}

void KisTimeline::breakFrame(QRect position)
{
    if(m_lastBrokenFrame.x() == position.x() && m_lastBrokenFrame.y() == position.y()) {
        return;
    }

    QRect globalGeometry = this->m_cells->getSelectedFrame()->convertSelectionToFrame(KisAnimationFrame::KEYFRAME);

    this->m_lastBrokenFrame = position;
    dynamic_cast<KisAnimationDoc*>(this->getCanvas()->view()->document())->breakFrame(globalGeometry, this->m_frameBreakState);
}

void KisTimeline::nextFramePressed()
{
    this->m_cells->setSelectedFrame(m_cells->getSelectedFrame()->geometry().x() + 10);
}

void KisTimeline::prevFramePressed()
{
    this->m_cells->setSelectedFrame(m_cells->getSelectedFrame()->geometry().x() - 10);
}

void KisTimeline::nextKeyFramePressed()
{
    KisAnimationFrame* currSelection = this->m_cells->getSelectedFrame();

    QRect nextKeyFrame = dynamic_cast<KisAnimationDoc*>(m_canvas->view()->document())->getNextKeyFramePosition(currSelection->x(),
                                                                                                               currSelection->getParent()->getLayerIndex() * 20);
    this->m_cells->setSelectedFrame(nextKeyFrame.x());
}

void KisTimeline::prevKeyFramePressed()
{
    KisAnimationFrame* currSelection = this->m_cells->getSelectedFrame();

    QRect prevKeyFrame = dynamic_cast<KisAnimationDoc*>(m_canvas->view()->document())->getPreviousKeyFramePosition(currSelection->x(),
                                                                                                                   currSelection->getParent()->getLayerIndex() * 20);

    this->m_cells->setSelectedFrame(prevKeyFrame.x());
}

void KisTimeline::settingsButtonPressed()
{
    m_settingsDialog->setVisible(true);
}

void KisTimeline::playAnimation()
{
    dynamic_cast<KisAnimationDoc*>(this->m_canvas->view()->document())->play();
}

void KisTimeline::pauseAnimation()
{
    dynamic_cast<KisAnimationDoc*>(this->m_canvas->view()->document())->pause();
}

void KisTimeline::stopAnimation()
{
    dynamic_cast<KisAnimationDoc*>(this->m_canvas->view()->document())->stop();
}

void KisTimeline::timelineWidthChanged(int width)
{
    m_cells->setFixedWidth(width * 10);
}

void KisTimeline::documentModified()
{
    emit canvasModified();
    KisAnimationFrame* selectedFrame = this->m_cells->getSelectedFrame();
    if(selectedFrame) {
        if(m_animation->frameBreakingEnabled()) {
            this->breakFrame(QRect(selectedFrame->x(), selectedFrame->y(), 10, 20));
        }
    }
}

void KisTimeline::importUI(QList<QRect> timelineMap)
{
    kWarning() << "Importing...";
    kWarning() << timelineMap.size();
}
