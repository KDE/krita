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
#include <QMenu>

#include "kis_global.h"
#include <kis_debug.h>

// for HAVE_WAYLAND
#include <KoConfig.h>

#if defined HAVE_WAYLAND && QT_VERSION >= QT_VERSION_CHECK(6, 11, 0)
#define HAVE_QT_PRIVATE_TOOLTIP_POSITION_API
#endif

#ifdef HAVE_QT_PRIVATE_TOOLTIP_POSITION_API
#include <QtWaylandClient/private/qwaylandwindow_p.h>
#endif


class KisPopupButtonFrame : public QFrame
{
    QHBoxLayout* frameLayout {0};

public:
    KisPopupButtonFrame(QWidget *parent, bool detach, bool popupIsMenu)
        : QFrame(parent, Qt::Window)
    {
        setObjectName("KisPopupButtonFrame");
        setProperty("_kis_excludeFromLayoutThumbnail", true);
        frameLayout = new QHBoxLayout(this);
        frameLayout->setContentsMargins(0, 0, 0, 0);

        setDetached(detach, popupIsMenu);
    }

    void setDetached(bool detach, bool popupIsMenu)
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
        }
        else {
            setWindowFlags(Qt::Popup);
            // Menus already have a frame, don't need another.
            if (popupIsMenu) {
                setFrameStyle(QFrame::NoFrame);
            } else {
                setFrameStyle(QFrame::Box | QFrame::Plain);
            }
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
        m_d->frame->setDetached(detach, qobject_cast<QMenu *>(m_d->popupWidget));
        if (wasVisible) {
            // Setting the window flags closes the widget, so make it visible again.
            setPopupWidgetVisible(true);
        }
    }
}

void KisPopupButton::setPopupWidget(QWidget* widget)
{
    if (widget) {
        delete m_d->frame;

        QWidget *effectiveParent = widget->parentWidget() ? widget->parentWidget() : this->window();

        // Bit wonky to assign a popup menu to a popup widget, so they need
        // extra coddling to make them work properly.
        QMenu *menu = qobject_cast<QMenu *>(widget);
        m_d->frame = new KisPopupButtonFrame(effectiveParent, m_d->isPopupDetached, menu);
        m_d->frame->setWindowTitle(widget->windowTitle());

        m_d->popupWidget = widget;

        m_d->frame->layout()->addWidget(m_d->popupWidget);

        if (menu) {
            // The menu may decide to hide itself in response to user input.
            // Compensate for that by catching the situation where we'd hide
            // the menu while the frame is still up and show it again.
            connect(menu, &QMenu::aboutToHide, this, &KisPopupButton::slotMenuAboutToHide);
            // If the menu still hides itself somehow, make it show up again.
            connect(this, &KisPopupButton::sigPopupWidgetShown, menu, &QMenu::show);
        }
    }
}

void KisPopupButton::showPopupWidget()
{
    setPopupWidgetVisible(!m_d->frame->isVisible());
}

void KisPopupButton::hidePopupWidget()
{
    setPopupWidgetVisible(false);
}

void KisPopupButton::setPopupWidgetVisible(bool visible)
{
    if (m_d->popupWidget) {
        if (visible) {
            adjustPositionAndShow();
            Q_EMIT sigPopupWidgetShown();
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

void KisPopupButton::adjustPositionAndShow()
{
    const QSize popupSize = m_d->frame->sizeHint();
    QRect popupRect(QPoint(), popupSize);

    // resize **must** come before `setParentControlGeometry()`, otherwise
    // compositor will place the window incorrectly!
    m_d->frame->resize(popupSize);

    // make sure that all the platform-specific structures for the window
    // are created (i.e. QWaylandWindow) before trying to request them
    // via `windowHandle()->handle()`
    m_d->frame->winId();

#ifdef HAVE_QT_PRIVATE_TOOLTIP_POSITION_API
    if (auto waylandWindow = dynamic_cast<QNativeInterface::Private::QWaylandWindow *>(m_d->frame->windowHandle()->handle())) {
        if (m_d->isPopupDetached) {
            // one cannot control dialog position on Wayland, sorry :(
        } else {
            if (this->isVisible()) {
                QWidget *parentWidget = m_d->frame->parentWidget();

                const QRect gravityRect(parentWidget->window()->mapFromGlobal(mapToGlobal(QPoint())), size());

                waylandWindow->setParentControlGeometry(gravityRect);
                waylandWindow->setExtendedWindowType(QNativeInterface::Private::QWaylandWindow::ComboBox);
            } else {
                QWidget *parentWidget = m_d->frame->parentWidget();
                const QPoint parentPos = parentWidget->window()->mapFromGlobal(QCursor::pos());
                const QRect gravityRect(parentPos.x() - popupSize.width() / 2, parentPos.y() - popupSize.height() / 2, popupSize.width(), popupSize.height());

                waylandWindow->setParentControlGeometry(gravityRect);
                waylandWindow->setExtendedWindowType(QNativeInterface::Private::QWaylandWindow::Default);
            }
        }

        m_d->frame->show();
    } else
#endif /* HAVE_QT_PRIVATE_TOOLTIP_POSITION_API */

    {
        // Get the available geometry of the screen which contains the popup.
        QScreen *screen = this->parentWidget() ? this->screen() : qApp->screenAt(QCursor::pos());
        const QRect screenRect = screen ? screen->availableGeometry() : QRect();

        if (m_d->isPopupDetached) {

            if (m_d->isDetachedGeometrySet) {
                popupRect.moveTo(m_d->frame->geometry().topLeft());
            } else {
                popupRect.moveTo(this->window()->geometry().center() - QRect(QPoint(0, 0), popupSize).center());
                m_d->isDetachedGeometrySet = true;
            }
        } else {
            // Attach to the button if it's visible, else attach to the cursor.
            if (this->isVisible()) {
                popupRect.moveTo(this->mapToGlobal(rect().bottomLeft()));
            } else {
                popupRect.moveCenter(QCursor::pos());
            }
        }

        if (!screenRect.isEmpty()) {
            popupRect = kisEnsureInRect(popupRect, screenRect);
        }

        /**
         * On non-wayland platforms we first show the widget and only then
         * apply geometry, otherwise the settings are not applied properly
         */
        m_d->frame->show();
        m_d->frame->setGeometry(popupRect);
    }
}

void KisPopupButton::setArrowVisible (bool v)
{
    if (v) {
        m_d->arrowVisible = true;
    } else {
        m_d->arrowVisible = false;
    }
}

void KisPopupButton::slotMenuAboutToHide()
{
    // Menus will hide themselves when clicking on one of their separators,
    // which just ends up with a blank popup frame. Fight back against this by
    // showing the menu again if the frame is still up when the menu disappears.
    if (m_d->popupWidget && m_d->frame->isVisible()) {
        m_d->popupWidget->show();
    }
}
