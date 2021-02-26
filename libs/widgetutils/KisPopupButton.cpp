/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisPopupButton.h"

#include <QPointer>
#include <QApplication>
#include <QDesktopWidget>
#include <QFrame>
#include <QHBoxLayout>
#include <QStyleOption>
#include <QStylePainter>

#include "kis_global.h"
#include <kis_debug.h>

struct KisPopupButton::Private {
    Private()
        : frameLayout(0)
    {}
    QScopedPointer<QFrame> frame;
    QPointer<QWidget> popupWidget;
    QPointer<QHBoxLayout> frameLayout;
    bool arrowVisible = true;
};

KisPopupButton::KisPopupButton(QWidget* parent)
        : QPushButton(parent)
        , m_d(new Private)
{
    setObjectName("KisPopupButton");
    connect(this, SIGNAL(released()), SLOT(showPopupWidget()));
}

KisPopupButton::~KisPopupButton()
{
    delete m_d;
}

void KisPopupButton::setAlwaysVisible(bool v)
{
    if (v) {
        m_d->frame->setFrameStyle(Qt::SubWindow);
        showPopupWidget();
    } else {
        m_d->frame->setFrameStyle(Qt::Popup);
    }
}

void KisPopupButton::setPopupWidget(QWidget* widget)
{
    if (widget) {
        /**
         * Set parent explicitly to null to avoid propagation of the
         * ignored events (specifically, QEvent::ShortcutOverride) to
         * the view object. This avoids starting global actions while
         * the PopUp is active. See bug 329842.
         */
        m_d->frame.reset(new QFrame(0));
        m_d->frame->setObjectName("popup frame");
        m_d->frame->setFrameStyle(QFrame::Box | QFrame::Plain);
        m_d->frame->setWindowFlags(Qt::Popup);
        m_d->frameLayout = new QHBoxLayout(m_d->frame.data());
        m_d->frameLayout->setMargin(0);
        m_d->frameLayout->setSizeConstraint(QLayout::SetFixedSize);
        m_d->frame->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
        m_d->popupWidget = widget;
        m_d->popupWidget->setParent(m_d->frame.data());
        m_d->frameLayout->addWidget(m_d->popupWidget);
    }
}

void KisPopupButton::setPopupWidgetWidth(int w)
{
    m_d->frame->resize(w, m_d->frame->height());
}

void KisPopupButton::showPopupWidget()
{
    if (m_d->popupWidget && !m_d->frame->isVisible()) {
        m_d->frame->raise();
        m_d->frame->show();
        adjustPosition();
    }
    else {
        hidePopupWidget();
    }
}

void KisPopupButton::hidePopupWidget()
{
    if (m_d->popupWidget) {
        m_d->frame->setVisible(false);
    }
}

void KisPopupButton::paintEvent ( QPaintEvent * event  )
{
    QPushButton::paintEvent(event);
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
    QSize popSize = m_d->popupWidget->size();
    QRect popupRect(this->mapToGlobal(QPoint(0, this->size().height())), popSize);

    // Get the available geometry of the screen which contains this KisPopupButton
    QDesktopWidget* desktopWidget = QApplication::desktop();
    QRect screenRect = desktopWidget->availableGeometry(this);
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
