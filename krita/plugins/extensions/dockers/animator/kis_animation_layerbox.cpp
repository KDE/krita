#include "kis_animation_layerbox.h"
#include <QPainter>
#include <QModelIndex>
#include <kis_node_model.h>
#include <kis_node_manager.h>
#include "kis_timeline.h"
#include <kis_canvas2.h>
#include <kis_view2.h>
#include <kis_action_manager.h>
#include <kis_action.h>
#include "kis_animation_layer.h"
#include <kis_debug.h>

KisAnimationLayerBox::KisAnimationLayerBox(KisTimeline *parent)
{
    this->m_numberofLayers = 0;
    this->m_dock = parent;
    m_nodeModel = new KisNodeModel(this);
    KisAnimationLayer* firstLayer = new KisAnimationLayer(this);
    firstLayer->setGeometry(QRect(0,this->m_numberofLayers*20,width(),20));
    this->m_numberofLayers = 1;
}

void KisAnimationLayerBox::paintEvent(QPaintEvent *event){
    QPainter painter(this);

    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::darkGray);
    painter.drawRect(QRect(0,0,width(), height()));
}

inline void KisAnimationLayerBox::connectActionToButton(QToolButton *button, const QString &id){
    Q_ASSERT(m_dock->getCanvas());

    KisAction *action = m_dock->getCanvas()->view()->actionManager()->actionByName(id);
    connect(button, SIGNAL(clicked()), action, SLOT(trigger()));
    connect(action, SIGNAL(sigEnableSlaves(bool)), button, SLOT(setEnabled(bool)));
}

void KisAnimationLayerBox::makeConnections(){
    this->connectActionToButton(m_dock->m_addLayerButton, "add_new_paint_layer");
    connect(m_dock->m_addLayerButton, SIGNAL(clicked()), this, SLOT(updateUI()));
}

void KisAnimationLayerBox::updateUI(){
    KisAnimationLayer* newLayer = new KisAnimationLayer(this);
    newLayer->setGeometry(QRect(0, this->m_numberofLayers*20, width(), 20));
    newLayer->show();
    this->m_numberofLayers++;
}
