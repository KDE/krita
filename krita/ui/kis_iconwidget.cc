/*
 *  Copyright (c) 2000 Matthias Elter  <elter@kde.org>
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.g
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <QPainter>
#include <QIcon>
#include <QTableWidgetItem>
#include "kis_iconwidget.h"

KisIconWidget::KisIconWidget(QWidget *parent, const char *name) : super(parent)
{
    setObjectName(name);
    m_item = 0;
}

void KisIconWidget::slotSetItem(QTableWidgetItem& item)
{
    m_item = &item;
    update();
}

void KisIconWidget::paintEvent(QPaintEvent *)
{
    if (m_item) {
        QPainter p(this);

        m_item->icon().paint(&p, 0, 0, 24, 24);
        qint32 cw = width();
        qint32 ch = height();

        p.setPen(Qt::gray);
        p.drawRect(0, 0, cw + 1, ch + 1);
    }
}

#include "kis_iconwidget.moc"

