/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
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
 */

#include "kis_color_selector_base.h"

#include <QMouseEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QTimer>
#include <QCursor>
#include <QPainter>

#include <KConfig>
#include <KConfigGroup>
#include <KComponentData>
#include <KGlobal>

#include "KoColorSpace.h"
#include "KoColorSpaceRegistry.h"

#include "kis_canvas2.h"
#include "kis_canvas_resource_provider.h"
#include "kis_node.h"
#include "kis_view2.h"
#include "kis_image.h"

class KisColorPreviewPopup : public QWidget {
public:
    KisColorPreviewPopup(KisColorSelectorBase* parent) : QWidget(), m_parent(parent)
    {
        setWindowFlags(Qt::Popup);
        setColor(QColor(0,0,0));
        setMouseTracking(true);
    }

    void show()
    {
        updatePosition();
        QWidget::show();
    }

    void updatePosition()
    {
        QPoint parentPos = m_parent->mapToGlobal(QPoint(0,0));
        QRect availRect = QApplication::desktop()->availableGeometry(this);
        QPoint targetPos;
        if ( parentPos.x() - 100 > availRect.x() ) {
            targetPos =  QPoint(parentPos.x() - 100, parentPos.y());
        } else if ( parentPos.x() + m_parent->width() + 100 < availRect.right()) {
            targetPos = m_parent->mapToGlobal(QPoint(m_parent->width(), 0));
        } else if ( parentPos.y() - 100 > availRect.y() ) {
            targetPos =  QPoint(parentPos.x(), parentPos.y() - 100);
        } else {
            targetPos =  QPoint(parentPos.x(), parentPos.y() + m_parent->height());
        }
        setGeometry(targetPos.x(), targetPos.y(), 100, 100);
    }

    void setColor(const QColor& color)
    {
        m_color = color;
        update();
    }

protected:
    void paintEvent(QPaintEvent *e) {
        Q_UNUSED(e);
        QPainter p(this);
        p.fillRect(0,0, width(), width(), m_color);
    }

    // these are hacks, as it seems, that at a time only one widget can be a Qt::Popup and therefore grab the mouse globally
    void mouseReleaseEvent(QMouseEvent *e) {
        QMouseEvent* newEvent = new QMouseEvent(e->type(),
                                             m_parent->mapFromGlobal(e->globalPos()),
                                             e->globalPos(),
                                             e->button(),
                                             e->buttons(),
                                             e->modifiers());
        m_parent->mouseReleaseEvent(newEvent);
        delete newEvent;
    }

    void mousePressEvent(QMouseEvent *e) {
        QMouseEvent* newEvent = new QMouseEvent(e->type(),
                                             m_parent->mapFromGlobal(e->globalPos()),
                                             e->globalPos(),
                                             e->button(),
                                             e->buttons(),
                                             e->modifiers());
        m_parent->mousePressEvent(newEvent);
        delete newEvent;
    }

    void mouseMoveEvent(QMouseEvent *e) {
        QMouseEvent* newEvent = new QMouseEvent(e->type(),
                                             m_parent->mapFromGlobal(e->globalPos()),
                                             e->globalPos(),
                                             e->button(),
                                             e->buttons(),
                                             e->modifiers());
        m_parent->mouseMoveEvent(newEvent);
        delete newEvent;
    }

private:
    KisColorSelectorBase* m_parent;
    QColor m_color;
};

KisColorSelectorBase::KisColorSelectorBase(QWidget *parent) :
    QWidget(parent),
    m_canvas(0),
    m_popup(0),
    m_parent(0),
    m_colorUpdateAllowed(true),
    m_hideDistance(0),
    m_hideTimer(new QTimer(this)),
    m_popupOnMouseOver(false),
    m_popupOnMouseClick(true),
    m_colorSpace(0),
    m_isPopup(false),
    m_colorPreviewPopup(new KisColorPreviewPopup(this))
{
    m_hideTimer->setInterval(0);
    m_hideTimer->setSingleShot(true);
    connect(m_hideTimer, SIGNAL(timeout()), this, SLOT(hidePopup()));
}

