/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "black_white_point_chooser.h"

#include "klocalizedstring.h"

#include "kis_global.h"

#include <QApplication>
#include <QDesktopWidget>

#include <QFormLayout>
#include "kis_slider_spin_box.h"


BlackWhitePointChooser::BlackWhitePointChooser(QWidget* parent)
    : QFrame(parent, Qt::Popup)
{
    setFrameStyle(QFrame::Panel|QFrame::Raised);

    m_black = new KisDoubleSliderSpinBox(this);
    m_black->setRange(0.0, 10000, 4);
    m_black->setValue(0.0);
    m_black->setSingleStep(0.01);
    m_black->setMinimumWidth(120);
    m_black->setExponentRatio(6.0);

    m_white = new KisDoubleSliderSpinBox(this);
    m_white->setRange(0.0, 10000, 4);
    m_white->setValue(1.0);
    m_white->setSingleStep(0.01);
    m_white->setMinimumWidth(120);
    m_white->setExponentRatio(6.0);

    connect(m_black, SIGNAL(valueChanged(qreal)), SIGNAL(sigBlackPointChanged(qreal)));
    connect(m_white, SIGNAL(valueChanged(qreal)), SIGNAL(sigWhitePointChanged(qreal)));

    QFormLayout *layout = new QFormLayout(this);
    layout->addRow(i18n("Black:"), m_black);
    layout->addRow(i18n("White:"), m_white);
}

BlackWhitePointChooser::~BlackWhitePointChooser()
{
}

void BlackWhitePointChooser::showPopup(const QPoint &basePoint)
{
    show();

    QSize popupSize = size();
    QRect popupRect(basePoint - QPoint(0, popupSize.height()), popupSize);
    QRect screenRect = QApplication::desktop()->availableGeometry(this);

    popupRect = kisEnsureInRect(popupRect, screenRect);
    setGeometry(popupRect);
}

qreal BlackWhitePointChooser::blackPoint() const
{
    return m_black->value();
}

void BlackWhitePointChooser::setBlackPoint(qreal bp)
{
    m_black->setValue(bp);
}

qreal BlackWhitePointChooser::whitePoint() const
{
    return m_white->value();
}

void BlackWhitePointChooser::setWhitePoint(qreal wp)
{
    m_white->setValue(wp);
}
