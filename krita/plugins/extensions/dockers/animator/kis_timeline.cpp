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

KisTimeline::KisTimeline(QWidget *parent) : QWidget(parent)
{
    m_list = new KisAnimationLayerBox(this);
    m_cells = new KisFrameBox(this);
    m_settingsDialog = new AnimatorSettingsDialog();

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

    QToolButton* m_addLayerButton = new QToolButton(this);
    m_addLayerButton->setIcon(koIcon("list-add"));
    m_addLayerButton->setFixedSize(15, 15);
    m_addLayerButton->setToolTip("Add Layer");

    m_addPaintLayerAction = new QAction(koIcon("list-add"), "Add Paint Layer", this);
    m_addVectorLayerAction = new QAction(koIcon("list-add"), "Add Vector Layer", this);
    QMenu* layerMenu = new QMenu("Add Layer", this);
    layerMenu->addAction(m_addPaintLayerAction);
    layerMenu->addAction(m_addVectorLayerAction);
    m_addLayerButton->setMenu(layerMenu);
    m_addLayerButton->setPopupMode(QToolButton::InstantPopup);

    QToolButton* removeLayerButton = new QToolButton(this);
    removeLayerButton->setIcon(koIcon("list-remove"));
    removeLayerButton->setToolTip("Remove Layer");
    removeLayerButton->setFixedSize(15, 15);

    layerButtons->addWidget(m_addLayerButton);
    layerButtons->addWidget(removeLayerButton);

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
    connect(m_addPaintLayerAction, SIGNAL(triggered()), this, SLOT(updateHeight()));
    connect(m_addVectorLayerAction, SIGNAL(triggered()), this, SLOT(updateHeight()));

    QGridLayout* leftLayout = new QGridLayout();
    leftLayout->addWidget(leftToolBar, 1, 0);
    leftLayout->addWidget(leftScrollArea, 0, 0);
    leftLayout->setMargin(0);
    leftLayout->setSpacing(0);
    leftWidget->setLayout(leftLayout);

    QToolBar* frameButtons = new QToolBar(this);

    QToolButton* addFrameButton = new QToolButton(this);
    addFrameButton->setIcon(koIcon("list-add"));
    addFrameButton->setToolTip("Insert Frame");
    addFrameButton->setFixedSize(15, 15);

    QToolButton* addKeyFrameButton = new QToolButton(this);
    addKeyFrameButton->setIcon(koIcon("list-add"));
    addKeyFrameButton->setToolTip("Insert key frame");
    addKeyFrameButton->setFixedSize(15, 15);

    QToolButton* addBlankFrameButton = new QToolButton(this);
    addBlankFrameButton->setIcon(koIcon("list-add"));
    addBlankFrameButton->setToolTip("Insert blank frame");
    addBlankFrameButton->setFixedSize(15, 15);

    QToolButton* removeFrameButton = new QToolButton(this);
    removeFrameButton->setIcon(koIcon("list-remove"));
    removeFrameButton->setToolTip("Remove frame");
    removeFrameButton->setFixedSize(15, 15);

    frameButtons->addWidget(addFrameButton);
    frameButtons->addWidget(addKeyFrameButton);
    frameButtons->addWidget(addBlankFrameButton);
    frameButtons->addWidget(removeFrameButton);

    connect(addFrameButton, SIGNAL(pressed()), this, SLOT(addframePressed()));
    connect(addKeyFrameButton, SIGNAL(pressed()), this, SLOT(keyFramePressed()));
    connect(addBlankFrameButton, SIGNAL(pressed()), this, SLOT(blankFramePressed()));

    QToolBar* navToolBar = new QToolBar(this);

    QToolButton* nextFrameButton = new QToolButton(this);
    nextFrameButton->setIcon(koIcon("go-next-view"));
    nextFrameButton->setToolTip("Next frame");
    nextFrameButton->setFixedSize(15, 15);

    QToolButton* prevFrameButton = new QToolButton(this);
    prevFrameButton->setIcon(koIcon("go-previous-view"));
    prevFrameButton->setToolTip("Previous frame");
    prevFrameButton->setFixedSize(15, 15);

    QToolButton* prevKeyFrameButton = new QToolButton(this);
    prevKeyFrameButton->setIcon(koIcon("go-previous-content"));
    prevKeyFrameButton->setToolTip("Previous keyframe");
    prevKeyFrameButton->setFixedSize(15, 15);

    QToolButton* nextKeyFrameButton = new QToolButton(this);
    nextKeyFrameButton->setIcon(koIcon("go-next-content"));
    nextKeyFrameButton->setToolTip("Next keyframe");
    nextKeyFrameButton->setFixedSize(15, 15);

    navToolBar->addWidget(prevKeyFrameButton);
    navToolBar->addWidget(prevFrameButton);
    navToolBar->addWidget(nextFrameButton);
    navToolBar->addWidget(nextKeyFrameButton);

    connect(prevFrameButton, SIGNAL(clicked()), this, SLOT(prevFramePressed()));
    connect(nextFrameButton, SIGNAL(clicked()), this, SLOT(nextFramePressed()));
    connect(nextKeyFrameButton, SIGNAL(clicked()), this, SLOT(nextKeyFramePressed()));
    connect(prevKeyFrameButton, SIGNAL(clicked()), this, SLOT(prevKeyFramePressed()));

    QToolBar* playerButtons = new QToolBar(this);

    QToolButton* playButton = new QToolButton(this);
    playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    playButton->setToolTip("Play Animation");
    playButton->setFixedSize(15, 15);
    playerButtons->addWidget(playButton);

    QToolButton* pauseButton = new QToolButton(this);
    pauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    pauseButton->setToolTip("Pause Animation");
    pauseButton->setFixedSize(15, 15);
    playerButtons->addWidget(pauseButton);

    QToolButton* stopButton = new QToolButton(this);
    stopButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    stopButton->setToolTip("Stop Animation");
    stopButton->setFixedSize(15, 15);
    playerButtons->addWidget(stopButton);

    connect(playButton, SIGNAL(clicked()), this, SLOT(playAnimation()));
    connect(pauseButton, SIGNAL(clicked()), this, SLOT(pauseAnimation()));
    connect(stopButton, SIGNAL(clicked()), this, SLOT(stopAnimation()));

    QToolBar* settingsToolBar = new QToolBar(this);

    QCheckBox* frameBreakState = new QCheckBox(this);
    frameBreakState->setText("Add blank frame ");

    QToolButton* settingsButton = new QToolButton(this);
    settingsButton->setIcon(koIcon("configure"));
    settingsButton->setToolTip("Settings");
    settingsButton->setFixedSize(15, 15);

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
    m_cells->setFixedWidth(4000);       //Needs to be varied
    m_cells->setFixedHeight(45);
    rightScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    connect(rightScrollArea->verticalScrollBar(), SIGNAL(sliderMoved(int)), leftScrollArea->verticalScrollBar(), SLOT(setValue(int)));

    QGridLayout* rightLayout = new QGridLayout();
    rightLayout->addWidget(rightToolBar, 1, 0);
    rightLayout->addWidget(rightScrollArea, 0, 0);
    rightLayout->setMargin(0);
    rightLayout->setSpacing(0);
    rightWidget->setLayout(rightLayout);

    QSplitter* splitter = new QSplitter(parent);
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
}