KisColorSelectorBase::~KisColorSelectorBase()
{
    delete m_colorPreviewPopup;
}

void KisColorSelectorBase::setPopupBehaviour(bool onMouseOver, bool onMouseClick)
{
    m_popupOnMouseClick = onMouseClick;
    m_popupOnMouseOver = onMouseOver;
    if(onMouseClick) {
        m_popupOnMouseOver = false;
    }

    if(m_popupOnMouseOver) {
        setMouseTracking(true);
    }
}

void KisColorSelectorBase::setColorSpace(const KoColorSpace *colorSpace)
{
    m_colorSpace = colorSpace;
}

void KisColorSelectorBase::setCanvas(KisCanvas2 *canvas)
{
    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
    }
    m_canvas = canvas;
    if (m_canvas) {
        connect(m_canvas->resourceManager(), SIGNAL(resourceChanged(int, const QVariant&)),
            this, SLOT(resourceChanged(int, const QVariant&)), Qt::UniqueConnection);
    }

    update();
}

void KisColorSelectorBase::mousePressEvent(QMouseEvent* event)
{
    if(!rect().contains(event->pos())) {
        event->accept();
    }
    else if(m_popupOnMouseClick && (event->buttons()&Qt::MidButton)>0 && !m_isPopup) {
        //open popup
        showPopup(MoveToMousePosition);

        int x = event->globalX();
        int y = event->globalY();
        int popupsize = m_popup->width();
        x-=popupsize/2;
        y-=popupsize/2;

        QRect availRect = QApplication::desktop()->availableGeometry(this);
        if(x<availRect.x())
            x = availRect.x();
        if(y<availRect.y())
            y = availRect.y();
        if(x+m_popup->width()>availRect.x()+availRect.width())
            x = availRect.x()+availRect.width()-m_popup->width();
        if(y+m_popup->height()>availRect.y()+availRect.height())
            y = availRect.y()+availRect.height()-m_popup->height();


        m_popup->move(x, y);
        m_popup->show();
        m_popup->m_colorPreviewPopup->updatePosition();

        event->accept();
    }
    else if(m_isPopup && ((event->button()==Qt::MidButton) || (!rect().contains(event->pos())))) {
        event->accept();
        hide();
    }
    else {
        if(m_colorPreviewPopup->isHidden())
            m_colorPreviewPopup->show();
        event->ignore();
    }
}

void KisColorSelectorBase::mouseReleaseEvent(QMouseEvent *e) {
    Q_UNUSED(e);
    hidePopup();
}

void KisColorSelectorBase::mouseMoveEvent(QMouseEvent* e)
{
//    kDebug()<<"mouse move event, e="<<e->pos()<<"  global="<<e->globalPos();

    if(!(e->buttons()&Qt::LeftButton || e->buttons()&Qt::RightButton)
       && (qMin(e->x(), e->y())<-m_hideDistance || e->x() > width()+m_hideDistance || e->y()>height()+m_hideDistance)) {

        // don't hide preview, if this isn't a popup, otherwise the popup wouldn't get any global mouse
        // events and therefore couldn't hide. in case of popup it will be hidden together with the popup
        if(!m_isPopup) m_colorPreviewPopup->hide();

        if(m_isPopup && !m_parent->rect().contains(m_parent->mapFromGlobal(e->globalPos()))) {
            if(!m_hideTimer->isActive()) {
                m_hideTimer->start();
            }
            e->accept();
            return;
        }
    }
    else if (m_isPopup){
        m_hideTimer->stop();
        e->accept();
        return;
    }
    else if(!m_isPopup && m_popupOnMouseOver && this->rect().contains(e->pos()) && (m_popup==0 || m_popup->isHidden())) {
        //open popup
        privateCreatePopup();

        QRect availRect = QApplication::desktop()->availableGeometry(this);
        QRect forbiddenRect = QRect(parentWidget()->mapToGlobal(QPoint(0,0)),
                                    QSize(parentWidget()->width(), parentWidget()->height()));
//        QRect forbiddenRect = rect();
//        forbiddenRect.moveTo(mapToGlobal(QPoint(0,0)));

        int x,y;
        if(forbiddenRect.y()+forbiddenRect.height()/2 > availRect.height()/2) {
            //popup above forbiddenRect
            y = forbiddenRect.y()-m_popup->height();
        }
        else {
            //popup below forbiddenRect
            y = forbiddenRect.y()+forbiddenRect.height();
        }

        if(forbiddenRect.x()+forbiddenRect.width()/2 < availRect.width()/2) {
            //left edge of popup justified with left edge of popup
            x = forbiddenRect.x();
//            kDebug()<<"1 forbiddenRect.x="<<forbiddenRect.x();
        }
        else {
            //the other way round
            x = forbiddenRect.x()+forbiddenRect.width()-m_popup->width();
//            kDebug()<<"2 forbiddenRect.x="<<m_popup->width();
        }

        m_popup->move(x, y);
        m_popup->setHidingDistanceAndTime(0, 250);
        showPopup(DontMove);
        e->accept();
        return;
    }
}

