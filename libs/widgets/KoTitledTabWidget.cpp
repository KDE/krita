/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
