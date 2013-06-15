#include "kis_animation_layer.h"
#include <QPainter>
#include <QLineEdit>
#include <QHBoxLayout>

KisAnimationLayer::KisAnimationLayer(KisAnimationLayerBox *parent)
{
    this->setParent(parent);
    this->m_layerBox = parent;

    m_inputLayerName = new QLineEdit();
    m_inputLayerName->setFixedHeight(20);

    m_lblLayerName = new QLabel(this);
    m_lblLayerName->setText("Layer");
    m_lblLayerName->setFixedHeight(20);

    lay = new QHBoxLayout();
    lay->addWidget(m_lblLayerName);
    this->setLayout(lay);

    this->setFixedSize(m_layerBox->width(), 20);

    connect(m_inputLayerName, SIGNAL(returnPressed()), this, SLOT(onLayerNameEdited()));
}

void KisAnimationLayer::paintEvent(QPaintEvent *event){
    QPainter painter(this);
    painter.setPen(Qt::darkGray);
    painter.setBrush(Qt::lightGray);
    painter.drawRect(QRect(0,0,width(), height()));
}

void KisAnimationLayer::mouseDoubleClickEvent(QMouseEvent *event){
    lay->removeWidget(m_lblLayerName);
    m_lblLayerName->hide();
    m_inputLayerName->setText(m_lblLayerName->text());
    lay->addWidget(m_inputLayerName);
    m_inputLayerName->show();
}

void KisAnimationLayer::onLayerNameEdited(){
    lay->removeWidget(m_inputLayerName);
    m_inputLayerName->hide();
    m_lblLayerName->setText(m_inputLayerName->text());
    lay->addWidget(m_lblLayerName);
    m_lblLayerName->show();
}
