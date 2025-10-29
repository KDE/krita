/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisPopupButton.h"

#include <QPointer>
#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QScreen>
#include <QStyleOption>
#include <QStylePainter>
#include <QWindow>

#include "kis_global.h"
#include <kis_debug.h>


class KisPopupButtonFrame : public QFrame
{
    QHBoxLayout* frameLayout {0};

public:
    KisPopupButtonFrame(QWidget *parent, bool detach)
        : QFrame(parent)
    {
        setObjectName("KisPopupButtonFrame");
        setProperty("_kis_excludeFromLayoutThumbnail", true);
        frameLayout = new QHBoxLayout(this);
        frameLayout->setMargin(0);

        setDetached(detach);
    }

    void setDetached(bool detach)
    {
#if defined Q_OS_ANDROID || defined Q_OS_MACOS
        // for some reason when calling destroy() the platform window isn't
        // hidden first, this corrupts state of the window stack
        hide();
#endif

        // Need to destroy the platform window before changing window flags
        // so that Qt knows to actually apply the new flags...
        // At least on Windows, not doing this may result in weird window drop
        // shadows.
        destroy();

        if (detach) {
            setWindowFlags(Qt::Dialog);
            setFrameStyle(QFrame::NoFrame);        
            setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
            // setting the size policy to fixed sets the maximum size, somehow
            setMaximumSize(0xFFFFFF, 0xFFFFFF);
            frameLayout->setSizeConstraint(QLayout::SizeConstraint::SetDefaultConstraint);
        }
        else {
            setWindowFlags(Qt::Popup);
            setFrameStyle(QFrame::Box | QFrame::Plain);
            setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
            frameLayout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
        }

        updateGeometry();
    }

protected:
    void keyPressEvent(QKeyEvent *event) override
    {
        if (event->matches(QKeySequence::Cancel)) {
            event->accept();
            hide();
        } else {
            QFrame::keyPressEvent(event);
        }
    }

    bool event(QEvent *e) override
    {
        if (e->type() == QEvent::Close) {
            e->ignore();
            hide();
            return true;
        }
        return QFrame::event(e);
    }
};


struct KisPopupButton::Private {
    Private()
        // : frameLayout(nullptr)
    {}
    QPointer<KisPopupButtonFrame> frame;
    QPointer<QWidget> popupWidget;
    bool arrowVisible { true };
    bool isPopupDetached { false };
    bool isDetachedGeometrySet { false };
};

KisPopupButton::KisPopupButton(QWidget* parent)
        : QToolButton(parent)
        , m_d(new Private)
{
    setObjectName("KisPopupButton");
    connect(this, SIGNAL(released()), SLOT(showPopupWidget()));
}

KisPopupButton::~KisPopupButton()
{
    delete m_d->frame;
    delete m_d;
}

void KisPopupButton::setPopupWidgetDetached(bool detach)
{
    m_d->isPopupDetached = detach;
    if (m_d->frame) {
        bool wasVisible = isPopupWidgetVisible();
        m_d->frame->setDetached(detach);
        if (wasVisible) {
            // Setting the window flags closes the widget, so make it visible again.
            setPopupWidgetVisible(true);
            if (detach) {
                m_d->isDetachedGeometrySet = true;
            }
            adjustPosition();
        }
    }
}

void KisPopupButton::setPopupWidget(QWidget* widget)
{
    if (widget) {
        delete m_d->frame;
        m_d->frame = new KisPopupButtonFrame(this->window(), m_d->isPopupDetached);
        m_d->frame->setWindowTitle(widget->windowTitle());

        m_d->popupWidget = widget;

        m_d->frame->layout()->addWidget(m_d->popupWidget);
    }
}

void KisPopupButton::setPopupWidgetWidth(int w)
{
    m_d->frame->resize(w, m_d->frame->height());
}

void KisPopupButton::showPopupWidget()
{
    if (m_d->popupWidget && !m_d->frame->isVisible()) {
        setPopupWidgetVisible(true);
    } else {
        hidePopupWidget();
    }
}

void KisPopupButton::hidePopupWidget()
{
    setPopupWidgetVisible(false);
}

void KisPopupButton::setPopupWidgetVisible(bool visible)
{
    if (m_d->popupWidget) {
        if (visible) {
            // Force layout sizing before positioning
            m_d->popupWidget->adjustSize();
            m_d->frame->adjustSize();
            adjustPosition();
            m_d->frame->raise();
            m_d->frame->show();
            m_d->frame->activateWindow();
        } else {
            m_d->frame->setVisible(false);
        }
    }
}

bool KisPopupButton::isPopupWidgetVisible()
{
    return m_d->popupWidget && m_d->frame->isVisible();
}

void KisPopupButton::paintEvent ( QPaintEvent * event  )
{
    QToolButton::paintEvent(event);
    if (m_d->arrowVisible) {
        paintPopupArrow();
    }
}

void KisPopupButton::paintPopupArrow()
{
    QStylePainter p(this);
    QStyleOption option;
    option.rect = QRect(rect().right() - 15, rect().bottom() - 15, 14, 14);
    option.palette = palette();
    option.palette.setBrush(QPalette::ButtonText, Qt::black); // Force color to black
    option.state = QStyle::State_Enabled;
    p.setBrush(Qt::black); // work around some theme that don't use QPalette::ButtonText like they  should, but instead the QPainter brushes and pen
    p.setPen(Qt::black);
    p.drawPrimitive(QStyle::PE_IndicatorArrowDown, option);
}

void KisPopupButton::adjustPosition()
{
    // If popup is not detached, or if its detached geometry hasn't been set,
    // we first move the popup to the "current" screen.
    if (!m_d->isPopupDetached || !m_d->isDetachedGeometrySet) {
        QScreen *currentScreen = [this]() {
            QWindow *mainWinHandle = this->window()->windowHandle();
            if (mainWinHandle) {
                return mainWinHandle->screen();
            }
            return QApplication::primaryScreen();
        }();
        QWindow *winHandle = m_d->frame->windowHandle();
        if (winHandle) {
            winHandle->setScreen(currentScreen);
        }
    }

    QSize popSize = m_d->popupWidget->size();
    QRect popupRect(this->mapToGlobal(QPoint(0, this->size().height())), popSize);

    // Get the available geometry of the screen which contains the popup.
    QScreen *screen = [this]() {
        QWindow *winHandle = m_d->frame->windowHandle();
        if (winHandle && winHandle->screen()) {
            return winHandle->screen();
        }
        return QApplication::primaryScreen();
    }();
    QRect screenRect = screen->availableGeometry();
    if (m_d->isPopupDetached) {
        if (m_d->isDetachedGeometrySet) {
            popupRect.moveTo(m_d->frame->geometry().topLeft());
        } else {
            popupRect.moveTo(this->window()->geometry().center() - QRect(QPoint(0, 0), popSize).center());
            m_d->isDetachedGeometrySet = true;
        }
    }
    popupRect = kisEnsureInRect(popupRect, screenRect);

    m_d->frame->setGeometry(popupRect);
}

void KisPopupButton::setArrowVisible (bool v)
{
    if (v) {
        m_d->arrowVisible = true;
    } else {
        m_d->arrowVisible = false;
    }
}
