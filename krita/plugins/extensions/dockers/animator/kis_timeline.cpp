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
#include <QSplitter>
#include <KoIcon.h>
#include <QScrollArea>
#include <QMenu>
#include <QScrollBar>
#include "kis_animation_frame.h"
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

    this->m_numberOfLayers = 0;

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
    m_addLayerButton->setFixedSize(10,10);
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
    removeLayerButton->setFixedSize(10, 10);

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
    addFrameButton->setFixedSize(10, 10);

    QToolButton* addKeyFrameButton = new QToolButton(this);
    addKeyFrameButton->setIcon(koIcon("list-add"));
    addKeyFrameButton->setToolTip("Insert key frame");
    addKeyFrameButton->setFixedSize(10, 10);

    QToolButton* addBlankFrameButton = new QToolButton(this);
    addBlankFrameButton->setIcon(koIcon("list-add"));
    addBlankFrameButton->setToolTip("Insert blank frame");
    addBlankFrameButton->setFixedSize(10, 10);

    QToolButton* removeFrameButton = new QToolButton(this);
    removeFrameButton->setIcon(koIcon("list-remove"));
    removeFrameButton->setToolTip("Remove frame");
    removeFrameButton->setFixedSize(10, 10);

    frameButtons->addWidget(addFrameButton);
    frameButtons->addWidget(addKeyFrameButton);
    frameButtons->addWidget(addBlankFrameButton);
    frameButtons->addWidget(removeFrameButton);

    connect(addFrameButton, SIGNAL(pressed()), this, SLOT(addframePressed()));
    connect(addKeyFrameButton, SIGNAL(pressed()), this, SLOT(keyFramePressed()));
    connect(addBlankFrameButton, SIGNAL(pressed()), this, SLOT(blankFramePressed()));

    QHBoxLayout* rightToolBarLayout = new QHBoxLayout();
    rightToolBarLayout->addWidget(frameButtons);
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
        dynamic_cast<KisAnimationDoc*>(this->m_canvas->view()->document())->addBlankFrame(globalGeometry);
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

void KisTimeline::documentModified()
{
    emit canvasModified();
}
