#include "kis_timeline.h"
#include "kis_timeline_cells.h"
#include "kis_animation_layerbox.h"
#include <QToolButton>
#include <QToolBar>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <KoIcon.h>

KisTimeline::KisTimeline(QWidget *parent) : QWidget(parent)
{
    m_list = new KisAnimationLayerBox(this);
    m_cells = new KisTimelineCells(this);

    this->m_numberOfLayers = 0;

    m_hScrollBar = new QScrollBar(Qt::Horizontal);
    m_vScrollBar = new QScrollBar(Qt::Vertical);

    m_vScrollBar->setMinimum(0);
    m_vScrollBar->setMaximum(1);
    m_vScrollBar->setPageStep(1);

    QWidget* leftWidget = new QWidget();
    leftWidget->setMinimumWidth(120);
    QWidget* rightWidget = new QWidget();

    QWidget* leftToolBar = new QWidget();
    leftToolBar->setFixedHeight(31);
    QWidget* rightToolBar = new QWidget();
    rightToolBar->setFixedHeight(31);

    QToolBar* layerButtons = new QToolBar(this);
    m_addLayerButton = new QToolButton(this);
    m_addLayerButton->setIcon(koIcon("list-add"));
    m_addLayerButton->setFixedSize(10,10);
    m_addLayerButton->setToolTip("Add Layer");

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

    QGridLayout* leftLayout = new QGridLayout();
    leftLayout->addWidget(leftToolBar, 1, 0);
    leftLayout->addWidget(m_list, 0, 0);
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

    QHBoxLayout* rightToolBarLayout = new QHBoxLayout();
    rightToolBarLayout->addWidget(frameButtons);
    rightToolBar->setLayout(rightToolBarLayout);

    QGridLayout* rightLayout = new QGridLayout();
    rightLayout->addWidget(rightToolBar, 1, 0);
    rightLayout->addWidget(m_cells, 0, 0);
    rightLayout->setMargin(0);
    rightLayout->setSpacing(0);
    rightWidget->setLayout(rightLayout);

    QSplitter* splitter = new QSplitter(parent);
    splitter->addWidget(leftWidget);
    splitter->addWidget(rightWidget);
    splitter->setSizes(QList<int>() << 100 << 600);

    QGridLayout* lay = new QGridLayout();

    lay->addWidget(splitter, 0, 0);
    lay->addWidget(m_vScrollBar, 0,1);
    lay->addWidget(m_hScrollBar, 1, 0);
    lay->setMargin(0);
    lay->setSpacing(0);
    this->setLayout(lay);
}

void KisTimeline::resizeEvent(QResizeEvent *event){

}

void KisTimeline::setCanvas(KisCanvas2 *canvas){
    m_canvas = canvas;
}

KisCanvas2* KisTimeline::getCanvas(){
    return m_canvas;
}

KisAnimationLayerBox* KisTimeline::getLayerBox(){
    return m_list;
}
