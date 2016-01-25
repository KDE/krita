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
#include "kis_debug.h"
#include <QTimer>

KisConfigWidget::KisConfigWidget(QWidget * parent, Qt::WFlags f, int delay)
        : QWidget(parent, f)
        , m_compressor(delay, KisSignalCompressor::FIRST_ACTIVE)
{
    connect(&m_compressor, SIGNAL(timeout()), SLOT(slotConfigChanged()));
    connect(this, SIGNAL(sigConfigurationItemChanged()), &m_compressor, SLOT(start()));
}

KisConfigWidget::~KisConfigWidget()
{
}

void KisConfigWidget::slotConfigChanged()
{
    emit sigConfigurationUpdated();
}

void KisConfigWidget::setView(KisViewManager *view)
{
    if (!view) {
        warnKrita << "KisConfigWidget::setView has got view == 0. That's a bug! Please report it!";
    }
}

