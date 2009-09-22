/*
 *  Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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

#include "kis_config_widget.h"
#include <QTimer>

KisConfigWidget::KisConfigWidget(QWidget * parent, Qt::WFlags f)
        : QWidget(parent, f)
{
    m_timer.setSingleShot(true);
    connect(&m_timer, SIGNAL(timeout()), SLOT(slotConfigChanged()));
    connect(this, SIGNAL(sigConfigurationItemChanged()), SLOT(kickTimer()));
}

KisConfigWidget::~KisConfigWidget()
{
}

void KisConfigWidget::slotConfigChanged()
{
    emit sigConfigurationUpdated();
}

void KisConfigWidget::kickTimer()
{
    m_timer.start(500);
}

#include "kis_config_widget.moc"
