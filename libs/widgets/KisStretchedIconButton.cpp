/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisStretchedIconButton.h"

#include <QIcon>
#include <QLabel>
#include <QHBoxLayout>
#include <QAction>

#include <kis_debug.h>

KisStretchedIconButton::KisStretchedIconButton(QWidget *parent)
    : QToolButton(parent),
      m_label(new QLabel(this))
{
    m_label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(m_label);
    setLayout(layout);
    slotActionChanged();
}

KisStretchedIconButton::~KisStretchedIconButton()
{
}

void KisStretchedIconButton::setStretchedIcon(const QIcon &icon)
{
    m_stretchedIcon = icon;
    updateLabelIcon();
}

QIcon KisStretchedIconButton::stretchedIcon() const
{
    return m_stretchedIcon;
}

void KisStretchedIconButton::setAssociatedAction(QAction *action)
{
    m_actionConnections.clear();
    m_action = action;
    if (action) {
        m_actionConnections.addConnection(action, &QAction::changed,
                                          this, &KisStretchedIconButton::slotActionChanged);
        m_actionConnections.addConnection(this, &KisStretchedIconButton::clicked,
                                          action, &QAction::trigger);
    }
    slotActionChanged();
}

QAction *KisStretchedIconButton::associatedAction() const
{
    return m_action;
}

void KisStretchedIconButton::resizeEvent(QResizeEvent *event)
{
    QToolButton::resizeEvent(event);
    updateLabelIcon();
}

void KisStretchedIconButton::updateLabelIcon()
{
    m_label->setPixmap(m_stretchedIcon.pixmap(this->windowHandle(), m_label->size()));
}

void KisStretchedIconButton::slotActionChanged()
{
    setStretchedIcon(m_action ? m_action->icon() : QIcon());
    setText("");
    setToolTip(m_action ? m_action->toolTip() : "");
    setStatusTip(m_action ? m_action->statusTip() : "");
    setWhatsThis(m_action ? m_action->whatsThis() : "");
    setEnabled(m_action && m_action->isEnabled());
    setVisible(m_action && m_action->isVisible());
}
