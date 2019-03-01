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
#include <QMimeData>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include "KoColorSpace.h"
#include "KoColorSpaceRegistry.h"

#include "kis_canvas2.h"
#include "kis_canvas_resource_provider.h"
#include "kis_node.h"
#include "KisViewManager.h"
#include <KisView.h>
#include "kis_image.h"
#include "kis_global.h"
#include "kis_display_color_converter.h"

#include <resources/KoGamutMask.h>

class KisColorPreviewPopup : public QWidget {
public:
    KisColorPreviewPopup(KisColorSelectorBase* parent)
        : QWidget(parent), m_parent(parent)
    {
        setWindowFlags(Qt::ToolTip | Qt::NoDropShadowWindowHint);
        setQColor(QColor(0,0,0));
        m_baseColor = QColor(0,0,0,0);
        m_previousColor = QColor(0,0,0,0);
        m_lastUsedColor = QColor(0,0,0,0);
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
        setGeometry(targetPos.x(), targetPos.y(), 100, 150);
        setAttribute(Qt::WA_TranslucentBackground);
    }

    void setQColor(const QColor& color)
    {
        m_color = color;
        update();
    }

    void setPreviousColor()
    {
        m_previousColor = m_baseColor;
    }

    void setBaseColor(const QColor& color)
    {
        m_baseColor = color;
        update();
    }

    void setLastUsedColor(const QColor& color)
    {
        m_lastUsedColor = color;
        update();
    }

protected:
    void paintEvent(QPaintEvent *e) override {
        Q_UNUSED(e);
        QPainter p(this);
        p.fillRect(0, 0, width(), width(), m_color);
        p.fillRect(50, width(), width(), height(), m_previousColor);
        p.fillRect(0, width(), 50, height(), m_lastUsedColor);
    }

    void enterEvent(QEvent *e) override {
        QWidget::enterEvent(e);
        m_parent->tryHideAllPopups();
    }

    void leaveEvent(QEvent *e) override {
        QWidget::leaveEvent(e);
        m_parent->tryHideAllPopups();
    }

private:
    KisColorSelectorBase* m_parent;
    QColor m_color;
    QColor m_baseColor;
    QColor m_previousColor;
    QColor m_lastUsedColor;
};

KisColorSelectorBase::KisColorSelectorBase(QWidget *parent) :
    QWidget(parent),
    m_canvas(0),
    m_popup(0),
    m_parent(0),
    m_colorUpdateAllowed(true),
    m_colorUpdateSelf(false),
    m_hideTimer(new QTimer(this)),
    m_popupOnMouseOver(false),
    m_popupOnMouseClick(true),
    m_colorSpace(0),
    m_isPopup(false),
    m_hideOnMouseClick(false),
    m_colorPreviewPopup(new KisColorPreviewPopup(this))
{
    m_hideTimer->setInterval(0);
    m_hideTimer->setSingleShot(true);
    connect(m_hideTimer, SIGNAL(timeout()), this, SLOT(hidePopup()));

    using namespace std::placeholders; // For _1 placeholder
    auto function = std::bind(&KisColorSelectorBase::slotUpdateColorAndPreview, this, _1);
    m_updateColorCompressor.reset(new ColorCompressorType(20 /* ms */, function));
}

KisColorSelectorBase::~KisColorSelectorBase()
{
    delete m_popup;
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
        connect(m_canvas->resourceManager(), SIGNAL(canvasResourceChanged(int,QVariant)),
                SLOT(canvasResourceChanged(int,QVariant)), Qt::UniqueConnection);

        connect(m_canvas->displayColorConverter(), SIGNAL(displayConfigurationChanged()),
                SLOT(reset()), Qt::UniqueConnection);

        connect(canvas->imageView()->resourceProvider(), SIGNAL(sigFGColorUsed(KoColor)),
                this,                               SLOT(updateLastUsedColorPreview(KoColor)), Qt::UniqueConnection);

        if (m_canvas->viewManager() && m_canvas->viewManager()->canvasResourceProvider()) {
            setColor(Acs::currentColor(m_canvas->viewManager()->canvasResourceProvider(), Acs::Foreground));
        }
    }
    if (m_popup) {
        m_popup->setCanvas(canvas);
    }

    reset();
}

void KisColorSelectorBase::unsetCanvas()
{
    if (m_popup) {
        m_popup->unsetCanvas();
    }
    m_canvas = 0;
}



