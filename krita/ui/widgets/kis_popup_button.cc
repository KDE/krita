/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "widgets/kis_popup_button.h"

#include <QApplication>
#include <QDesktopWidget>

#include <QFrame>
#include <QHBoxLayout>

#include <kis_debug.h>

#include <QStyleOption>
#include <QStylePainter>


struct KisPopupButton::Private {
    Private() : frame(0), frameLayout(0), popupWidget(0) {}
    QFrame* frame;
    QHBoxLayout* frameLayout;
    QWidget* popupWidget;
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
    delete m_d->popupWidget;
    delete m_d->frame;
    if (widget) {
        m_d->frame = new QFrame(this);
        m_d->frame->setFrameStyle(QFrame::Box | QFrame::Plain);
        m_d->frame->setWindowFlags(Qt::Popup);
        m_d->frameLayout = new QHBoxLayout(m_d->frame);
        m_d->frameLayout->setMargin(0);
        m_d->frameLayout->setSizeConstraint(QLayout::SetFixedSize);
        m_d->frame->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
        m_d->popupWidget = widget;
        m_d->popupWidget->setParent(m_d->frame);
        m_d->frameLayout->addWidget(m_d->popupWidget);
    }
}

void KisPopupButton::setPopupWidgetWidth(int w)
{
    m_d->frame->resize(w, m_d->frame->height());
}

void KisPopupButton::showPopupWidget()
{
    if (m_d->popupWidget) {
        m_d->frame->raise();
        m_d->frame->show();
        QSize popSize = m_d->popupWidget->size();
        QRect popupRect(this->mapToGlobal(QPoint(0, this->size().height())), popSize);

        // Make sure the popup is not drawn outside the screen area
        QDesktopWidget* desktopWidget = QApplication::desktop();
        QRect screenRect = desktopWidget->availableGeometry(desktopWidget->screenNumber());
        if (popupRect.right() > screenRect.right())
            popupRect.translate(screenRect.right() - popupRect.right(), 0);
        if (popupRect.left() < screenRect.left())
            popupRect.translate(screenRect.left() - popupRect.left(), 0);
        //if (popupRect.bottom() > screenRect.bottom())
        //    popupRect.translate(0, -m_d->frame->height());
        m_d->frame->setGeometry(popupRect);

    }
}

void KisPopupButton::hidePopupWidget()
{
    if (m_d->popupWidget) {
        m_d->frame->setVisible(false);
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

#include "kis_popup_button.moc"