void KisColorSelectorBase::keyPressEvent(QKeyEvent *)
{
    hidePopup();
}

qreal distance(const QColor& c1, const QColor& c2)
{
    qreal dr = c1.redF()-c2.redF();
    qreal dg = c1.greenF()-c2.greenF();
    qreal db = c1.blueF()-c2.blueF();

    return sqrt(dr*dr+dg*dg+db*db);
}

inline bool inRange(qreal m) {
    if(m>=0. && m<=1.) return true;
    else return false;
}

inline bool modify(QColor* estimate, const QColor& target, const QColor& result)
{
    qreal r = estimate->redF() - (result.redF() - target.redF());
    qreal g = estimate->greenF() - (result.greenF() - target.greenF());
    qreal b = estimate->blueF() - (result.blueF() - target.blueF());

    if(inRange(r) && inRange(g) && inRange(b)) {
        estimate->setRgbF(r, g, b);
        return true;
    }
    else {
        return false;
    }
}

QColor KisColorSelectorBase::findGeneratingColor(const KoColor& ref) const
{
//    kDebug() << "starting search for generating colour";
    KoColor converter(colorSpace());
    QColor currentEstimate;
    ref.toQColor(&currentEstimate);
//    kDebug() << "currentEstimate: " << currentEstimate;

    QColor currentResult;
    converter.fromQColor(currentEstimate);
    converter.toQColor(&currentResult);
//    kDebug() << "currentResult: " << currentResult;


    QColor target;
    ref.toQColor(&target);
//    kDebug() << "target: " << target;

    bool estimateValid=true;
    int iterationCounter=0;

//    kDebug() << "current distance = " << distance(target, currentResult);
    while(distance(target, currentResult)>0.001 && estimateValid && iterationCounter<100) {
        estimateValid = modify(&currentEstimate, target, currentResult);
        converter.fromQColor(currentEstimate);
        converter.toQColor(&currentResult);
//        kDebug() << "current distance = " << distance(target, currentResult);

        iterationCounter++;
    }

//    kDebug() << "end search for generating colour";

    return currentEstimate;
}

void KisColorSelectorBase::dragEnterEvent(QDragEnterEvent *e)
{
    if(e->mimeData()->hasColor())
        e->acceptProposedAction();
    if(e->mimeData()->hasText() && QColor(e->mimeData()->text()).isValid())
        e->acceptProposedAction();
}

void KisColorSelectorBase::dropEvent(QDropEvent *e)
{
    QColor color;
    if(e->mimeData()->hasColor()) {
        color = qvariant_cast<QColor>(e->mimeData()->colorData());
    }
    else if(e->mimeData()->hasText()) {
        color.setNamedColor(e->mimeData()->text());
        if(!color.isValid())
            return;
    }

    KoColor kocolor(color , KoColorSpaceRegistry::instance()->rgb8());
    color = findGeneratingColor(kocolor);
    setColor(color);
    commitColor(kocolor, Foreground);
}

void KisColorSelectorBase::setColor(const QColor& color)
{
    Q_UNUSED(color);
}