void KisColorSelectorBase::mousePressEvent(QMouseEvent* event)
{
    event->accept();


//this boolean here is to check if the colour selector is updating the resource, so it won't update itself when the resource is updated//
   if (m_colorUpdateSelf==false)
   {m_colorUpdateSelf=true;}

    if(!m_isPopup && m_popupOnMouseClick &&
       event->button() == Qt::MidButton) {

        lazyCreatePopup();

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
        m_popup->setHidingTime(200);
        showPopup(DontMove);

    } else if (m_isPopup && event->button() == Qt::MidButton) {
        if (m_colorPreviewPopup) {
            m_colorPreviewPopup->hide();
        }
        hide();
    } else {
        showColorPreview();
        event->ignore();
    }
}

void KisColorSelectorBase::mouseReleaseEvent(QMouseEvent *e) {

   Q_UNUSED(e);

    if (e->button() == Qt::MidButton) {
        e->accept();
    } else if (m_isPopup &&
               (m_hideOnMouseClick && !m_popupOnMouseOver) &&
               !m_hideTimer->isActive()) {
        if (m_colorPreviewPopup) {
            m_colorPreviewPopup->hide();
        }
        hide();
    }
}

void KisColorSelectorBase::enterEvent(QEvent *e)
{
    if (m_popup && m_popup->isVisible()) {
        m_popup->m_hideTimer->stop();
    }

    if (m_isPopup && m_hideTimer->isActive()) {
        m_hideTimer->stop();
    }

    // do not show the popup when boxed in
    // the configuration dialog (m_canvas == 0)

    if (m_canvas &&
        !m_isPopup && m_popupOnMouseOver &&
        (!m_popup || m_popup->isHidden())) {

        lazyCreatePopup();

        const QRect availRect = QApplication::desktop()->availableGeometry(this);

        QPoint proposedTopLeft = rect().center() - m_popup->rect().center();
        proposedTopLeft = mapToGlobal(proposedTopLeft);

        QRect popupRect = QRect(proposedTopLeft, m_popup->size());
        popupRect = kisEnsureInRect(popupRect, availRect);

        m_popup->setGeometry(popupRect);
        m_popup->setHidingTime(200);
        showPopup(DontMove);
    }

    QWidget::enterEvent(e);
}

void KisColorSelectorBase::leaveEvent(QEvent *e)
{
    tryHideAllPopups();
    QWidget::leaveEvent(e);
}

void KisColorSelectorBase::keyPressEvent(QKeyEvent *)
{
    if (m_isPopup) {
        hidePopup();
    }
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
    updateColor(kocolor, Acs::Foreground, true);
}

void KisColorSelectorBase::updateColor(const KoColor &color, Acs::ColorRole role, bool needsExplicitColorReset)
{
    commitColor(color, role);

    if (needsExplicitColorReset) {
        setColor(color);
    }
}

void KisColorSelectorBase::requestUpdateColorAndPreview(const KoColor &color, Acs::ColorRole role)
{
    m_updateColorCompressor->start(qMakePair(color, role));
}

void KisColorSelectorBase::slotUpdateColorAndPreview(QPair<KoColor, Acs::ColorRole> color)
{
    updateColorPreview(color.first);
    updateColor(color.first, color.second, false);
}

void KisColorSelectorBase::setColor(const KoColor& color)
{
    Q_UNUSED(color);
}

void KisColorSelectorBase::setHidingTime(int time)
{
    KIS_ASSERT_RECOVER_NOOP(m_isPopup);

    m_hideTimer->setInterval(time);
}

void KisColorSelectorBase::lazyCreatePopup()
{
    if (!m_popup) {
        m_popup = createPopup();
        Q_ASSERT(m_popup);
        m_popup->setParent(this);

        /**
         * On Linux passing Qt::X11BypassWindowManagerHint makes
         * the window never hide if one switches it with Alt+Tab
         * or something like that. The window also don't get any
         * mouse-enter/leave events. So we have to make it normal
         * window (which is visible to the window manager and
         * appears in the tasks bar). Therefore we don't use it
         * anyomore.
         */
        m_popup->setWindowFlags(Qt::FramelessWindowHint |
                                Qt::Window |
                                Qt::NoDropShadowWindowHint);
        m_popup->m_parent = this;
        m_popup->m_isPopup=true;
    }
    m_popup->setCanvas(m_canvas);
    m_popup->updateSettings();
}

