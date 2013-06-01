#include "kis_opacity_selector_view.h"
#include <QMouseEvent>

KisOpacitySelectorView::KisOpacitySelectorView(QWidget *parent) : QGraphicsView(parent)
{
    setFrameShape(QFrame::Box);
    setFrameShadow(QFrame::Sunken);
    setRenderHint(QPainter::Antialiasing, false);

    m_opacitySelectorScene = new QGraphicsScene;
    setScene(m_opacitySelectorScene);
}

KisOpacitySelectorView::~KisOpacitySelectorView(){

}

void KisOpacitySelectorView::init(){
    m_opacitySelectorScene->setSceneRect(0,0,100,70);
    m_opacitySelector = new KisOpacitySelector(0,0,100,70,m_opacitySelectorScene, m_numberOfFrames);
    m_opacitySelectorScene->addItem(m_opacitySelector);
}

QSize KisOpacitySelectorView::sizeHint() const{
    return QSize(300,70);
}

void KisOpacitySelectorView::setNumberOfFrames(int val){
    m_numberOfFrames = val;
    m_opacitySelectorScene->clear();
    this->init();
}

void KisOpacitySelectorView::mousePressEvent(QMouseEvent *event){
    int x = event->x();
    int y = event->y();
    QPointF p = this->mapToScene(x, y);
    float frameWidth = (float)100/(float)m_numberOfFrames;
    m_opacitySelector->setOpacityValue((int)((float)p.x()/(float)frameWidth), 100 - (int)((float)p.y()*(float)100/(float)70));
}