void KisTimeline::frameSelectionChanged(QRect frame)
{
    dynamic_cast<KisAnimationDoc*>(this->m_canvas->view()->document())->frameSelectionChanged(frame);
}

void KisTimeline::resizeEvent(QResizeEvent *event)
{

}

void KisTimeline::setCanvas(KisCanvas2 *canvas)
{
    m_canvas = canvas;
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

void KisTimeline::updateHeight()
{
    m_list->setFixedHeight(m_list->height()+20);
    m_cells->setFixedHeight(m_cells->height()+20);
}

void KisTimeline::blankFramePressed()
{
    if(m_cells->getSelectedFrame()) {

        KisAnimationFrame* oldSelection = this->m_cells->getSelectedFrame();
        QRect globalGeometry = this->m_cells->getSelectedFrame()->convertSelectionToFrame(KisAnimationFrame::BLANKFRAME);

        KisAnimationFrame* newSelection = new KisAnimationFrame(oldSelection->getParent(), KisAnimationFrame::SELECTION, 10);
        newSelection->setGeometry(oldSelection->geometry());

        this->m_cells->setSelectedFrame(newSelection);
        newSelection->show();

        dynamic_cast<KisAnimationDoc*>(this->m_canvas->view()->document())->addBlankFrame(globalGeometry);
    }
}

void KisTimeline::keyFramePressed()
{
    if(m_cells->getSelectedFrame()) {

        KisAnimationFrame* oldSelection = this->m_cells->getSelectedFrame();
        QRect globalGeometry = this->m_cells->getSelectedFrame()->convertSelectionToFrame(KisAnimationFrame::KEYFRAME);

        KisAnimationFrame* newSelection = new KisAnimationFrame(oldSelection->getParent(), KisAnimationFrame::SELECTION, 10);
        newSelection->setGeometry(oldSelection->geometry());

        this->m_cells->setSelectedFrame(newSelection);
        newSelection->show();

        dynamic_cast<KisAnimationDoc*>(this->m_canvas->view()->document())->addKeyFrame(globalGeometry);
    }
}

void KisTimeline::addframePressed()
{
    if(m_cells->getSelectedFrame()) {

        KisAnimationFrame* oldSelection = this->m_cells->getSelectedFrame();
        this->m_cells->getSelectedFrame()->expandWidth();

        KisAnimationFrame* newSelection = new KisAnimationFrame(oldSelection->getParent(), KisAnimationFrame::SELECTION, 10);
        newSelection->setGeometry(oldSelection->geometry());

        this->m_cells->setSelectedFrame(newSelection);
        newSelection->show();
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

    kWarning() << "Break frame at frame" << position.x() << " layer " << position.y();

    KisAnimationFrame* oldSelection = this->m_cells->getSelectedFrame();
    QRect globalGeometry = this->m_cells->getSelectedFrame()->convertSelectionToFrame(KisAnimationFrame::KEYFRAME);

    KisAnimationFrame* newSelection = new KisAnimationFrame(oldSelection->getParent(), KisAnimationFrame::SELECTION, 10);
    newSelection->setGeometry(oldSelection->geometry());

    this->m_cells->setSelectedFrame(newSelection);
    newSelection->show();

    this->m_lastBrokenFrame = position;
    dynamic_cast<KisAnimationDoc*>(this->getCanvas()->view()->document())->breakFrame(globalGeometry, this->m_frameBreakState);
}

void KisTimeline::nextFramePressed()
{
    kWarning() << "Next frame pressed";

    KisAnimationFrame* currSelection = this->m_cells->getSelectedFrame();
    currSelection->hide();

    KisAnimationFrame* newSelection = new KisAnimationFrame(currSelection->getParent(), KisAnimationFrame::SELECTION, 10);
    newSelection->setGeometry(currSelection->geometry().x() + 10, currSelection->geometry().y(), 10, 20);

    this->m_cells->setSelectedFrame(newSelection);
    newSelection->show();
}

void KisTimeline::prevFramePressed()
{
    kWarning() << "Previous frame pressed";
    KisAnimationFrame* currSelection = this->m_cells->getSelectedFrame();

    if(currSelection->x() == 0) {
        return;
    }

    currSelection->hide();

    KisAnimationFrame* newSelection = new KisAnimationFrame(currSelection->getParent(), KisAnimationFrame::SELECTION, 10);
    newSelection->setGeometry(currSelection->geometry().x() - 10, currSelection->geometry().y(), 10, 20);

    this->m_cells->setSelectedFrame(newSelection);
    newSelection->show();
}

void KisTimeline::nextKeyFramePressed()
{
    kWarning() << "Next keyframe pressed";
    KisAnimationFrame* currSelection = this->m_cells->getSelectedFrame();

    QRect nextKeyFrame = dynamic_cast<KisAnimationDoc*>(m_canvas->view()->document())->getNextKeyFramePosition(currSelection->x(),
                                                                                                               currSelection->getParent()->getLayerIndex() * 20);
    kWarning() << nextKeyFrame.x() << nextKeyFrame.y();

    currSelection->hide();

    KisAnimationFrame* newSelection = new KisAnimationFrame(currSelection->getParent(), KisAnimationFrame::SELECTION, 10);
    newSelection->setGeometry(nextKeyFrame.x(), currSelection->y(), 10, 20);

    this->m_cells->setSelectedFrame(newSelection);
    newSelection->show();
}

void KisTimeline::prevKeyFramePressed()
{
    kWarning() << "Previous keyframe pressed";
    KisAnimationFrame* currSelection = this->m_cells->getSelectedFrame();

    QRect prevKeyFrame = dynamic_cast<KisAnimationDoc*>(m_canvas->view()->document())->getPreviousKeyFramePosition(currSelection->x(),
                                                                                                                   currSelection->getParent()->getLayerIndex() * 20);
    kWarning() << prevKeyFrame.x() << prevKeyFrame.y();

    currSelection->hide();

    KisAnimationFrame* newSelection = new KisAnimationFrame(currSelection->getParent(), KisAnimationFrame::SELECTION, 10);
    newSelection->setGeometry(prevKeyFrame.x(), currSelection->y(), 10, 20);

    this->m_cells->setSelectedFrame(newSelection);
    newSelection->show();
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