void KisColorSelectorBase::setHidingDistanceAndTime(int distance, int time)
{
    m_hideDistance = distance;
    m_hideTimer->setInterval(time);
}


void KisColorSelectorBase::privateCreatePopup()
{
    m_popup = createPopup();
    Q_ASSERT(m_popup);
    m_popup->setWindowFlags(Qt::FramelessWindowHint|Qt::SubWindow|Qt::X11BypassWindowManagerHint);
    m_popup->m_parent = this;
    m_popup->m_isPopup=true;
    m_popup->setCanvas(m_canvas);
    m_popup->updateSettings();
}

void KisColorSelectorBase::showPopup(Move move)
{
    if(m_popup==0) {
        privateCreatePopup();
    }

    QPoint cursorPos = QCursor::pos();

    if(move == MoveToMousePosition)
        m_popup->move(cursorPos.x()-m_popup->width()/2, cursorPos.y()-m_popup->height()/2);

    m_popup->show();
    m_popup->m_colorPreviewPopup->show();
}

void KisColorSelectorBase::hidePopup()
{
    if(m_isPopup) {
        m_colorPreviewPopup->hide();
        hide();
    }
    else if (m_popup!=0) {
        m_popup->m_colorPreviewPopup->hide();
        m_popup->hide();
    }
}

void KisColorSelectorBase::commitColor(const KoColor& color, ColorRole role)
{
    if (!m_canvas)
        return;

    m_colorUpdateAllowed=false;

    if (role==Foreground)
        m_canvas->resourceManager()->setForegroundColor(color);
    else
        m_canvas->resourceManager()->setBackgroundColor(color);

    m_colorUpdateAllowed=true;
}

void KisColorSelectorBase::updateColorPreview(const QColor& color)
{
//    kDebug() << "updating color preview " << color.red() << "," << color.green() << "," << color.blue();
    m_colorPreviewPopup->setColor(color);
}

void KisColorSelectorBase::resourceChanged(int key, const QVariant &v)
{
    if (key == KoCanvasResourceManager::ForegroundColor || key == KoCanvasResourceManager::BackgroundColor) {
        QColor c = findGeneratingColor(v.value<KoColor>());
        updateColorPreview(c);
        if(m_colorUpdateAllowed==false)
            return;
        setColor(c);
    }
}

const KoColorSpace* KisColorSelectorBase::colorSpace() const
{
    if (m_colorSpace != 0) {
        return m_colorSpace;
    }
    else if (m_canvas && m_canvas->resourceManager()) {
        KisNodeSP currentNode = m_canvas->resourceManager()->
                                resource(KisCanvasResourceProvider::CurrentKritaNode).value<KisNodeSP>();
        if (currentNode) {
            m_colorSpace=currentNode->colorSpace();
        } else {
            m_colorSpace=m_canvas->view()->image()->colorSpace();
        }
        return m_colorSpace;
    }
    return KoColorSpaceRegistry::instance()->rgb8(0);
}

void KisColorSelectorBase::updateSettings()
{
    if(m_popup!=0)
        m_popup->updateSettings();

    KConfigGroup cfg = KGlobal::config()->group("advancedColorSelector");

    bool onMouseOver = cfg.readEntry("popupOnMouseOver", false);
    bool onMouseClick = cfg.readEntry("popupOnMouseClick", true);
    setPopupBehaviour(onMouseOver, onMouseClick);

    if(cfg.readEntry("useCustomColorSpace", true)) {
        KoColorSpaceRegistry* csr = KoColorSpaceRegistry::instance();
        m_colorSpace = csr->colorSpace(cfg.readEntry("customColorSpaceModel", "RGBA"),
                                                         cfg.readEntry("customColorSpaceDepthID", "U8"),
                                                         cfg.readEntry("customColorSpaceProfile", "sRGB built-in - (lcms internal)"));
    }
    else {
        m_colorSpace=0;
        // the colorspace will be retrieved next time by calling colorSpace()
    }
    if(m_isPopup) {
        resize(cfg.readEntry("zoomSize", 280), cfg.readEntry("zoomSize", 280));
    }
}