void KisColorSelectorBase::showPopup(Move move)
{
    // This slot may be called by some action,
    // so we need to be able to handle it
    lazyCreatePopup();

    QPoint cursorPos = QCursor::pos();

    if (move == MoveToMousePosition) {
        m_popup->move(QPoint(cursorPos.x()-m_popup->width()/2, cursorPos.y()-m_popup->height()/2));
        QRect rc = m_popup->geometry();
        if (rc.x() < 0) rc.setX(0);
        if (rc.y() < 0) rc.setY(0);
        m_popup->setGeometry(rc);
    }

    if (m_colorPreviewPopup) {
        m_colorPreviewPopup->hide();
    }

    m_popup->show();
    m_popup->m_colorPreviewPopup->show();
}

void KisColorSelectorBase::hidePopup()
{
    KIS_ASSERT_RECOVER_RETURN(m_isPopup);

    m_colorPreviewPopup->hide();
    hide();
}

void KisColorSelectorBase::commitColor(const KoColor& color, Acs::ColorRole role)
{
    if (!m_canvas)
        return;

    m_colorUpdateAllowed=false;

    if (role == Acs::Foreground)
        m_canvas->resourceManager()->setForegroundColor(color);
    else
        m_canvas->resourceManager()->setBackgroundColor(color);

    m_colorUpdateAllowed=true;
}

void KisColorSelectorBase::showColorPreview()
{
    if(m_colorPreviewPopup->isHidden()) {
        m_colorPreviewPopup->show();
    }
}

void KisColorSelectorBase::updateColorPreview(const KoColor &color)
{
    m_colorPreviewPopup->setQColor(converter()->toQColor(color));
}

void KisColorSelectorBase::canvasResourceChanged(int key, const QVariant &v)
{
    if (key == KoCanvasResourceProvider::ForegroundColor || key == KoCanvasResourceProvider::BackgroundColor) {
        KoColor realColor(v.value<KoColor>());
        updateColorPreview(realColor);
        if (m_colorUpdateAllowed && !m_colorUpdateSelf) {
            setColor(realColor);
        }
    }
}

const KoColorSpace* KisColorSelectorBase::colorSpace() const
{
    return converter()->paintingColorSpace();
}

void KisColorSelectorBase::updateSettings()
{
    if(m_popup) {
        m_popup->updateSettings();
    }

    KConfigGroup cfg =  KSharedConfig::openConfig()->group("advancedColorSelector");


   int zoomSelectorOptions =  (int) cfg.readEntry("zoomSelectorOptions", 0) ;
   if (zoomSelectorOptions == 0)   {
       setPopupBehaviour(false, true);   // middle mouse button click will open zoom selector
   } else if (zoomSelectorOptions == 1)   {
       setPopupBehaviour(true, false);   // move over will open the zoom selector
   }
   else
   {
        setPopupBehaviour(false, false); // do not show zoom selector
   }


    if(m_isPopup) {
        m_hideOnMouseClick = cfg.readEntry("hidePopupOnClickCheck", false);
        const int zoomSize = cfg.readEntry("zoomSize", 280);
        resize(zoomSize, zoomSize);
    }

    reset();
}

void KisColorSelectorBase::reset()
{
    update();
}

void KisColorSelectorBase::updateBaseColorPreview(const KoColor &color)
{
    m_colorPreviewPopup->setBaseColor(converter()->toQColor(color));
}

void KisColorSelectorBase::updatePreviousColorPreview()
{
    m_colorPreviewPopup->setPreviousColor();
}

void KisColorSelectorBase::updateLastUsedColorPreview(const KoColor &color)
{
    m_colorPreviewPopup->setLastUsedColor(converter()->toQColor(color));
}

KisDisplayColorConverter* KisColorSelectorBase::converter() const
{
    return m_canvas ?
        m_canvas->displayColorConverter() :
                KisDisplayColorConverter::dumbConverterInstance();
}

void KisColorSelectorBase::tryHideAllPopups()
{
    if (m_colorPreviewPopup->isVisible()) {
        m_colorUpdateSelf=false; //this is for allowing advanced selector to listen to outside colour-change events.
        m_colorPreviewPopup->hide();
    }

    if (m_popup && m_popup->isVisible()) {
        m_popup->m_hideTimer->start();
    }

    if (m_isPopup && !m_hideTimer->isActive()) {
        m_hideTimer->start();
    }
}


void KisColorSelectorBase::mouseMoveEvent(QMouseEvent *event)
{
    event->accept();
}
