/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoTitledTabWidget.h"

#include <QLabel>

KoTitledTabWidget::KoTitledTabWidget(QWidget *parent)
    : QTabWidget(parent)
{
    m_titleLabel = new QLabel(this);
    setCornerWidget(m_titleLabel);

    connect(this, SIGNAL(currentChanged(int)), SLOT(slotUpdateTitle()));
    slotUpdateTitle();
}

void KoTitledTabWidget::slotUpdateTitle()
{
    QWidget *widget = this->widget(currentIndex());

    if (widget) {
        const QString title = widget->windowTitle();

        m_titleLabel->setVisible(!title.isEmpty());
        m_titleLabel->setText(title);
    }
}
